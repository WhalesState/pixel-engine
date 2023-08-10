/**************************************************************************/
/*  joypad_macos.h                                                        */
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

#ifndef JOYPAD_MACOS_H
#define JOYPAD_MACOS_H

#include "core/input/input.h"

#import <ForceFeedback/ForceFeedback.h>
#import <ForceFeedback/ForceFeedbackConstants.h>
#import <IOKit/hid/IOHIDLib.h>
#import <Kernel/IOKit/hidsystem/IOHIDUsageTables.h>

struct rec_element {
	IOHIDElementRef ref;
	IOHIDElementCookie cookie;

	uint32_t usage = 0;

	int min = 0;
	int max = 0;

	struct Comparator {
		bool operator()(const rec_element p_a, const rec_element p_b) const { return p_a.usage < p_b.usage; }
	};
};

struct joypad {
	IOHIDDeviceRef device_ref = nullptr;

	Vector<rec_element> axis_elements;
	Vector<rec_element> button_elements;
	Vector<rec_element> hat_elements;

	int id = 0;
	bool offset_hat = false;

	io_service_t ffservice = 0; // Interface for force feedback, 0 = no ff.
	FFCONSTANTFORCE ff_constant_force;
	FFDeviceObjectReference ff_device = nullptr;
	FFEffectObjectReference ff_object = nullptr;
	uint64_t ff_timestamp = 0;
	LONG *ff_directions = nullptr;
	FFEFFECT ff_effect;
	DWORD *ff_axes = nullptr;

	void add_hid_elements(CFArrayRef p_array);
	void add_hid_element(IOHIDElementRef p_element);

	bool has_element(IOHIDElementCookie p_cookie, Vector<rec_element> *p_list) const;
	bool config_force_feedback(io_service_t p_service);
	bool check_ff_features();

	int get_hid_element_state(rec_element *p_element) const;

	void free();
	joypad();
};

class JoypadMacOS {
	enum {
		JOYPADS_MAX = 16,
	};

private:
	Input *input = nullptr;
	IOHIDManagerRef hid_manager;

	Vector<joypad> device_list;

	bool have_device(IOHIDDeviceRef p_device) const;
	bool configure_joypad(IOHIDDeviceRef p_device_ref, joypad *p_joy);

	int get_joy_index(int p_id) const;
	int get_joy_ref(IOHIDDeviceRef p_device) const;

	void poll_joypads() const;
	void config_hid_manager(CFArrayRef p_matching_array) const;

	void joypad_vibration_start(int p_id, float p_magnitude, float p_duration, uint64_t p_timestamp);
	void joypad_vibration_stop(int p_id, uint64_t p_timestamp);

public:
	void process_joypads();

	void _device_added(IOReturn p_res, IOHIDDeviceRef p_device);
	void _device_removed(IOReturn p_res, IOHIDDeviceRef p_device);

	JoypadMacOS(Input *in);
	~JoypadMacOS();
};

#endif // JOYPAD_MACOS_H
