/**************************************************************************/
/*  rendering_server_default.h                                            */
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

#ifndef RENDERING_SERVER_DEFAULT_H
#define RENDERING_SERVER_DEFAULT_H

#include "core/os/thread.h"
#include "core/templates/command_queue_mt.h"
#include "core/templates/hash_map.h"
#include "renderer_canvas_cull.h"
#include "renderer_scene_cull.h"
#include "renderer_viewport.h"
#include "rendering_server_globals.h"
#include "servers/rendering/renderer_compositor.h"
#include "servers/rendering_server.h"
#include "servers/server_wrap_mt_common.h"

class RenderingServerDefault : public RenderingServer {
	enum {
		MAX_INSTANCE_CULL = 8192,
		LIGHT_CACHE_DIRTY = -1,
		MAX_LIGHTS_CULLED = 256,
		MAX_ROOM_CULL = 32,
		MAX_EXTERIOR_PORTALS = 128,
		MAX_LIGHT_SAMPLERS = 256,
		INSTANCE_ROOMLESS_MASK = (1 << 20)

	};

	static int changes;

	List<Callable> frame_drawn_callbacks;

	static void _changes_changed() {}

	uint64_t frame_profile_frame;
	Vector<FrameProfileArea> frame_profile;

	double frame_setup_time = 0;

	//for printing
	bool print_gpu_profile = false;
	HashMap<String, float> print_gpu_profile_task_time;
	uint64_t print_frame_profile_ticks_from = 0;
	uint32_t print_frame_profile_frame_count = 0;

	mutable CommandQueueMT command_queue;

	static void _thread_callback(void *_instance);
	void _thread_loop();

	Thread::ID server_thread;
	SafeFlag exit;
	Thread thread;
	SafeFlag draw_thread_up;
	bool create_thread;

	void _thread_draw(bool p_swap_buffers, double frame_step);
	void _thread_flush();

	void _thread_exit();

	Mutex alloc_mutex;

	void _draw(bool p_swap_buffers, double frame_step);
	void _init();
	void _finish();

	void _free(RID p_rid);

	void _call_on_render_thread(const Callable &p_callable);

public:
	//if editor is redrawing when it shouldn't, enable this and put a breakpoint in _changes_changed()
	//#define DEBUG_CHANGES

#ifdef DEBUG_CHANGES
	_FORCE_INLINE_ static void redraw_request() {
		changes++;
		_changes_changed();
	}

#define DISPLAY_CHANGED \
	changes++;          \
	_changes_changed();

#else
	_FORCE_INLINE_ static void redraw_request() {
		changes++;
	}
#endif

#define WRITE_ACTION redraw_request();

#ifdef DEBUG_SYNC
#define SYNC_DEBUG print_line("sync on: " + String(__FUNCTION__));
#else
#define SYNC_DEBUG
#endif

#include "servers/server_wrap_mt_common.h"

