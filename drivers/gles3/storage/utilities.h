/**************************************************************************/
/*  utilities.h                                                           */
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

#ifndef UTILITIES_GLES3_H
#define UTILITIES_GLES3_H

#ifdef GLES3_ENABLED

#include "servers/rendering/storage/utilities.h"

#include "platform_config.h"
#ifndef OPENGL_INCLUDE_H
#include <GLES3/gl3.h>
#else
#include OPENGL_INCLUDE_H
#endif

namespace GLES3 {

class Utilities : public RendererUtilities {
private:
	static Utilities *singleton;

	struct ResourceAllocation {
#ifdef DEV_ENABLED
		String name;
#endif
		uint32_t size = 0;
	};
	HashMap<GLuint, ResourceAllocation> buffer_allocs_cache;
	HashMap<GLuint, ResourceAllocation> texture_allocs_cache;

	uint64_t buffer_mem_cache = 0;
	uint64_t texture_mem_cache = 0;

public:
	static Utilities *get_singleton() { return singleton; }

	Utilities();
	~Utilities();

	// Buffer size is specified in bytes
	static Vector<uint8_t> buffer_get_data(GLenum p_target, GLuint p_buffer, uint32_t p_buffer_size);

	// Allocate memory with glBufferData. Does not handle resizing.
	_FORCE_INLINE_ void buffer_allocate_data(GLenum p_target, GLuint p_id, uint32_t p_size, const void *p_data, GLenum p_usage, String p_name = "") {
		glBufferData(p_target, p_size, p_data, p_usage);
		buffer_mem_cache += p_size;

#ifdef DEV_ENABLED
		ERR_FAIL_COND_MSG(buffer_allocs_cache.has(p_id), "trying to allocate buffer with name " + p_name + " but ID already used by " + buffer_allocs_cache[p_id].name);
#endif

		ResourceAllocation resource_allocation;
		resource_allocation.size = p_size;
#ifdef DEV_ENABLED
		resource_allocation.name = p_name + ": " + itos((uint64_t)p_id);
#endif
		buffer_allocs_cache[p_id] = resource_allocation;
	}

	_FORCE_INLINE_ void buffer_free_data(GLuint p_id) {
		ERR_FAIL_COND(!buffer_allocs_cache.has(p_id));
		glDeleteBuffers(1, &p_id);
		buffer_mem_cache -= buffer_allocs_cache[p_id].size;
		buffer_allocs_cache.erase(p_id);
	}

	// Records that data was allocated for state tracking purposes.
	_FORCE_INLINE_ void texture_allocated_data(GLuint p_id, uint32_t p_size, String p_name = "") {
		texture_mem_cache += p_size;
#ifdef DEV_ENABLED
		ERR_FAIL_COND_MSG(texture_allocs_cache.has(p_id), "trying to allocate texture with name " + p_name + " but ID already used by " + texture_allocs_cache[p_id].name);
#endif
		ResourceAllocation resource_allocation;
		resource_allocation.size = p_size;
#ifdef DEV_ENABLED
		resource_allocation.name = p_name + ": " + itos((uint64_t)p_id);
#endif
		texture_allocs_cache[p_id] = resource_allocation;
	}

	_FORCE_INLINE_ void texture_free_data(GLuint p_id) {
		ERR_FAIL_COND(!texture_allocs_cache.has(p_id));
		glDeleteTextures(1, &p_id);
		texture_mem_cache -= texture_allocs_cache[p_id].size;
		texture_allocs_cache.erase(p_id);
	}

	_FORCE_INLINE_ void texture_resize_data(GLuint p_id, uint32_t p_size) {
		ERR_FAIL_COND(!texture_allocs_cache.has(p_id));
		texture_mem_cache -= texture_allocs_cache[p_id].size;
		texture_mem_cache += p_size;
		texture_allocs_cache[p_id].size = p_size;
	}

	/* INSTANCES */

	virtual RS::InstanceType get_base_type(RID p_rid) const override;
	virtual bool free(RID p_rid) override;

	/* DEPENDENCIES */

	virtual void base_update_dependency(RID p_base, DependencyTracker *p_instance) override;

	/* TIMING */

#define MAX_QUERIES 256
#define FRAME_COUNT 3

	struct Frame {
		GLuint queries[MAX_QUERIES];
		TightLocalVector<String> timestamp_names;
		TightLocalVector<uint64_t> timestamp_cpu_values;
		uint32_t timestamp_count = 0;
		TightLocalVector<String> timestamp_result_names;
		TightLocalVector<uint64_t> timestamp_cpu_result_values;
		TightLocalVector<uint64_t> timestamp_result_values;
		uint32_t timestamp_result_count = 0;
		uint64_t index = 0;
	};

	const uint32_t max_timestamp_query_elements = MAX_QUERIES;

	Frame frames[FRAME_COUNT]; // Frames for capturing timestamps. We use 3 so we don't need to wait for commands to complete
	uint32_t frame = 0;

	virtual void capture_timestamps_begin() override;
	virtual void capture_timestamp(const String &p_name) override;
	virtual uint32_t get_captured_timestamps_count() const override;
	virtual uint64_t get_captured_timestamps_frame() const override;
	virtual uint64_t get_captured_timestamp_gpu_time(uint32_t p_index) const override;
	virtual uint64_t get_captured_timestamp_cpu_time(uint32_t p_index) const override;
	virtual String get_captured_timestamp_name(uint32_t p_index) const override;
	void _capture_timestamps_begin();
	void capture_timestamps_end();

	/* MISC */

	virtual void update_dirty_resources() override;
	virtual void set_debug_generate_wireframes(bool p_generate) override;

	virtual bool has_os_feature(const String &p_feature) const override;

	virtual void update_memory_info() override;

	virtual uint64_t get_rendering_info(RS::RenderingInfo p_info) override;
	virtual String get_video_adapter_name() const override;
	virtual String get_video_adapter_vendor() const override;
	virtual RenderingDevice::DeviceType get_video_adapter_type() const override;
	virtual String get_video_adapter_api_version() const override;

	virtual Size2i get_maximum_viewport_size() const override;
};

} // namespace GLES3

#endif // GLES3_ENABLED

#endif // UTILITIES_GLES3_H
