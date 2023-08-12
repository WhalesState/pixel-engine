/**************************************************************************/
/*  renderer_scene_render.h                                               */
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

#ifndef RENDERER_SCENE_RENDER_H
#define RENDERER_SCENE_RENDER_H

#include "core/math/projection.h"
#include "core/templates/paged_array.h"
#include "servers/rendering/renderer_geometry_instance.h"
#include "servers/rendering/rendering_method.h"
#include "storage/render_scene_buffers.h"
#include "storage/utilities.h"

class RendererSceneRender {
public:
	enum {
		MAX_DIRECTIONAL_LIGHTS = 8,
		MAX_DIRECTIONAL_LIGHT_CASCADES = 4,
		MAX_RENDER_VIEWS = 2
	};

	/* ENVIRONMENT API */

	struct RenderShadowData {
		RID light;
		int pass = 0;
		PagedArray<RenderGeometryInstance *> instances;
	};

	struct RenderSDFGIData {
		int region = 0;
		PagedArray<RenderGeometryInstance *> instances;
	};

	struct RenderSDFGIUpdateData {
		bool update_static = false;
		uint32_t static_cascade_count;
		uint32_t *static_cascade_indices = nullptr;
		PagedArray<RID> *static_positional_lights;

		const Vector<RID> *directional_lights;
		uint32_t positional_light_count;
	};

	struct CameraData {
		// flags
		uint32_t view_count;
		bool is_orthogonal;
		uint32_t visible_layers;
		bool vaspect;

		// Main/center projection
		Transform3D main_transform;
		Projection main_projection;

		Transform3D view_offset[RendererSceneRender::MAX_RENDER_VIEWS];
		Projection view_projection[RendererSceneRender::MAX_RENDER_VIEWS];
		Vector2 taa_jitter;

		void set_camera(const Transform3D p_transform, const Projection p_projection, bool p_is_orthogonal, bool p_vaspect, const Vector2 &p_taa_jitter = Vector2(), uint32_t p_visible_layers = 0xFFFFFFFF);
		void set_multiview_camera(uint32_t p_view_count, const Transform3D *p_transforms, const Projection *p_projections, bool p_is_orthogonal, bool p_vaspect);
	};

	virtual void set_time(double p_time, double p_step) = 0;
	virtual void set_debug_draw_mode(RS::ViewportDebugDraw p_debug_draw) = 0;

	virtual Ref<RenderSceneBuffers> render_buffers_create() = 0;

	virtual bool free(RID p_rid) = 0;

	virtual void update() = 0;
	virtual ~RendererSceneRender() {}
};

#endif // RENDERER_SCENE_RENDER_H