	/* TEXTURE API */

#define ServerName RendererTextureStorage
#define server_name RSG::texture_storage

#define FUNCRIDTEX0(m_type)                                                                                   \
	virtual RID m_type##_create() override {                                                                  \
		RID ret = RSG::texture_storage->texture_allocate();                                                   \
		if (Thread::get_caller_id() == server_thread || RSG::texture_storage->can_create_resources_async()) { \
			RSG::texture_storage->m_type##_initialize(ret);                                                   \
		} else {                                                                                              \
			command_queue.push(RSG::texture_storage, &RendererTextureStorage::m_type##_initialize, ret);      \
		}                                                                                                     \
		return ret;                                                                                           \
	}

#define FUNCRIDTEX1(m_type, m_type1)                                                                          \
	virtual RID m_type##_create(m_type1 p1) override {                                                        \
		RID ret = RSG::texture_storage->texture_allocate();                                                   \
		if (Thread::get_caller_id() == server_thread || RSG::texture_storage->can_create_resources_async()) { \
			RSG::texture_storage->m_type##_initialize(ret, p1);                                               \
		} else {                                                                                              \
			command_queue.push(RSG::texture_storage, &RendererTextureStorage::m_type##_initialize, ret, p1);  \
		}                                                                                                     \
		return ret;                                                                                           \
	}

#define FUNCRIDTEX2(m_type, m_type1, m_type2)                                                                    \
	virtual RID m_type##_create(m_type1 p1, m_type2 p2) override {                                               \
		RID ret = RSG::texture_storage->texture_allocate();                                                      \
		if (Thread::get_caller_id() == server_thread || RSG::texture_storage->can_create_resources_async()) {    \
			RSG::texture_storage->m_type##_initialize(ret, p1, p2);                                              \
		} else {                                                                                                 \
			command_queue.push(RSG::texture_storage, &RendererTextureStorage::m_type##_initialize, ret, p1, p2); \
		}                                                                                                        \
		return ret;                                                                                              \
	}

#define FUNCRIDTEX6(m_type, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6)                                                \
	virtual RID m_type##_create(m_type1 p1, m_type2 p2, m_type3 p3, m_type4 p4, m_type5 p5, m_type6 p6) override {               \
		RID ret = RSG::texture_storage->texture_allocate();                                                                      \
		if (Thread::get_caller_id() == server_thread || RSG::texture_storage->can_create_resources_async()) {                    \
			RSG::texture_storage->m_type##_initialize(ret, p1, p2, p3, p4, p5, p6);                                              \
		} else {                                                                                                                 \
			command_queue.push(RSG::texture_storage, &RendererTextureStorage::m_type##_initialize, ret, p1, p2, p3, p4, p5, p6); \
		}                                                                                                                        \
		return ret;                                                                                                              \
	}

	//these go pass-through, as they can be called from any thread
	FUNCRIDTEX1(texture_2d, const Ref<Image> &)
	FUNCRIDTEX2(texture_2d_layered, const Vector<Ref<Image>> &, TextureLayeredType)
	FUNCRIDTEX6(texture_3d, Image::Format, int, int, int, bool, const Vector<Ref<Image>> &)
	FUNCRIDTEX1(texture_proxy, RID)

	//these go through command queue if they are in another thread
	FUNC3(texture_2d_update, RID, const Ref<Image> &, int)
	FUNC2(texture_3d_update, RID, const Vector<Ref<Image>> &)
	FUNC2(texture_proxy_update, RID, RID)

	//these also go pass-through
	FUNCRIDTEX0(texture_2d_placeholder)
	FUNCRIDTEX1(texture_2d_layered_placeholder, TextureLayeredType)
	FUNCRIDTEX0(texture_3d_placeholder)

	FUNC1RC(Ref<Image>, texture_2d_get, RID)
	FUNC2RC(Ref<Image>, texture_2d_layer_get, RID, int)
	FUNC1RC(Vector<Ref<Image>>, texture_3d_get, RID)

	FUNC2(texture_replace, RID, RID)

	FUNC3(texture_set_size_override, RID, int, int)
// FIXME: Disabled during Vulkan refactoring, should be ported.
#if 0
	FUNC2(texture_bind, RID, uint32_t)
#endif

	FUNC3(texture_set_detect_3d_callback, RID, TextureDetectCallback, void *)
	FUNC3(texture_set_detect_normal_callback, RID, TextureDetectCallback, void *)
	FUNC3(texture_set_detect_roughness_callback, RID, TextureDetectRoughnessCallback, void *)

	FUNC2(texture_set_path, RID, const String &)
	FUNC1RC(String, texture_get_path, RID)

	FUNC1RC(Image::Format, texture_get_format, RID)

	FUNC1(texture_debug_usage, List<TextureInfo> *)

	FUNC2(texture_set_force_redraw_if_visible, RID, bool)
	FUNCRIDTEX2(texture_rd, const RID &, const RS::TextureLayeredType)
	FUNC2RC(RID, texture_get_rd_texture, RID, bool)
	FUNC2RC(uint64_t, texture_get_native_handle, RID, bool)

	/* SHADER API */

#undef ServerName
#undef server_name

#define ServerName RendererMaterialStorage
#define server_name RSG::material_storage

	FUNCRIDSPLIT(shader)

