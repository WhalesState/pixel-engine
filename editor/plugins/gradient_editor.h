/**************************************************************************/
/*  gradient_editor.h                                                     */
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

#ifndef GRADIENT_EDITOR_H
#define GRADIENT_EDITOR_H

#include "scene/gui/color_picker.h"
#include "scene/gui/popup.h"
#include "scene/resources/gradient.h"

class GradientTexture1D;

class GradientEditor : public Control {
	GDCLASS(GradientEditor, Control);

	PopupPanel *popup = nullptr;
	ColorPicker *picker = nullptr;

	bool grabbing = false;
	int grabbed = -1;
	Vector<Gradient::Point> points;
	Gradient::InterpolationMode interpolation_mode = Gradient::GRADIENT_INTERPOLATE_LINEAR;
	Gradient::ColorSpace interpolation_color_space = Gradient::GRADIENT_COLOR_SPACE_SRGB;

	bool editing = false;
	Ref<Gradient> gradient;
	Ref<Gradient> gradient_cache;
	Ref<GradientTexture1D> preview_texture;

	// Make sure to use the scaled value below.
	const int BASE_SPACING = 3;
	const int BASE_HANDLE_WIDTH = 8;

	int draw_spacing = BASE_SPACING;
	int handle_width = BASE_HANDLE_WIDTH;

	void _gradient_changed();
	void _ramp_changed();
	void _color_changed(const Color &p_color);

	int _get_point_from_pos(int x);
	void _show_color_picker();

protected:
	virtual void gui_input(const Ref<InputEvent> &p_event) override;
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_gradient(const Ref<Gradient> &p_gradient);
	void reverse_gradient();

	void set_ramp(const Vector<float> &p_offsets, const Vector<Color> &p_colors);

	Vector<float> get_offsets() const;
	Vector<Color> get_colors() const;
	void set_points(Vector<Gradient::Point> &p_points);
	Vector<Gradient::Point> &get_points();

	void set_interpolation_mode(Gradient::InterpolationMode p_interp_mode);
	Gradient::InterpolationMode get_interpolation_mode();

	void set_interpolation_color_space(Gradient::ColorSpace p_color_space);
	Gradient::ColorSpace get_interpolation_color_space();

	ColorPicker *get_picker();
	PopupPanel *get_popup();

	virtual Size2 get_minimum_size() const override;

	GradientEditor();
	virtual ~GradientEditor();
};

#endif // GRADIENT_EDITOR_H
