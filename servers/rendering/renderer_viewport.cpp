/**************************************************************************/
/*  renderer_viewport.cpp                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "renderer_viewport.h"

#include "core/config/project_settings.h"
#include "core/object/worker_thread_pool.h"
#include "renderer_canvas_cull.h"
#include "renderer_scene_cull.h"
#include "rendering_server_globals.h"
#include "storage/texture_storage.h"

static Transform2D _canvas_get_transform(RendererViewport::Viewport *p_viewport, RendererCanvasCull::Canvas *p_canvas, RendererViewport::Viewport::CanvasData *p_canvas_data, const Vector2 &p_vp_size) {
	Transform2D xf = p_viewport->global_transform;

	float scale = 1.0;
	if (p_viewport->canvas_map.has(p_canvas->parent)) {
		Transform2D c_xform = p_viewport->canvas_map[p_canvas->parent].transform;
		if (p_viewport->snap_2d_transforms_to_pixel) {
			c_xform.columns[2] = c_xform.columns[2].floor();
		}
		xf = xf * c_xform;
		scale = p_canvas->parent_scale;
	}

	Transform2D c_xform = p_canvas_data->transform;

	if (p_viewport->snap_2d_transforms_to_pixel) {
		c_xform.columns[2] = c_xform.columns[2].floor();
	}

	xf = xf * c_xform;

	if (scale != 1.0 && !RSG::canvas->disable_scale) {
		Vector2 pivot = p_vp_size * 0.5;
		Transform2D xfpivot;
		xfpivot.set_origin(pivot);
		Transform2D xfscale;
		xfscale.scale(Vector2(scale, scale));

		xf = xfpivot.affine_inverse() * xf;
		xf = xfscale * xf;
		xf = xfpivot * xf;
	}

	return xf;
}

Vector<RendererViewport::Viewport *> RendererViewport::_sort_active_viewports() {
	// We need to sort the viewports in a "topological order", children first and
	// parents last. We also need to keep sibling viewports in the original order
	// from top to bottom.

	Vector<Viewport *> result;
	List<Viewport *> nodes;

	for (int i = active_viewports.size() - 1; i >= 0; --i) {
		Viewport *viewport = active_viewports[i];
		if (viewport->parent.is_valid()) {
			continue;
		}

		nodes.push_back(viewport);
		result.insert(0, viewport);
	}

	while (!nodes.is_empty()) {
		const Viewport *node = nodes[0];
		nodes.pop_front();

		for (int i = active_viewports.size() - 1; i >= 0; --i) {
			Viewport *child = active_viewports[i];
			if (child->parent != node->self) {
				continue;
			}

			if (!nodes.find(child)) {
				nodes.push_back(child);
				result.insert(0, child);
			}
		}
	}

	return result;
}

void RendererViewport::_draw_viewport(Viewport *p_viewport) {
	if (p_viewport->measure_render_time) {
		String rt_id = "vp_begin_" + itos(p_viewport->self.get_id());
		RSG::utilities->capture_timestamp(rt_id);
		timestamp_vp_map[rt_id] = p_viewport->self;
	}

	if (OS::get_singleton()->get_current_rendering_method() == "gl_compatibility") {
		// This is currently needed for GLES to keep the current window being rendered to up to date
		DisplayServer::get_singleton()->gl_window_make_current(p_viewport->viewport_to_screen);
	}

	for (int i = 0; i < RS::VIEWPORT_RENDER_INFO_TYPE_MAX; i++) {
		for (int j = 0; j < RS::VIEWPORT_RENDER_INFO_MAX; j++) {
			p_viewport->render_info.info[i][j] = 0;
		}
	}

	Color bgcolor = p_viewport->transparent_bg ? Color(0, 0, 0, 0) : RSG::texture_storage->get_default_clear_color();

	if (p_viewport->clear_mode != RS::VIEWPORT_CLEAR_NEVER) {
		RSG::texture_storage->render_target_request_clear(p_viewport->render_target, bgcolor);
		if (p_viewport->clear_mode == RS::VIEWPORT_CLEAR_ONLY_NEXT_FRAME) {
			p_viewport->clear_mode = RS::VIEWPORT_CLEAR_NEVER;
		}
	}

	if (!p_viewport->disable_2d) {
		RBMap<Viewport::CanvasKey, Viewport::CanvasData *> canvas_map;

		Rect2 clip_rect(0, 0, p_viewport->size.x, p_viewport->size.y);
		RendererCanvasRender::Light *lights = nullptr;
		RendererCanvasRender::Light *lights_with_shadow = nullptr;

		RendererCanvasRender::Light *directional_lights = nullptr;
		RendererCanvasRender::Light *directional_lights_with_shadow = nullptr;

		if (p_viewport->sdf_active) {
			// Process SDF.

			Rect2 sdf_rect = RSG::texture_storage->render_target_get_sdf_rect(p_viewport->render_target);

			RendererCanvasRender::LightOccluderInstance *occluders = nullptr;

			// Make list of occluders.
			for (KeyValue<RID, Viewport::CanvasData> &E : p_viewport->canvas_map) {
				RendererCanvasCull::Canvas *canvas = static_cast<RendererCanvasCull::Canvas *>(E.value.canvas);
				Transform2D xf = _canvas_get_transform(p_viewport, canvas, &E.value, clip_rect.size);

				for (RendererCanvasRender::LightOccluderInstance *F : canvas->occluders) {
					if (!F->enabled) {
						continue;
					}
					F->xform_cache = xf * F->xform;

					if (sdf_rect.intersects_transformed(F->xform_cache, F->aabb_cache)) {
						F->next = occluders;
						occluders = F;
					}
				}
			}

			RSG::canvas_render->render_sdf(p_viewport->render_target, occluders);
			RSG::texture_storage->render_target_mark_sdf_enabled(p_viewport->render_target, true);

			p_viewport->sdf_active = false; // If used, gets set active again.
		} else {
			RSG::texture_storage->render_target_mark_sdf_enabled(p_viewport->render_target, false);
		}

		Rect2 shadow_rect;

		int shadow_count = 0;
		int directional_light_count = 0;

		RENDER_TIMESTAMP("Cull 2D Lights");
		for (KeyValue<RID, Viewport::CanvasData> &E : p_viewport->canvas_map) {
			RendererCanvasCull::Canvas *canvas = static_cast<RendererCanvasCull::Canvas *>(E.value.canvas);

			Transform2D xf = _canvas_get_transform(p_viewport, canvas, &E.value, clip_rect.size);

			// Find lights in canvas.

			for (RendererCanvasRender::Light *F : canvas->lights) {
				RendererCanvasRender::Light *cl = F;
				if (cl->enabled && cl->texture.is_valid()) {
					//not super efficient..
					Size2 tsize = RSG::texture_storage->texture_size_with_proxy(cl->texture);
					tsize *= cl->scale;

					Vector2 offset = tsize / 2.0;
					cl->rect_cache = Rect2(-offset + cl->texture_offset, tsize);
					cl->xform_cache = xf * cl->xform;

					if (clip_rect.intersects_transformed(cl->xform_cache, cl->rect_cache)) {
						cl->filter_next_ptr = lights;
						lights = cl;
						Transform2D scale;
						scale.scale(cl->rect_cache.size);
						scale.columns[2] = cl->rect_cache.position;
						cl->light_shader_xform = xf * cl->xform * scale;
						if (cl->use_shadow) {
							cl->shadows_next_ptr = lights_with_shadow;
							if (lights_with_shadow == nullptr) {
								shadow_rect = cl->xform_cache.xform(cl->rect_cache);
							} else {
								shadow_rect = shadow_rect.merge(cl->xform_cache.xform(cl->rect_cache));
							}
							lights_with_shadow = cl;
							cl->radius_cache = cl->rect_cache.size.length();
						}
					}
				}
			}

			for (RendererCanvasRender::Light *F : canvas->directional_lights) {
				RendererCanvasRender::Light *cl = F;
				if (cl->enabled) {
					cl->filter_next_ptr = directional_lights;
					directional_lights = cl;
					cl->xform_cache = xf * cl->xform;
					cl->xform_cache.columns[2] = Vector2(); //translation is pointless
					if (cl->use_shadow) {
						cl->shadows_next_ptr = directional_lights_with_shadow;
						directional_lights_with_shadow = cl;
					}

					directional_light_count++;

					if (directional_light_count == RS::MAX_2D_DIRECTIONAL_LIGHTS) {
						break;
					}
				}
			}

			canvas_map[Viewport::CanvasKey(E.key, E.value.layer, E.value.sublayer)] = &E.value;
		}

		if (lights_with_shadow) {
			//update shadows if any

			RendererCanvasRender::LightOccluderInstance *occluders = nullptr;

			RENDER_TIMESTAMP("> Render PointLight2D Shadows");
			RENDER_TIMESTAMP("Cull LightOccluder2Ds");

			//make list of occluders
			for (KeyValue<RID, Viewport::CanvasData> &E : p_viewport->canvas_map) {
				RendererCanvasCull::Canvas *canvas = static_cast<RendererCanvasCull::Canvas *>(E.value.canvas);
				Transform2D xf = _canvas_get_transform(p_viewport, canvas, &E.value, clip_rect.size);

				for (RendererCanvasRender::LightOccluderInstance *F : canvas->occluders) {
					if (!F->enabled) {
						continue;
					}
					F->xform_cache = xf * F->xform;
					if (shadow_rect.intersects_transformed(F->xform_cache, F->aabb_cache)) {
						F->next = occluders;
						occluders = F;
					}
				}
			}
			//update the light shadowmaps with them

			RendererCanvasRender::Light *light = lights_with_shadow;
			while (light) {
				RENDER_TIMESTAMP("Render PointLight2D Shadow");

				RSG::canvas_render->light_update_shadow(light->light_internal, shadow_count++, light->xform_cache.affine_inverse(), light->item_shadow_mask, light->radius_cache / 1000.0, light->radius_cache * 1.1, occluders);
				light = light->shadows_next_ptr;
			}

			RENDER_TIMESTAMP("< Render PointLight2D Shadows");
		}

		if (directional_lights_with_shadow) {
			//update shadows if any
			RendererCanvasRender::Light *light = directional_lights_with_shadow;
			while (light) {
				Vector2 light_dir = -light->xform_cache.columns[1].normalized(); // Y is light direction
				float cull_distance = light->directional_distance;

				Vector2 light_dir_sign;
				light_dir_sign.x = (ABS(light_dir.x) < CMP_EPSILON) ? 0.0 : ((light_dir.x > 0.0) ? 1.0 : -1.0);
				light_dir_sign.y = (ABS(light_dir.y) < CMP_EPSILON) ? 0.0 : ((light_dir.y > 0.0) ? 1.0 : -1.0);

				Vector2 points[6];
				int point_count = 0;

				for (int j = 0; j < 4; j++) {
					static const Vector2 signs[4] = { Vector2(1, 1), Vector2(1, 0), Vector2(0, 0), Vector2(0, 1) };
					Vector2 sign_cmp = signs[j] * 2.0 - Vector2(1.0, 1.0);
					Vector2 point = clip_rect.position + clip_rect.size * signs[j];

					if (sign_cmp == light_dir_sign) {
						//both point in same direction, plot offsetted
						points[point_count++] = point + light_dir * cull_distance;
					} else if (sign_cmp.x == light_dir_sign.x || sign_cmp.y == light_dir_sign.y) {
						int next_j = (j + 1) % 4;
						Vector2 next_sign_cmp = signs[next_j] * 2.0 - Vector2(1.0, 1.0);

						//one point in the same direction, plot segment

						if (next_sign_cmp.x == light_dir_sign.x || next_sign_cmp.y == light_dir_sign.y) {
							if (light_dir_sign.x != 0.0 || light_dir_sign.y != 0.0) {
								points[point_count++] = point;
							}
							points[point_count++] = point + light_dir * cull_distance;
						} else {
							points[point_count++] = point + light_dir * cull_distance;
							if (light_dir_sign.x != 0.0 || light_dir_sign.y != 0.0) {
								points[point_count++] = point;
							}
						}
					} else {
						//plot normally
						points[point_count++] = point;
					}
				}

				Vector2 xf_points[6];

				RendererCanvasRender::LightOccluderInstance *occluders = nullptr;

				RENDER_TIMESTAMP("> Render DirectionalLight2D Shadows");

				// Make list of occluders.
				for (KeyValue<RID, Viewport::CanvasData> &E : p_viewport->canvas_map) {
					RendererCanvasCull::Canvas *canvas = static_cast<RendererCanvasCull::Canvas *>(E.value.canvas);
					Transform2D xf = _canvas_get_transform(p_viewport, canvas, &E.value, clip_rect.size);

					for (RendererCanvasRender::LightOccluderInstance *F : canvas->occluders) {
						if (!F->enabled) {
							continue;
						}
						F->xform_cache = xf * F->xform;
						Transform2D localizer = F->xform_cache.affine_inverse();

						for (int j = 0; j < point_count; j++) {
							xf_points[j] = localizer.xform(points[j]);
						}
						if (F->aabb_cache.intersects_filled_polygon(xf_points, point_count)) {
							F->next = occluders;
							occluders = F;
						}
					}
				}

				RSG::canvas_render->light_update_directional_shadow(light->light_internal, shadow_count++, light->xform_cache, light->item_shadow_mask, cull_distance, clip_rect, occluders);

				light = light->shadows_next_ptr;
			}

			RENDER_TIMESTAMP("< Render DirectionalLight2D Shadows");
		}

		for (const KeyValue<Viewport::CanvasKey, Viewport::CanvasData *> &E : canvas_map) {
			RendererCanvasCull::Canvas *canvas = static_cast<RendererCanvasCull::Canvas *>(E.value->canvas);

			Transform2D xform = _canvas_get_transform(p_viewport, canvas, E.value, clip_rect.size);

			RendererCanvasRender::Light *canvas_lights = nullptr;
			RendererCanvasRender::Light *canvas_directional_lights = nullptr;

			RendererCanvasRender::Light *ptr = lights;
			while (ptr) {
				if (E.value->layer >= ptr->layer_min && E.value->layer <= ptr->layer_max) {
					ptr->next_ptr = canvas_lights;
					canvas_lights = ptr;
				}
				ptr = ptr->filter_next_ptr;
			}

			ptr = directional_lights;
			while (ptr) {
				if (E.value->layer >= ptr->layer_min && E.value->layer <= ptr->layer_max) {
					ptr->next_ptr = canvas_directional_lights;
					canvas_directional_lights = ptr;
				}
				ptr = ptr->filter_next_ptr;
			}

			RSG::canvas->render_canvas(p_viewport->render_target, canvas, xform, canvas_lights, canvas_directional_lights, clip_rect, p_viewport->texture_filter, p_viewport->texture_repeat, p_viewport->snap_2d_transforms_to_pixel, p_viewport->snap_2d_vertices_to_pixel, p_viewport->canvas_cull_mask);
			if (RSG::canvas->was_sdf_used()) {
				p_viewport->sdf_active = true;
			}
		}
	}

	if (RSG::texture_storage->render_target_is_clear_requested(p_viewport->render_target)) {
		//was never cleared in the end, force clear it
		RSG::texture_storage->render_target_do_clear_request(p_viewport->render_target);
	}

	if (p_viewport->measure_render_time) {
		String rt_id = "vp_end_" + itos(p_viewport->self.get_id());
		RSG::utilities->capture_timestamp(rt_id);
		timestamp_vp_map[rt_id] = p_viewport->self;
	}
}

void RendererViewport::draw_viewports() {
	timestamp_vp_map.clear();

	if (Engine::get_singleton()->is_editor_hint()) {
		set_default_clear_color(GLOBAL_GET("rendering/environment/defaults/default_clear_color"));
	}

	if (sorted_active_viewports_dirty) {
		sorted_active_viewports = _sort_active_viewports();
		sorted_active_viewports_dirty = false;
	}

	HashMap<DisplayServer::WindowID, Vector<BlitToScreen>> blit_to_screen_list;
	//draw viewports
	RENDER_TIMESTAMP("> Render Viewports");

	//determine what is visible
	draw_viewports_pass++;

	for (int i = sorted_active_viewports.size() - 1; i >= 0; i--) { //to compute parent dependency, must go in reverse draw order

		Viewport *vp = sorted_active_viewports[i];

		if (vp->update_mode == RS::VIEWPORT_UPDATE_DISABLED) {
			continue;
		}

		if (!vp->render_target.is_valid()) {
			continue;
		}
		//ERR_CONTINUE(!vp->render_target.is_valid());

		bool visible = vp->viewport_to_screen_rect != Rect2();

		if (vp->update_mode == RS::VIEWPORT_UPDATE_ALWAYS || vp->update_mode == RS::VIEWPORT_UPDATE_ONCE) {
			visible = true;
		}

		if (vp->update_mode == RS::VIEWPORT_UPDATE_WHEN_VISIBLE && RSG::texture_storage->render_target_was_used(vp->render_target)) {
			visible = true;
		}

		if (vp->update_mode == RS::VIEWPORT_UPDATE_WHEN_PARENT_VISIBLE) {
			Viewport *parent = viewport_owner.get_or_null(vp->parent);
			if (parent && parent->last_pass == draw_viewports_pass) {
				visible = true;
			}
		}

		visible = visible && vp->size.x > 1 && vp->size.y > 1;

		if (visible) {
			vp->last_pass = draw_viewports_pass;
		}
	}

	int vertices_drawn = 0;
	int objects_drawn = 0;
	int draw_calls_used = 0;

	for (int i = 0; i < sorted_active_viewports.size(); i++) {
		Viewport *vp = sorted_active_viewports[i];

		if (vp->last_pass != draw_viewports_pass) {
			continue; //should not draw
		}

		RENDER_TIMESTAMP("> Render Viewport " + itos(i));

		RSG::texture_storage->render_target_set_as_unused(vp->render_target);

		RSG::texture_storage->render_target_set_override(vp->render_target, RID(), RID(), RID());

		RSG::scene->set_debug_draw_mode(vp->debug_draw);

		// render standard mono camera
		_draw_viewport(vp);

		if (vp->viewport_to_screen != DisplayServer::INVALID_WINDOW_ID && (!vp->viewport_render_direct_to_screen || !RSG::rasterizer->is_low_end())) {
			//copy to screen if set as such
			BlitToScreen blit;
			blit.render_target = vp->render_target;
			if (vp->viewport_to_screen_rect != Rect2()) {
				blit.dst_rect = vp->viewport_to_screen_rect;
			} else {
				blit.dst_rect.position = Vector2();
				blit.dst_rect.size = vp->size;
			}

			if (!blit_to_screen_list.has(vp->viewport_to_screen)) {
				blit_to_screen_list[vp->viewport_to_screen] = Vector<BlitToScreen>();
			}

			if (OS::get_singleton()->get_current_rendering_driver_name() == "opengl3") {
				Vector<BlitToScreen> blit_to_screen_vec;
				blit_to_screen_vec.push_back(blit);
				RSG::rasterizer->blit_render_targets_to_screen(vp->viewport_to_screen, blit_to_screen_vec.ptr(), 1);
				RSG::rasterizer->end_frame(true);
			} else {
				blit_to_screen_list[vp->viewport_to_screen].push_back(blit);
			}
		}

		if (vp->update_mode == RS::VIEWPORT_UPDATE_ONCE) {
			vp->update_mode = RS::VIEWPORT_UPDATE_DISABLED;
		}

		RENDER_TIMESTAMP("< Render Viewport " + itos(i));

		objects_drawn += vp->render_info.info[RS::VIEWPORT_RENDER_INFO_TYPE_VISIBLE][RS::VIEWPORT_RENDER_INFO_OBJECTS_IN_FRAME] + vp->render_info.info[RS::VIEWPORT_RENDER_INFO_TYPE_SHADOW][RS::VIEWPORT_RENDER_INFO_OBJECTS_IN_FRAME];
		vertices_drawn += vp->render_info.info[RS::VIEWPORT_RENDER_INFO_TYPE_VISIBLE][RS::VIEWPORT_RENDER_INFO_PRIMITIVES_IN_FRAME] + vp->render_info.info[RS::VIEWPORT_RENDER_INFO_TYPE_SHADOW][RS::VIEWPORT_RENDER_INFO_PRIMITIVES_IN_FRAME];
		draw_calls_used += vp->render_info.info[RS::VIEWPORT_RENDER_INFO_TYPE_VISIBLE][RS::VIEWPORT_RENDER_INFO_DRAW_CALLS_IN_FRAME] + vp->render_info.info[RS::VIEWPORT_RENDER_INFO_TYPE_SHADOW][RS::VIEWPORT_RENDER_INFO_DRAW_CALLS_IN_FRAME];
	}
	RSG::scene->set_debug_draw_mode(RS::VIEWPORT_DEBUG_DRAW_DISABLED);

	total_objects_drawn = objects_drawn;
	total_vertices_drawn = vertices_drawn;
	total_draw_calls_used = draw_calls_used;

	RENDER_TIMESTAMP("< Render Viewports");
	//this needs to be called to make screen swapping more efficient
	RSG::rasterizer->prepare_for_blitting_render_targets();

	for (const KeyValue<int, Vector<BlitToScreen>> &E : blit_to_screen_list) {
		RSG::rasterizer->blit_render_targets_to_screen(E.key, E.value.ptr(), E.value.size());
	}
}

RID RendererViewport::viewport_allocate() {
	return viewport_owner.allocate_rid();
}

void RendererViewport::viewport_initialize(RID p_rid) {
	viewport_owner.initialize_rid(p_rid);
	Viewport *viewport = viewport_owner.get_or_null(p_rid);
	viewport->self = p_rid;
	viewport->render_target = RSG::texture_storage->render_target_create();
	viewport->shadow_atlas = RSG::light_storage->shadow_atlas_create();
	viewport->viewport_render_direct_to_screen = false;
}

void RendererViewport::viewport_set_size(RID p_viewport, int p_width, int p_height) {
	ERR_FAIL_COND(p_width < 0 || p_height < 0);

	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	_viewport_set_size(viewport, p_width, p_height, 1);
}

void RendererViewport::_viewport_set_size(Viewport *p_viewport, int p_width, int p_height, uint32_t p_view_count) {
	Size2i new_size(p_width, p_height);
	if (p_viewport->size != new_size || p_viewport->view_count != p_view_count) {
		p_viewport->size = new_size;
		p_viewport->view_count = p_view_count;

		RSG::texture_storage->render_target_set_size(p_viewport->render_target, p_width, p_height, p_view_count);
	}
}

void RendererViewport::viewport_set_active(RID p_viewport, bool p_active) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	if (p_active) {
		ERR_FAIL_COND_MSG(active_viewports.has(viewport), "Can't make active a Viewport that is already active.");
		active_viewports.push_back(viewport);
	} else {
		active_viewports.erase(viewport);
	}

	sorted_active_viewports_dirty = true;
}

void RendererViewport::viewport_set_parent_viewport(RID p_viewport, RID p_parent_viewport) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	viewport->parent = p_parent_viewport;
}

void RendererViewport::viewport_set_clear_mode(RID p_viewport, RS::ViewportClearMode p_clear_mode) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	viewport->clear_mode = p_clear_mode;
}

void RendererViewport::viewport_attach_to_screen(RID p_viewport, const Rect2 &p_rect, DisplayServer::WindowID p_screen) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	if (p_screen != DisplayServer::INVALID_WINDOW_ID) {
		// If using OpenGL we can optimize this operation by rendering directly to system_fbo
		// instead of rendering to fbo and copying to system_fbo after
		if (RSG::rasterizer->is_low_end() && viewport->viewport_render_direct_to_screen) {
			RSG::texture_storage->render_target_set_size(viewport->render_target, p_rect.size.x, p_rect.size.y, viewport->view_count);
			RSG::texture_storage->render_target_set_position(viewport->render_target, p_rect.position.x, p_rect.position.y);
		}

		viewport->viewport_to_screen_rect = p_rect;
		viewport->viewport_to_screen = p_screen;
	} else {
		// if render_direct_to_screen was used, reset size and position
		if (RSG::rasterizer->is_low_end() && viewport->viewport_render_direct_to_screen) {
			RSG::texture_storage->render_target_set_position(viewport->render_target, 0, 0);
			RSG::texture_storage->render_target_set_size(viewport->render_target, viewport->size.x, viewport->size.y, viewport->view_count);
		}

		viewport->viewport_to_screen_rect = Rect2();
		viewport->viewport_to_screen = DisplayServer::INVALID_WINDOW_ID;
	}
}

void RendererViewport::viewport_set_render_direct_to_screen(RID p_viewport, bool p_enable) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	if (p_enable == viewport->viewport_render_direct_to_screen) {
		return;
	}

	// if disabled, reset render_target size and position
	if (!p_enable) {
		RSG::texture_storage->render_target_set_position(viewport->render_target, 0, 0);
		RSG::texture_storage->render_target_set_size(viewport->render_target, viewport->size.x, viewport->size.y, viewport->view_count);
	}

	RSG::texture_storage->render_target_set_direct_to_screen(viewport->render_target, p_enable);
	viewport->viewport_render_direct_to_screen = p_enable;

	// if attached to screen already, setup screen size and position, this needs to happen after setting flag to avoid an unnecessary buffer allocation
	if (RSG::rasterizer->is_low_end() && viewport->viewport_to_screen_rect != Rect2() && p_enable) {
		RSG::texture_storage->render_target_set_size(viewport->render_target, viewport->viewport_to_screen_rect.size.x, viewport->viewport_to_screen_rect.size.y, viewport->view_count);
		RSG::texture_storage->render_target_set_position(viewport->render_target, viewport->viewport_to_screen_rect.position.x, viewport->viewport_to_screen_rect.position.y);
	}
}

void RendererViewport::viewport_set_update_mode(RID p_viewport, RS::ViewportUpdateMode p_mode) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	viewport->update_mode = p_mode;
}

RID RendererViewport::viewport_get_render_target(RID p_viewport) const {
	const Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND_V(!viewport, RID());

	return viewport->render_target;
}

RID RendererViewport::viewport_get_texture(RID p_viewport) const {
	const Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND_V(!viewport, RID());

	return RSG::texture_storage->render_target_get_texture(viewport->render_target);
}

void RendererViewport::viewport_set_disable_2d(RID p_viewport, bool p_disable) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	viewport->disable_2d = p_disable;
}

void RendererViewport::viewport_attach_camera(RID p_viewport, RID p_camera) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	viewport->camera = p_camera;
}

void RendererViewport::viewport_attach_canvas(RID p_viewport, RID p_canvas) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	ERR_FAIL_COND(viewport->canvas_map.has(p_canvas));
	RendererCanvasCull::Canvas *canvas = RSG::canvas->canvas_owner.get_or_null(p_canvas);
	ERR_FAIL_COND(!canvas);

	canvas->viewports.insert(p_viewport);
	viewport->canvas_map[p_canvas] = Viewport::CanvasData();
	viewport->canvas_map[p_canvas].layer = 0;
	viewport->canvas_map[p_canvas].sublayer = 0;
	viewport->canvas_map[p_canvas].canvas = canvas;
}

void RendererViewport::viewport_remove_canvas(RID p_viewport, RID p_canvas) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	RendererCanvasCull::Canvas *canvas = RSG::canvas->canvas_owner.get_or_null(p_canvas);
	ERR_FAIL_COND(!canvas);

	viewport->canvas_map.erase(p_canvas);
	canvas->viewports.erase(p_viewport);
}

void RendererViewport::viewport_set_canvas_transform(RID p_viewport, RID p_canvas, const Transform2D &p_offset) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	ERR_FAIL_COND(!viewport->canvas_map.has(p_canvas));
	viewport->canvas_map[p_canvas].transform = p_offset;
}

void RendererViewport::viewport_set_transparent_background(RID p_viewport, bool p_enabled) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	RSG::texture_storage->render_target_set_transparent(viewport->render_target, p_enabled);
	viewport->transparent_bg = p_enabled;
}

void RendererViewport::viewport_set_global_canvas_transform(RID p_viewport, const Transform2D &p_transform) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	viewport->global_transform = p_transform;
}

void RendererViewport::viewport_set_canvas_stacking(RID p_viewport, RID p_canvas, int p_layer, int p_sublayer) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	ERR_FAIL_COND(!viewport->canvas_map.has(p_canvas));
	viewport->canvas_map[p_canvas].layer = p_layer;
	viewport->canvas_map[p_canvas].sublayer = p_sublayer;
}

void RendererViewport::viewport_set_positional_shadow_atlas_size(RID p_viewport, int p_size, bool p_16_bits) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	viewport->shadow_atlas_size = p_size;
	viewport->shadow_atlas_16_bits = p_16_bits;

	RSG::light_storage->shadow_atlas_set_size(viewport->shadow_atlas, viewport->shadow_atlas_size, viewport->shadow_atlas_16_bits);
}

void RendererViewport::viewport_set_positional_shadow_atlas_quadrant_subdivision(RID p_viewport, int p_quadrant, int p_subdiv) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	RSG::light_storage->shadow_atlas_set_quadrant_subdivision(viewport->shadow_atlas, p_quadrant, p_subdiv);
}

void RendererViewport::viewport_set_msaa_2d(RID p_viewport, RS::ViewportMSAA p_msaa) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	if (viewport->msaa_2d == p_msaa) {
		return;
	}
	viewport->msaa_2d = p_msaa;
	RSG::texture_storage->render_target_set_msaa(viewport->render_target, p_msaa);
}

int RendererViewport::viewport_get_render_info(RID p_viewport, RS::ViewportRenderInfoType p_type, RS::ViewportRenderInfo p_info) {
	ERR_FAIL_INDEX_V(p_type, RS::VIEWPORT_RENDER_INFO_TYPE_MAX, -1);
	ERR_FAIL_INDEX_V(p_info, RS::VIEWPORT_RENDER_INFO_MAX, -1);

	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	if (!viewport) {
		return 0; //there should be a lock here..
	}

	return viewport->render_info.info[p_type][p_info];
}

void RendererViewport::viewport_set_debug_draw(RID p_viewport, RS::ViewportDebugDraw p_draw) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	viewport->debug_draw = p_draw;
}

void RendererViewport::viewport_set_measure_render_time(RID p_viewport, bool p_enable) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	viewport->measure_render_time = p_enable;
}

float RendererViewport::viewport_get_measured_render_time_cpu(RID p_viewport) const {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND_V(!viewport, 0);

	return double(viewport->time_cpu_end - viewport->time_cpu_begin) / 1000.0;
}

float RendererViewport::viewport_get_measured_render_time_gpu(RID p_viewport) const {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND_V(!viewport, 0);

	return double((viewport->time_gpu_end - viewport->time_gpu_begin) / 1000) / 1000.0;
}

void RendererViewport::viewport_set_snap_2d_transforms_to_pixel(RID p_viewport, bool p_enabled) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);
	viewport->snap_2d_transforms_to_pixel = p_enabled;
}

void RendererViewport::viewport_set_snap_2d_vertices_to_pixel(RID p_viewport, bool p_enabled) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);
	viewport->snap_2d_vertices_to_pixel = p_enabled;
}

void RendererViewport::viewport_set_default_canvas_item_texture_filter(RID p_viewport, RS::CanvasItemTextureFilter p_filter) {
	ERR_FAIL_COND_MSG(p_filter == RS::CANVAS_ITEM_TEXTURE_FILTER_DEFAULT, "Viewport does not accept DEFAULT as texture filter (it's the topmost choice already).)");
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	viewport->texture_filter = p_filter;
}
void RendererViewport::viewport_set_default_canvas_item_texture_repeat(RID p_viewport, RS::CanvasItemTextureRepeat p_repeat) {
	ERR_FAIL_COND_MSG(p_repeat == RS::CANVAS_ITEM_TEXTURE_REPEAT_DEFAULT, "Viewport does not accept DEFAULT as texture repeat (it's the topmost choice already).)");
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	viewport->texture_repeat = p_repeat;
}

void RendererViewport::viewport_set_sdf_oversize_and_scale(RID p_viewport, RS::ViewportSDFOversize p_size, RS::ViewportSDFScale p_scale) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);

	RSG::texture_storage->render_target_set_sdf_size_and_scale(viewport->render_target, p_size, p_scale);
}

RID RendererViewport::viewport_find_from_screen_attachment(DisplayServer::WindowID p_id) const {
	RID *rids = nullptr;
	uint32_t rid_count = viewport_owner.get_rid_count();
	rids = (RID *)alloca(sizeof(RID *) * rid_count);
	viewport_owner.fill_owned_buffer(rids);
	for (uint32_t i = 0; i < rid_count; i++) {
		Viewport *viewport = viewport_owner.get_or_null(rids[i]);
		if (viewport->viewport_to_screen == p_id) {
			return rids[i];
		}
	}
	return RID();
}

bool RendererViewport::free(RID p_rid) {
	if (viewport_owner.owns(p_rid)) {
		Viewport *viewport = viewport_owner.get_or_null(p_rid);

		RSG::texture_storage->render_target_free(viewport->render_target);
		RSG::light_storage->shadow_atlas_free(viewport->shadow_atlas);
		if (viewport->render_buffers.is_valid()) {
			viewport->render_buffers.unref();
		}

		while (viewport->canvas_map.begin()) {
			viewport_remove_canvas(p_rid, viewport->canvas_map.begin()->key);
		}

		active_viewports.erase(viewport);
		sorted_active_viewports_dirty = true;

		viewport_owner.free(p_rid);

		return true;
	}

	return false;
}

void RendererViewport::handle_timestamp(String p_timestamp, uint64_t p_cpu_time, uint64_t p_gpu_time) {
	RID *vp = timestamp_vp_map.getptr(p_timestamp);
	if (!vp) {
		return;
	}

	Viewport *viewport = viewport_owner.get_or_null(*vp);
	if (!viewport) {
		return;
	}

	if (p_timestamp.begins_with("vp_begin")) {
		viewport->time_cpu_begin = p_cpu_time;
		viewport->time_gpu_begin = p_gpu_time;
	}

	if (p_timestamp.begins_with("vp_end")) {
		viewport->time_cpu_end = p_cpu_time;
		viewport->time_gpu_end = p_gpu_time;
	}
}

void RendererViewport::set_default_clear_color(const Color &p_color) {
	RSG::texture_storage->set_default_clear_color(p_color);
}

void RendererViewport::viewport_set_canvas_cull_mask(RID p_viewport, uint32_t p_canvas_cull_mask) {
	Viewport *viewport = viewport_owner.get_or_null(p_viewport);
	ERR_FAIL_COND(!viewport);
	viewport->canvas_cull_mask = p_canvas_cull_mask;
}

// Workaround for setting this on thread.
void RendererViewport::call_set_vsync_mode(DisplayServer::VSyncMode p_mode, DisplayServer::WindowID p_window) {
	DisplayServer::get_singleton()->window_set_vsync_mode(p_mode, p_window);
}

int RendererViewport::get_total_objects_drawn() const {
	return total_objects_drawn;
}
int RendererViewport::get_total_primitives_drawn() const {
	return total_vertices_drawn;
}
int RendererViewport::get_total_draw_calls_used() const {
	return total_draw_calls_used;
}

RendererViewport::RendererViewport() {
}