	FUNC2(shader_set_code, RID, const String &)
	FUNC2(shader_set_path_hint, RID, const String &)
	FUNC1RC(String, shader_get_code, RID)

	FUNC2SC(get_shader_parameter_list, RID, List<PropertyInfo> *)

	FUNC4(shader_set_default_texture_parameter, RID, const StringName &, RID, int)
	FUNC3RC(RID, shader_get_default_texture_parameter, RID, const StringName &, int)
	FUNC2RC(Variant, shader_get_parameter_default, RID, const StringName &)

	FUNC1RC(ShaderNativeSourceCode, shader_get_native_source_code, RID)

	/* COMMON MATERIAL API */

	FUNCRIDSPLIT(material)

	FUNC2(material_set_shader, RID, RID)

	FUNC3(material_set_param, RID, const StringName &, const Variant &)
	FUNC2RC(Variant, material_get_param, RID, const StringName &)

	FUNC2(material_set_render_priority, RID, int)
	FUNC2(material_set_next_pass, RID, RID)

	/* MESH API */

//from now on, calls forwarded to this singleton
#undef ServerName
#undef server_name

#define ServerName RendererMeshStorage
#define server_name RSG::mesh_storage

	virtual RID mesh_create_from_surfaces(const Vector<SurfaceData> &p_surfaces, int p_blend_shape_count = 0) override {
		RID mesh = RSG::mesh_storage->mesh_allocate();

		// TODO once we have RSG::mesh_storage, add can_create_resources_async and call here instead of texture_storage!!

		if (Thread::get_caller_id() == server_thread || RSG::texture_storage->can_create_resources_async()) {
			if (Thread::get_caller_id() == server_thread) {
				command_queue.flush_if_pending();
			}
			RSG::mesh_storage->mesh_initialize(mesh);
			RSG::mesh_storage->mesh_set_blend_shape_count(mesh, p_blend_shape_count);
			for (int i = 0; i < p_surfaces.size(); i++) {
				RSG::mesh_storage->mesh_add_surface(mesh, p_surfaces[i]);
			}
		} else {
			command_queue.push(RSG::mesh_storage, &RendererMeshStorage::mesh_initialize, mesh);
			command_queue.push(RSG::mesh_storage, &RendererMeshStorage::mesh_set_blend_shape_count, mesh, p_blend_shape_count);
			for (int i = 0; i < p_surfaces.size(); i++) {
				command_queue.push(RSG::mesh_storage, &RendererMeshStorage::mesh_add_surface, mesh, p_surfaces[i]);
			}
		}

		return mesh;
	}

	FUNC2(mesh_set_blend_shape_count, RID, int)

	FUNCRIDSPLIT(mesh)

	FUNC2(mesh_add_surface, RID, const SurfaceData &)

	FUNC1RC(int, mesh_get_blend_shape_count, RID)

	FUNC2(mesh_set_blend_shape_mode, RID, BlendShapeMode)
	FUNC1RC(BlendShapeMode, mesh_get_blend_shape_mode, RID)

	FUNC4(mesh_surface_update_vertex_region, RID, int, int, const Vector<uint8_t> &)
	FUNC4(mesh_surface_update_attribute_region, RID, int, int, const Vector<uint8_t> &)
	FUNC4(mesh_surface_update_skin_region, RID, int, int, const Vector<uint8_t> &)

	FUNC3(mesh_surface_set_material, RID, int, RID)
	FUNC2RC(RID, mesh_surface_get_material, RID, int)

	FUNC2RC(SurfaceData, mesh_get_surface, RID, int)

	FUNC1RC(int, mesh_get_surface_count, RID)

	FUNC2(mesh_set_custom_aabb, RID, const AABB &)
	FUNC1RC(AABB, mesh_get_custom_aabb, RID)

	FUNC2(mesh_set_shadow_mesh, RID, RID)

	FUNC1(mesh_clear, RID)

	/* MULTIMESH API */

	FUNCRIDSPLIT(multimesh)

	FUNC5(multimesh_allocate_data, RID, int, MultimeshTransformFormat, bool, bool)
	FUNC1RC(int, multimesh_get_instance_count, RID)

