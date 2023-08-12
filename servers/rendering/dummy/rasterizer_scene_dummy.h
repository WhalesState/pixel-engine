/**************************************************************************/
/*  rasterizer_scene_dummy.h                                              */
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

#ifndef RASTERIZER_SCENE_DUMMY_H
#define RASTERIZER_SCENE_DUMMY_H

#include "core/templates/paged_allocator.h"
#include "servers/rendering/renderer_scene_render.h"
#include "storage/utilities.h"

class RasterizerSceneDummy : public RendererSceneRender {
public:
	class GeometryInstanceDummy : public RenderGeometryInstance {
	public:
		GeometryInstanceDummy() {}

		virtual void _mark_dirty() override {}

		virtual void set_skeleton(RID p_skeleton) override {}
		virtual void set_material_override(RID p_override) override {}
		virtual void set_material_overlay(RID p_overlay) override {}
		virtual void set_surface_materials(const Vector<RID> &p_materials) override {}
		virtual void set_mesh_instance(RID p_mesh_instance) override {}
		virtual void set_transform(const Transform3D &p_transform, const AABB &p_aabb, const AABB &p_transformed_aabb) override {}
		virtual void set_pivot_data(float p_sorting_offset, bool p_use_aabb_center) override {}
		virtual void set_lod_bias(float p_lod_bias) override {}
		virtual void set_layer_mask(uint32_t p_layer_mask) override {}
		virtual void set_fade_range(bool p_enable_near, float p_near_begin, float p_near_end, bool p_enable_far, float p_far_begin, float p_far_end) override {}
		virtual void set_parent_fade_alpha(float p_alpha) override {}
		virtual void set_transparency(float p_transparency) override {}
		virtual void set_use_baked_light(bool p_enable) override {}
		virtual void set_use_dynamic_gi(bool p_enable) override {}
		virtual void set_instance_shader_uniforms_offset(int32_t p_offset) override {}
		virtual void set_cast_double_sided_shadows(bool p_enable) override {}

		virtual Transform3D get_transform() override { return Transform3D(); }
		virtual AABB get_aabb() override { return AABB(); }

		virtual void set_softshadow_projector_pairing(bool p_softshadow, bool p_projector) override {}
	};

	PagedAllocator<GeometryInstanceDummy> geometry_instance_alloc;

public:
	/* ENVIRONMENT API */

	void set_time(double p_time, double p_step) override {}
	void set_debug_draw_mode(RS::ViewportDebugDraw p_debug_draw) override {}

	Ref<RenderSceneBuffers> render_buffers_create() override { return Ref<RenderSceneBuffers>(); }

	bool free(RID p_rid) override {
		return true;
	}
	void update() override {}

	RasterizerSceneDummy() {}
	~RasterizerSceneDummy() {}
};

#endif // RASTERIZER_SCENE_DUMMY_H