	FUNC2(multimesh_set_mesh, RID, RID)
	FUNC3(multimesh_instance_set_transform, RID, int, const Transform3D &)
	FUNC3(multimesh_instance_set_transform_2d, RID, int, const Transform2D &)
	FUNC3(multimesh_instance_set_color, RID, int, const Color &)
	FUNC3(multimesh_instance_set_custom_data, RID, int, const Color &)

	FUNC1RC(RID, multimesh_get_mesh, RID)
	FUNC1RC(AABB, multimesh_get_aabb, RID)

	FUNC2RC(Transform3D, multimesh_instance_get_transform, RID, int)
	FUNC2RC(Transform2D, multimesh_instance_get_transform_2d, RID, int)
	FUNC2RC(Color, multimesh_instance_get_color, RID, int)
	FUNC2RC(Color, multimesh_instance_get_custom_data, RID, int)

	FUNC2(multimesh_set_buffer, RID, const Vector<float> &)
	FUNC1RC(Vector<float>, multimesh_get_buffer, RID)

	FUNC2(multimesh_set_visible_instances, RID, int)
	FUNC1RC(int, multimesh_get_visible_instances, RID)

	/* SKELETON API */

	FUNCRIDSPLIT(skeleton)
	FUNC3(skeleton_allocate_data, RID, int, bool)
	FUNC1RC(int, skeleton_get_bone_count, RID)
	FUNC3(skeleton_bone_set_transform_2d, RID, int, const Transform2D &)
	FUNC2RC(Transform2D, skeleton_bone_get_transform_2d, RID, int)
	FUNC2(skeleton_set_base_transform_2d, RID, const Transform2D &)

	/* PARTICLES */

#undef ServerName
#undef server_name

#define ServerName RendererParticlesStorage
#define server_name RSG::particles_storage

	FUNCRIDSPLIT(particles)

	FUNC2(particles_set_mode, RID, ParticlesMode)
	FUNC2(particles_set_emitting, RID, bool)
	FUNC1R(bool, particles_get_emitting, RID)
	FUNC2(particles_set_amount, RID, int)
	FUNC2(particles_set_lifetime, RID, double)
	FUNC2(particles_set_one_shot, RID, bool)
	FUNC2(particles_set_pre_process_time, RID, double)
	FUNC2(particles_set_explosiveness_ratio, RID, float)
	FUNC2(particles_set_randomness_ratio, RID, float)
	FUNC2(particles_set_custom_aabb, RID, const AABB &)
	FUNC2(particles_set_speed_scale, RID, double)
	FUNC2(particles_set_use_local_coordinates, RID, bool)
	FUNC2(particles_set_process_material, RID, RID)
	FUNC2(particles_set_fixed_fps, RID, int)
	FUNC2(particles_set_interpolate, RID, bool)
	FUNC2(particles_set_fractional_delta, RID, bool)
	FUNC1R(bool, particles_is_inactive, RID)
	FUNC3(particles_set_trails, RID, bool, float)
	FUNC2(particles_set_trail_bind_poses, RID, const Vector<Transform3D> &)

	FUNC1(particles_request_process, RID)
	FUNC1(particles_restart, RID)
	FUNC6(particles_emit, RID, const Transform3D &, const Vector3 &, const Color &, const Color &, uint32_t)
	FUNC2(particles_set_subemitter, RID, RID)
	FUNC2(particles_set_collision_base_size, RID, float)

	FUNC2(particles_set_transform_align, RID, RS::ParticlesTransformAlign)

	FUNC2(particles_set_draw_order, RID, RS::ParticlesDrawOrder)

	FUNC2(particles_set_draw_passes, RID, int)
	FUNC3(particles_set_draw_pass_mesh, RID, int, RID)

	FUNC1R(AABB, particles_get_current_aabb, RID)
	FUNC2(particles_set_emission_transform, RID, const Transform3D &)

	/* PARTICLES COLLISION */

	FUNCRIDSPLIT(particles_collision)

	FUNC2(particles_collision_set_collision_type, RID, ParticlesCollisionType)
	FUNC2(particles_collision_set_cull_mask, RID, uint32_t)
	FUNC2(particles_collision_set_sphere_radius, RID, real_t)
	FUNC2(particles_collision_set_box_extents, RID, const Vector3 &)
	FUNC2(particles_collision_set_attractor_strength, RID, real_t)
	FUNC2(particles_collision_set_attractor_directionality, RID, real_t)
	FUNC2(particles_collision_set_attractor_attenuation, RID, real_t)
	FUNC2(particles_collision_set_field_texture, RID, RID)
	FUNC1(particles_collision_height_field_update, RID)
	FUNC2(particles_collision_set_height_field_resolution, RID, ParticlesCollisionHeightfieldResolution)

#undef server_name
#undef ServerName
//from now on, calls forwarded to this singleton
#define ServerName RendererViewport
#define server_name RSG::viewport

	/* VIEWPORT TARGET API */

	FUNCRIDSPLIT(viewport)

	FUNC3(viewport_set_size, RID, int, int)

	FUNC2(viewport_set_active, RID, bool)
	FUNC2(viewport_set_parent_viewport, RID, RID)

	FUNC2(viewport_set_clear_mode, RID, ViewportClearMode)

	FUNC3(viewport_attach_to_screen, RID, const Rect2 &, int)
	FUNC2(viewport_set_render_direct_to_screen, RID, bool)

	FUNC2(viewport_set_update_mode, RID, ViewportUpdateMode)

	FUNC1RC(RID, viewport_get_render_target, RID)
	FUNC1RC(RID, viewport_get_texture, RID)

	FUNC2(viewport_set_disable_2d, RID, bool)

	FUNC2(viewport_set_canvas_cull_mask, RID, uint32_t)

	FUNC2(viewport_attach_camera, RID, RID)
	FUNC2(viewport_attach_canvas, RID, RID)

	FUNC2(viewport_remove_canvas, RID, RID)
	FUNC3(viewport_set_canvas_transform, RID, RID, const Transform2D &)
	FUNC2(viewport_set_transparent_background, RID, bool)
	FUNC2(viewport_set_snap_2d_transforms_to_pixel, RID, bool)
	FUNC2(viewport_set_snap_2d_vertices_to_pixel, RID, bool)

	FUNC2(viewport_set_default_canvas_item_texture_filter, RID, CanvasItemTextureFilter)
	FUNC2(viewport_set_default_canvas_item_texture_repeat, RID, CanvasItemTextureRepeat)

	FUNC2(viewport_set_global_canvas_transform, RID, const Transform2D &)
	FUNC4(viewport_set_canvas_stacking, RID, RID, int, int)
	FUNC3(viewport_set_sdf_oversize_and_scale, RID, ViewportSDFOversize, ViewportSDFScale)
	FUNC2(viewport_set_msaa_2d, RID, ViewportMSAA)

	FUNC3R(int, viewport_get_render_info, RID, ViewportRenderInfoType, ViewportRenderInfo)
	FUNC2(viewport_set_debug_draw, RID, ViewportDebugDraw)

	FUNC2(viewport_set_measure_render_time, RID, bool)
	FUNC1RC(double, viewport_get_measured_render_time_cpu, RID)
	FUNC1RC(double, viewport_get_measured_render_time_gpu, RID)
	FUNC1RC(RID, viewport_find_from_screen_attachment, DisplayServer::WindowID)

	FUNC2(call_set_vsync_mode, DisplayServer::VSyncMode, DisplayServer::WindowID)

	/* SCENARIO API */

#undef server_name
#undef ServerName

#define ServerName RenderingMethod
#define server_name RSG::scene

	/* INSTANCING API */
	FUNCRIDSPLIT(instance)

	FUNC2(instance_set_layer_mask, RID, uint32_t)
	FUNC3(instance_set_pivot_data, RID, float, bool)
	FUNC2(instance_set_transform, RID, const Transform3D &)
	FUNC2(instance_attach_object_instance_id, RID, ObjectID)
	FUNC3(instance_set_blend_shape_weight, RID, int, float)
	FUNC3(instance_set_surface_override_material, RID, int, RID)
	FUNC2(instance_set_visible, RID, bool)

	FUNC2(instance_attach_skeleton, RID, RID)

	FUNC2(instance_set_extra_visibility_margin, RID, real_t)
	FUNC2(instance_set_visibility_parent, RID, RID)

	FUNC2(instance_set_ignore_culling, RID, bool)

	// don't use these in a game!

	FUNC3(instance_geometry_set_flag, RID, InstanceFlags, bool)
	FUNC2(instance_geometry_set_cast_shadows_setting, RID, ShadowCastingSetting)
	FUNC2(instance_geometry_set_material_override, RID, RID)
	FUNC2(instance_geometry_set_material_overlay, RID, RID)

	FUNC6(instance_geometry_set_visibility_range, RID, float, float, float, float, VisibilityRangeFadeMode)
	FUNC2(instance_geometry_set_lod_bias, RID, float)
	FUNC2(instance_geometry_set_transparency, RID, float)
	FUNC3(instance_geometry_set_shader_parameter, RID, const StringName &, const Variant &)
	FUNC2RC(Variant, instance_geometry_get_shader_parameter, RID, const StringName &)
	FUNC2RC(Variant, instance_geometry_get_shader_parameter_default_value, RID, const StringName &)

#undef server_name
#undef ServerName
//from now on, calls forwarded to this singleton
#define ServerName RendererCanvasCull
#define server_name RSG::canvas

	/* CANVAS (2D) */

	FUNCRIDSPLIT(canvas)
	FUNC3(canvas_set_item_mirroring, RID, RID, const Point2 &)
	FUNC2(canvas_set_modulate, RID, const Color &)
	FUNC3(canvas_set_parent, RID, RID, float)
	FUNC1(canvas_set_disable_scale, bool)

	FUNCRIDSPLIT(canvas_texture)
	FUNC3(canvas_texture_set_channel, RID, CanvasTextureChannel, RID)
	FUNC3(canvas_texture_set_shading_parameters, RID, const Color &, float)

	FUNC2(canvas_texture_set_texture_filter, RID, CanvasItemTextureFilter)
	FUNC2(canvas_texture_set_texture_repeat, RID, CanvasItemTextureRepeat)

	FUNCRIDSPLIT(canvas_item)
	FUNC2(canvas_item_set_parent, RID, RID)

	FUNC2(canvas_item_set_default_texture_filter, RID, CanvasItemTextureFilter)
	FUNC2(canvas_item_set_default_texture_repeat, RID, CanvasItemTextureRepeat)

	FUNC2(canvas_item_set_visible, RID, bool)
	FUNC2(canvas_item_set_light_mask, RID, int)

	FUNC2(canvas_item_set_visibility_layer, RID, uint32_t)

	FUNC2(canvas_item_set_update_when_visible, RID, bool)

	FUNC2(canvas_item_set_transform, RID, const Transform2D &)
	FUNC2(canvas_item_set_clip, RID, bool)
	FUNC2(canvas_item_set_distance_field_mode, RID, bool)
	FUNC3(canvas_item_set_custom_rect, RID, bool, const Rect2 &)
	FUNC2(canvas_item_set_modulate, RID, const Color &)
	FUNC2(canvas_item_set_self_modulate, RID, const Color &)

	FUNC2(canvas_item_set_draw_behind_parent, RID, bool)

	FUNC6(canvas_item_add_line, RID, const Point2 &, const Point2 &, const Color &, float, bool)
	FUNC5(canvas_item_add_polyline, RID, const Vector<Point2> &, const Vector<Color> &, float, bool)
	FUNC4(canvas_item_add_multiline, RID, const Vector<Point2> &, const Vector<Color> &, float)
	FUNC3(canvas_item_add_rect, RID, const Rect2 &, const Color &)
	FUNC4(canvas_item_add_circle, RID, const Point2 &, float, const Color &)
	FUNC6(canvas_item_add_texture_rect, RID, const Rect2 &, RID, bool, const Color &, bool)
	FUNC7(canvas_item_add_texture_rect_region, RID, const Rect2 &, RID, const Rect2 &, const Color &, bool, bool)
	FUNC8(canvas_item_add_msdf_texture_rect_region, RID, const Rect2 &, RID, const Rect2 &, const Color &, int, float, float)
	FUNC5(canvas_item_add_lcd_texture_rect_region, RID, const Rect2 &, RID, const Rect2 &, const Color &)
	FUNC10(canvas_item_add_nine_patch, RID, const Rect2 &, const Rect2 &, RID, const Vector2 &, const Vector2 &, NinePatchAxisMode, NinePatchAxisMode, bool, const Color &)
	FUNC5(canvas_item_add_primitive, RID, const Vector<Point2> &, const Vector<Color> &, const Vector<Point2> &, RID)
	FUNC5(canvas_item_add_polygon, RID, const Vector<Point2> &, const Vector<Color> &, const Vector<Point2> &, RID)
	FUNC9(canvas_item_add_triangle_array, RID, const Vector<int> &, const Vector<Point2> &, const Vector<Color> &, const Vector<Point2> &, const Vector<int> &, const Vector<float> &, RID, int)
	FUNC5(canvas_item_add_mesh, RID, const RID &, const Transform2D &, const Color &, RID)
	FUNC3(canvas_item_add_multimesh, RID, RID, RID)
	FUNC3(canvas_item_add_particles, RID, RID, RID)
	FUNC2(canvas_item_add_set_transform, RID, const Transform2D &)
	FUNC2(canvas_item_add_clip_ignore, RID, bool)
	FUNC5(canvas_item_add_animation_slice, RID, double, double, double, double)

	FUNC2(canvas_item_set_sort_children_by_y, RID, bool)
	FUNC2(canvas_item_set_z_index, RID, int)
	FUNC2(canvas_item_set_z_as_relative_to_parent, RID, bool)
	FUNC3(canvas_item_set_copy_to_backbuffer, RID, bool, const Rect2 &)
	FUNC2(canvas_item_attach_skeleton, RID, RID)

	FUNC1(canvas_item_clear, RID)
	FUNC2(canvas_item_set_draw_index, RID, int)

	FUNC2(canvas_item_set_material, RID, RID)

	FUNC2(canvas_item_set_use_parent_material, RID, bool)

	FUNC5(canvas_item_set_visibility_notifier, RID, bool, const Rect2 &, const Callable &, const Callable &)

	FUNC6(canvas_item_set_canvas_group_mode, RID, CanvasGroupMode, float, bool, float, bool)

	FUNCRIDSPLIT(canvas_light)

	FUNC2(canvas_light_set_mode, RID, CanvasLightMode)

	FUNC2(canvas_light_attach_to_canvas, RID, RID)
	FUNC2(canvas_light_set_enabled, RID, bool)
	FUNC2(canvas_light_set_texture_scale, RID, float)
	FUNC2(canvas_light_set_transform, RID, const Transform2D &)
	FUNC2(canvas_light_set_texture, RID, RID)
	FUNC2(canvas_light_set_texture_offset, RID, const Vector2 &)
	FUNC2(canvas_light_set_color, RID, const Color &)
	FUNC2(canvas_light_set_height, RID, float)
	FUNC2(canvas_light_set_energy, RID, float)
	FUNC3(canvas_light_set_z_range, RID, int, int)
	FUNC3(canvas_light_set_layer_range, RID, int, int)
	FUNC2(canvas_light_set_item_cull_mask, RID, int)
	FUNC2(canvas_light_set_item_shadow_cull_mask, RID, int)
	FUNC2(canvas_light_set_directional_distance, RID, float)

	FUNC2(canvas_light_set_blend_mode, RID, CanvasLightBlendMode)

	FUNC2(canvas_light_set_shadow_enabled, RID, bool)
	FUNC2(canvas_light_set_shadow_filter, RID, CanvasLightShadowFilter)
	FUNC2(canvas_light_set_shadow_color, RID, const Color &)
	FUNC2(canvas_light_set_shadow_smooth, RID, float)

	FUNCRIDSPLIT(canvas_light_occluder)
	FUNC2(canvas_light_occluder_attach_to_canvas, RID, RID)
	FUNC2(canvas_light_occluder_set_enabled, RID, bool)
	FUNC2(canvas_light_occluder_set_polygon, RID, RID)
	FUNC2(canvas_light_occluder_set_as_sdf_collision, RID, bool)
	FUNC2(canvas_light_occluder_set_transform, RID, const Transform2D &)
	FUNC2(canvas_light_occluder_set_light_mask, RID, int)

	FUNCRIDSPLIT(canvas_occluder_polygon)
	FUNC3(canvas_occluder_polygon_set_shape, RID, const Vector<Vector2> &, bool)

	FUNC2(canvas_occluder_polygon_set_cull_mode, RID, CanvasOccluderPolygonCullMode)

	FUNC1(canvas_set_shadow_texture_size, int)

	/* GLOBAL SHADER UNIFORMS */

#undef server_name
#undef ServerName
//from now on, calls forwarded to this singleton
#define ServerName RendererMaterialStorage
#define server_name RSG::material_storage

	FUNC3(global_shader_parameter_add, const StringName &, GlobalShaderParameterType, const Variant &)
	FUNC1(global_shader_parameter_remove, const StringName &)
	FUNC0RC(Vector<StringName>, global_shader_parameter_get_list)
	FUNC2(global_shader_parameter_set, const StringName &, const Variant &)
	FUNC2(global_shader_parameter_set_override, const StringName &, const Variant &)
	FUNC1RC(GlobalShaderParameterType, global_shader_parameter_get_type, const StringName &)
	FUNC1RC(Variant, global_shader_parameter_get, const StringName &)

	FUNC1(global_shader_parameters_load_settings, bool)
	FUNC0(global_shader_parameters_clear)

#undef server_name
#undef ServerName
	/* STATUS INFORMATION */
#define ServerName RendererUtilities
#define server_name RSG::utilities
	FUNC0RC(String, get_video_adapter_name)
	FUNC0RC(String, get_video_adapter_vendor)
	FUNC0RC(String, get_video_adapter_api_version)
#undef server_name
#undef ServerName
#undef WRITE_ACTION
#undef SYNC_DEBUG

	virtual uint64_t get_rendering_info(RenderingInfo p_info) override;
	virtual RenderingDevice::DeviceType get_video_adapter_type() const override;

	virtual void set_frame_profiling_enabled(bool p_enable) override;
	virtual Vector<FrameProfileArea> get_frame_profile() override;
	virtual uint64_t get_frame_profile_frame() override;

	/* FREE */

	virtual void free(RID p_rid) override {
		if (Thread::get_caller_id() == server_thread) {
			command_queue.flush_if_pending();
			_free(p_rid);
		} else {
			command_queue.push(this, &RenderingServerDefault::_free, p_rid);
		}
	}

	/* EVENT QUEUING */

	virtual void request_frame_drawn_callback(const Callable &p_callable) override;

	virtual void draw(bool p_swap_buffers, double frame_step) override;
	virtual void sync() override;
	virtual bool has_changed() const override;
	virtual void init() override;
	virtual void finish() override;

	virtual void call_on_render_thread(const Callable &p_callable) override {
		if (Thread::get_caller_id() == server_thread) {
			command_queue.flush_if_pending();
			_call_on_render_thread(p_callable);
		} else {
			command_queue.push(this, &RenderingServerDefault::_call_on_render_thread, p_callable);
		}
	}

	/* TESTING */

	virtual double get_frame_setup_time_cpu() const override;

	virtual void set_boot_image(const Ref<Image> &p_image, const Color &p_color, bool p_scale, bool p_use_filter = true) override;
	virtual Color get_default_clear_color() override;
	virtual void set_default_clear_color(const Color &p_color) override;

	virtual bool has_feature(Features p_feature) const override;

	virtual bool has_os_feature(const String &p_feature) const override;
	virtual void set_debug_generate_wireframes(bool p_generate) override;

	virtual bool is_low_end() const override;

	virtual void set_print_gpu_profile(bool p_enable) override;

	virtual Size2i get_maximum_viewport_size() const override;

	RenderingServerDefault(bool p_create_thread = false);
	~RenderingServerDefault();
};

#endif // RENDERING_SERVER_DEFAULT_H
