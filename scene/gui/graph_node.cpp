/**************************************************************************/
/*  graph_node.cpp                                                        */
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

#include "graph_node.h"

#include "core/string/translation.h"

#include "graph_edit.h"

bool GraphNode::_set(const StringName &p_name, const Variant &p_value) {
	String str = p_name;

	if (!str.begins_with("slot/")) {
		return false;
	}

	int idx = str.get_slice("/", 1).to_int();
	String what = str.get_slice("/", 2);

	Slot si;
	if (slot_info.has(idx)) {
		si = slot_info[idx];
	}

	if (what == "left_enabled") {
		si.enable_left = p_value;
	} else if (what == "left_type") {
		si.type_left = p_value;
	} else if (what == "left_icon") {
		si.custom_slot_left = p_value;
	} else if (what == "left_color") {
		si.color_left = p_value;
	} else if (what == "right_enabled") {
		si.enable_right = p_value;
	} else if (what == "right_type") {
		si.type_right = p_value;
	} else if (what == "right_color") {
		si.color_right = p_value;
	} else if (what == "right_icon") {
		si.custom_slot_right = p_value;
	} else if (what == "draw_stylebox") {
		si.draw_stylebox = p_value;
	} else {
		return false;
	}

	set_slot(idx, si.enable_left, si.type_left, si.color_left, si.enable_right, si.type_right, si.color_right, si.custom_slot_left, si.custom_slot_right, si.draw_stylebox);
	queue_redraw();
	return true;
}

bool GraphNode::_get(const StringName &p_name, Variant &r_ret) const {
	String str = p_name;
	if (!str.begins_with("slot/")) {
		return false;
	}

	int idx = str.get_slice("/", 1).to_int();
	String what = str.get_slice("/", 2);

	Slot si;
	if (slot_info.has(idx)) {
		si = slot_info[idx];
	}

	if (what == "left_enabled") {
		r_ret = si.enable_left;
	} else if (what == "left_type") {
		r_ret = si.type_left;
	} else if (what == "left_color") {
		r_ret = si.color_left;
	} else if (what == "left_icon") {
		r_ret = si.custom_slot_left;
	} else if (what == "right_enabled") {
		r_ret = si.enable_right;
	} else if (what == "right_type") {
		r_ret = si.type_right;
	} else if (what == "right_color") {
		r_ret = si.color_right;
	} else if (what == "right_icon") {
		r_ret = si.custom_slot_right;
	} else if (what == "draw_stylebox") {
		r_ret = si.draw_stylebox;
	} else {
		return false;
	}

	return true;
}

void GraphNode::_get_property_list(List<PropertyInfo> *p_list) const {
	int idx = 0;
	for (int i = 0; i < get_child_count(); i++) {
		Control *c = Object::cast_to<Control>(get_child(i));
		if (!c || c->is_set_as_top_level()) {
			continue;
		}

		String base = "slot/" + itos(idx) + "/";

		p_list->push_back(PropertyInfo(Variant::BOOL, base + "left_enabled"));
		p_list->push_back(PropertyInfo(Variant::INT, base + "left_type"));
		p_list->push_back(PropertyInfo(Variant::COLOR, base + "left_color"));
		p_list->push_back(PropertyInfo(Variant::OBJECT, base + "left_icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_STORE_IF_NULL));
		p_list->push_back(PropertyInfo(Variant::BOOL, base + "right_enabled"));
		p_list->push_back(PropertyInfo(Variant::INT, base + "right_type"));
		p_list->push_back(PropertyInfo(Variant::COLOR, base + "right_color"));
		p_list->push_back(PropertyInfo(Variant::OBJECT, base + "right_icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_STORE_IF_NULL));
		p_list->push_back(PropertyInfo(Variant::BOOL, base + "draw_stylebox"));
		idx++;
	}
}

void GraphNode::_resort() {
	/** First pass, determine minimum size AND amount of stretchable elements */

	Size2i new_size = get_size();
	Ref<StyleBox> sb = get_theme_stylebox(SNAME("frame"));
	Ref<StyleBox> sb_slot = get_theme_stylebox(SNAME("slot"));

	int sep = get_theme_constant(SNAME("separation"));

	bool first = true;
	int children_count = 0;
	int stretch_min = 0;
	int stretch_avail = 0;
	float stretch_ratio_total = 0;
	HashMap<Control *, _MinSizeCache> min_size_cache;

	for (int i = 0; i < get_child_count(); i++) {
		Control *c = Object::cast_to<Control>(get_child(i));
		if (!c || !c->is_visible_in_tree()) {
			continue;
		}
		if (c->is_set_as_top_level()) {
			continue;
		}

		Size2i size = c->get_combined_minimum_size() + (slot_info[i].draw_stylebox ? sb_slot->get_minimum_size() : Size2());
		_MinSizeCache msc;

		stretch_min += size.height;
		msc.min_size = size.height;
		msc.will_stretch = c->get_v_size_flags().has_flag(SIZE_EXPAND);

		if (msc.will_stretch) {
			stretch_avail += msc.min_size;
			stretch_ratio_total += c->get_stretch_ratio();
		}
		msc.final_size = msc.min_size;
		min_size_cache[c] = msc;
		children_count++;
	}

	if (children_count == 0) {
		return;
	}

	int stretch_max = new_size.height - (children_count - 1) * sep;
	int stretch_diff = stretch_max - stretch_min;
	if (stretch_diff < 0) {
		//avoid negative stretch space
		stretch_diff = 0;
	}

	stretch_avail += stretch_diff - sb->get_margin(SIDE_BOTTOM) - sb->get_margin(SIDE_TOP); //available stretch space.
	/** Second, pass successively to discard elements that can't be stretched, this will run while stretchable
		elements exist */

	while (stretch_ratio_total > 0) { // first of all, don't even be here if no stretchable objects exist
		bool refit_successful = true; //assume refit-test will go well

		for (int i = 0; i < get_child_count(); i++) {
			Control *c = Object::cast_to<Control>(get_child(i));
			if (!c || !c->is_visible_in_tree()) {
				continue;
			}
			if (c->is_set_as_top_level()) {
				continue;
			}

			ERR_FAIL_COND(!min_size_cache.has(c));
			_MinSizeCache &msc = min_size_cache[c];

			if (msc.will_stretch) { //wants to stretch
				//let's see if it can really stretch

				int final_pixel_size = stretch_avail * c->get_stretch_ratio() / stretch_ratio_total;
				if (final_pixel_size < msc.min_size) {
					//if available stretching area is too small for widget,
					//then remove it from stretching area
					msc.will_stretch = false;
					stretch_ratio_total -= c->get_stretch_ratio();
					refit_successful = false;
					stretch_avail -= msc.min_size;
					msc.final_size = msc.min_size;
					break;
				} else {
					msc.final_size = final_pixel_size;
				}
			}
		}

		if (refit_successful) { //uf refit went well, break
			break;
		}
	}

	/** Final pass, draw and stretch elements **/

	int ofs = sb->get_margin(SIDE_TOP);

	first = true;
	int idx = 0;
	cache_y.clear();
	int w = new_size.width - sb->get_minimum_size().x;

	for (int i = 0; i < get_child_count(); i++) {
		Control *c = Object::cast_to<Control>(get_child(i));
		if (!c || !c->is_visible_in_tree()) {
			continue;
		}
		if (c->is_set_as_top_level()) {
			continue;
		}

		_MinSizeCache &msc = min_size_cache[c];

		if (first) {
			first = false;
		} else {
			ofs += sep;
		}

		int from = ofs;
		int to = ofs + msc.final_size;

		if (msc.will_stretch && idx == children_count - 1) {
			//adjust so the last one always fits perfect
			//compensating for numerical imprecision

			to = new_size.height - sb->get_margin(SIDE_BOTTOM);
		}

		int size = to - from;

		float margin = sb->get_margin(SIDE_LEFT) + (slot_info[i].draw_stylebox ? sb_slot->get_margin(SIDE_LEFT) : 0);
		float width = w - (slot_info[i].draw_stylebox ? sb_slot->get_minimum_size().x : 0);
		Rect2 rect(margin, from, width, size);

		fit_child_in_rect(c, rect);
		cache_y.push_back(from - sb->get_margin(SIDE_TOP) + size * 0.5);

		ofs = to;
		idx++;
	}

	queue_redraw();
	connpos_dirty = true;
}

void GraphNode::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			Ref<StyleBox> sb;

			sb = get_theme_stylebox(selected ? SNAME("selected_frame") : SNAME("frame"));

			Ref<StyleBox> sb_slot = get_theme_stylebox(SNAME("slot"));

			Ref<Texture2D> port = get_theme_icon(SNAME("port"));
			Ref<Texture2D> close = get_theme_icon(SNAME("close"));
			Ref<Texture2D> resizer = get_theme_icon(SNAME("resizer"));
			int close_offset = get_theme_constant(SNAME("close_offset"));
			int close_h_offset = get_theme_constant(SNAME("close_h_offset"));
			Color close_color = get_theme_color(SNAME("close_color"));
			Color resizer_color = get_theme_color(SNAME("resizer_color"));
			int title_offset = get_theme_constant(SNAME("title_offset"));
			int title_h_offset = get_theme_constant(SNAME("title_h_offset"));
			Color title_color = get_theme_color(SNAME("title_color"));
			Point2i icofs = -port->get_size() * 0.5;
			int edgeofs = get_theme_constant(SNAME("port_offset"));
			icofs.y += sb->get_margin(SIDE_TOP);

			draw_style_box(sb, Rect2(Point2(), get_size()));

			switch (overlay) {
				case OVERLAY_DISABLED: {
				} break;
				case OVERLAY_BREAKPOINT: {
					draw_style_box(get_theme_stylebox(SNAME("breakpoint")), Rect2(Point2(), get_size()));
				} break;
				case OVERLAY_POSITION: {
					draw_style_box(get_theme_stylebox(SNAME("position")), Rect2(Point2(), get_size()));

				} break;
			}

			int w = get_size().width - sb->get_minimum_size().x;

			title_buf->draw(get_canvas_item(), Point2(sb->get_margin(SIDE_LEFT) + title_h_offset, -title_buf->get_size().y + title_offset), title_color);
			if (show_close) {
				Vector2 cpos = Point2(w + sb->get_margin(SIDE_LEFT) + close_h_offset - close->get_width(), -close->get_height() + close_offset);
				draw_texture(close, cpos, close_color);
				close_rect.position = cpos;
				close_rect.size = close->get_size();
			} else {
				close_rect = Rect2();
			}

			if (get_child_count() > 0) {
				for (const KeyValue<int, Slot> &E : slot_info) {
					if (E.key < 0 || E.key >= cache_y.size()) {
						continue;
					}
					if (!slot_info.has(E.key)) {
						continue;
					}
					const Slot &s = slot_info[E.key];
					// Left port.
					if (s.enable_left) {
						Ref<Texture2D> p = port;
						if (s.custom_slot_left.is_valid()) {
							p = s.custom_slot_left;
						}
						p->draw(get_canvas_item(), icofs + Point2(edgeofs, cache_y[E.key]), s.color_left);
					}
					// Right port.
					if (s.enable_right) {
						Ref<Texture2D> p = port;
						if (s.custom_slot_right.is_valid()) {
							p = s.custom_slot_right;
						}
						p->draw(get_canvas_item(), icofs + Point2(get_size().x - edgeofs, cache_y[E.key]), s.color_right);
					}

					// Draw slot stylebox.
					if (s.draw_stylebox) {
						Control *c = Object::cast_to<Control>(get_child(E.key));
						if (!c || !c->is_visible_in_tree()) {
							continue;
						}
						if (c->is_set_as_top_level()) {
							continue;
						}
						Rect2 c_rect = c->get_rect();
						c_rect.position.x = sb->get_margin(SIDE_LEFT);
						c_rect.size.width = w;
						draw_style_box(sb_slot, c_rect);
					}
				}
			}

			if (resizable) {
				draw_texture(resizer, get_size() - resizer->get_size(), resizer_color);
			}
		} break;

		case NOTIFICATION_SORT_CHILDREN: {
			_resort();
		} break;

		case NOTIFICATION_LAYOUT_DIRECTION_CHANGED:
		case NOTIFICATION_TRANSLATION_CHANGED:
		case NOTIFICATION_THEME_CHANGED: {
			_shape();

			update_minimum_size();
			queue_redraw();
		} break;
	}
}

void GraphNode::_shape() {
	Ref<Font> font = get_theme_font(SNAME("title_font"));
	int font_size = get_theme_font_size(SNAME("title_font_size"));

	title_buf->clear();
	if (text_direction == Control::TEXT_DIRECTION_INHERITED) {
		title_buf->set_direction(is_layout_rtl() ? TextServer::DIRECTION_RTL : TextServer::DIRECTION_LTR);
	} else {
		title_buf->set_direction((TextServer::Direction)text_direction);
	}
	title_buf->add_string(title, font, font_size, language);
}

#ifdef TOOLS_ENABLED
void GraphNode::_edit_set_position(const Point2 &p_position) {
	GraphEdit *graph = Object::cast_to<GraphEdit>(get_parent());
	if (graph) {
		Point2 offset = (p_position + graph->get_scroll_offset()) * graph->get_zoom();
		set_position_offset(offset);
	}
	set_position(p_position);
}
#endif

void GraphNode::_validate_property(PropertyInfo &p_property) const {
	GraphEdit *graph = Object::cast_to<GraphEdit>(get_parent());
	if (graph) {
		if (p_property.name == "position") {
			p_property.usage |= PROPERTY_USAGE_READ_ONLY;
		}
	}
}

void GraphNode::set_slot(int p_idx, bool p_enable_left, int p_type_left, const Color &p_color_left, bool p_enable_right, int p_type_right, const Color &p_color_right, const Ref<Texture2D> &p_custom_left, const Ref<Texture2D> &p_custom_right, bool p_draw_stylebox) {
	ERR_FAIL_COND_MSG(p_idx < 0, vformat("Cannot set slot with p_idx (%d) lesser than zero.", p_idx));

	if (!p_enable_left && p_type_left == 0 && p_color_left == Color(1, 1, 1, 1) &&
			!p_enable_right && p_type_right == 0 && p_color_right == Color(1, 1, 1, 1) &&
			!p_custom_left.is_valid() && !p_custom_right.is_valid()) {
		slot_info.erase(p_idx);
		return;
	}

	Slot s;
	s.enable_left = p_enable_left;
	s.type_left = p_type_left;
	s.color_left = p_color_left;
	s.enable_right = p_enable_right;
	s.type_right = p_type_right;
	s.color_right = p_color_right;
	s.custom_slot_left = p_custom_left;
	s.custom_slot_right = p_custom_right;
	s.draw_stylebox = p_draw_stylebox;
	slot_info[p_idx] = s;
	queue_redraw();
	connpos_dirty = true;

	emit_signal(SNAME("slot_updated"), p_idx);
}

void GraphNode::clear_slot(int p_idx) {
	slot_info.erase(p_idx);
	queue_redraw();
	connpos_dirty = true;
}

void GraphNode::clear_all_slots() {
	slot_info.clear();
	queue_redraw();
	connpos_dirty = true;
}

bool GraphNode::is_slot_enabled_left(int p_idx) const {
	if (!slot_info.has(p_idx)) {
		return false;
	}
	return slot_info[p_idx].enable_left;
}

void GraphNode::set_slot_enabled_left(int p_idx, bool p_enable_left) {
	ERR_FAIL_COND_MSG(p_idx < 0, vformat("Cannot set enable_left for the slot with p_idx (%d) lesser than zero.", p_idx));

	if (slot_info[p_idx].enable_left == p_enable_left) {
		return;
	}

	slot_info[p_idx].enable_left = p_enable_left;
	queue_redraw();
	connpos_dirty = true;

	emit_signal(SNAME("slot_updated"), p_idx);
}

void GraphNode::set_slot_type_left(int p_idx, int p_type_left) {
	ERR_FAIL_COND_MSG(!slot_info.has(p_idx), vformat("Cannot set type_left for the slot '%d' because it hasn't been enabled.", p_idx));

	if (slot_info[p_idx].type_left == p_type_left) {
		return;
	}

	slot_info[p_idx].type_left = p_type_left;
	queue_redraw();
	connpos_dirty = true;

	emit_signal(SNAME("slot_updated"), p_idx);
}

int GraphNode::get_slot_type_left(int p_idx) const {
	if (!slot_info.has(p_idx)) {
		return 0;
	}
	return slot_info[p_idx].type_left;
}

void GraphNode::set_slot_color_left(int p_idx, const Color &p_color_left) {
	ERR_FAIL_COND_MSG(!slot_info.has(p_idx), vformat("Cannot set color_left for the slot '%d' because it hasn't been enabled.", p_idx));

	if (slot_info[p_idx].color_left == p_color_left) {
		return;
	}

	slot_info[p_idx].color_left = p_color_left;
	queue_redraw();
	connpos_dirty = true;

	emit_signal(SNAME("slot_updated"), p_idx);
}

Color GraphNode::get_slot_color_left(int p_idx) const {
	if (!slot_info.has(p_idx)) {
		return Color(1, 1, 1, 1);
	}
	return slot_info[p_idx].color_left;
}

bool GraphNode::is_slot_enabled_right(int p_idx) const {
	if (!slot_info.has(p_idx)) {
		return false;
	}
	return slot_info[p_idx].enable_right;
}

void GraphNode::set_slot_enabled_right(int p_idx, bool p_enable_right) {
	ERR_FAIL_COND_MSG(p_idx < 0, vformat("Cannot set enable_right for the slot with p_idx (%d) lesser than zero.", p_idx));

	if (slot_info[p_idx].enable_right == p_enable_right) {
		return;
	}

	slot_info[p_idx].enable_right = p_enable_right;
	queue_redraw();
	connpos_dirty = true;

	emit_signal(SNAME("slot_updated"), p_idx);
}

void GraphNode::set_slot_type_right(int p_idx, int p_type_right) {
	ERR_FAIL_COND_MSG(!slot_info.has(p_idx), vformat("Cannot set type_right for the slot '%d' because it hasn't been enabled.", p_idx));

	if (slot_info[p_idx].type_right == p_type_right) {
		return;
	}

	slot_info[p_idx].type_right = p_type_right;
	queue_redraw();
	connpos_dirty = true;

	emit_signal(SNAME("slot_updated"), p_idx);
}

int GraphNode::get_slot_type_right(int p_idx) const {
	if (!slot_info.has(p_idx)) {
		return 0;
	}
	return slot_info[p_idx].type_right;
}

void GraphNode::set_slot_color_right(int p_idx, const Color &p_color_right) {
	ERR_FAIL_COND_MSG(!slot_info.has(p_idx), vformat("Cannot set color_right for the slot '%d' because it hasn't been enabled.", p_idx));

	if (slot_info[p_idx].color_right == p_color_right) {
		return;
	}

	slot_info[p_idx].color_right = p_color_right;
	queue_redraw();
	connpos_dirty = true;

	emit_signal(SNAME("slot_updated"), p_idx);
}

Color GraphNode::get_slot_color_right(int p_idx) const {
	if (!slot_info.has(p_idx)) {
		return Color(1, 1, 1, 1);
	}
	return slot_info[p_idx].color_right;
}

bool GraphNode::is_slot_draw_stylebox(int p_idx) const {
	if (!slot_info.has(p_idx)) {
		return false;
	}
	return slot_info[p_idx].draw_stylebox;
}

void GraphNode::set_slot_draw_stylebox(int p_idx, bool p_enable) {
	ERR_FAIL_COND_MSG(p_idx < 0, vformat("Cannot set draw_stylebox for the slot with p_idx (%d) lesser than zero.", p_idx));

	slot_info[p_idx].draw_stylebox = p_enable;
	queue_redraw();
	connpos_dirty = true;

	emit_signal(SNAME("slot_updated"), p_idx);
}

Size2 GraphNode::get_minimum_size() const {
	Ref<StyleBox> sb = get_theme_stylebox(SNAME("frame"));
	Ref<StyleBox> sb_slot = get_theme_stylebox(SNAME("slot"));

	int sep = get_theme_constant(SNAME("separation"));
	int title_h_offset = get_theme_constant(SNAME("title_h_offset"));

	bool first = true;

	Size2 minsize;
	minsize.x = title_buf->get_size().x + title_h_offset;
	if (show_close) {
		int close_h_offset = get_theme_constant(SNAME("close_h_offset"));
		Ref<Texture2D> close = get_theme_icon(SNAME("close"));
		//TODO: Remove this magic number after GraphNode rework.
		minsize.x += 12 + close->get_width() + close_h_offset;
	}

	for (int i = 0; i < get_child_count(); i++) {
		Control *c = Object::cast_to<Control>(get_child(i));
		if (!c || !c->is_visible()) {
			continue;
		}
		if (c->is_set_as_top_level()) {
			continue;
		}

		Size2i size = c->get_combined_minimum_size();
		if (slot_info.has(i)) {
			size += slot_info[i].draw_stylebox ? sb_slot->get_minimum_size() : Size2();
		}

		minsize.y += size.y;
		minsize.x = MAX(minsize.x, size.x);

		if (first) {
			first = false;
		} else {
			minsize.y += sep;
		}
	}

	return minsize + sb->get_minimum_size();
}

void GraphNode::set_title(const String &p_title) {
	if (title == p_title) {
		return;
	}
	title = p_title;
	_shape();

	queue_redraw();
	update_minimum_size();
}

String GraphNode::get_title() const {
	return title;
}

void GraphNode::set_text_direction(Control::TextDirection p_text_direction) {
	ERR_FAIL_COND((int)p_text_direction < -1 || (int)p_text_direction > 3);
	if (text_direction != p_text_direction) {
		text_direction = p_text_direction;
		_shape();
		queue_redraw();
	}
}

Control::TextDirection GraphNode::get_text_direction() const {
	return text_direction;
}

void GraphNode::set_language(const String &p_language) {
	if (language != p_language) {
		language = p_language;
		_shape();
		queue_redraw();
	}
}

String GraphNode::get_language() const {
	return language;
}

void GraphNode::set_position_offset(const Vector2 &p_offset) {
	if (position_offset == p_offset) {
		return;
	}

	position_offset = p_offset;
	emit_signal(SNAME("position_offset_changed"));
	queue_redraw();
}

Vector2 GraphNode::get_position_offset() const {
	return position_offset;
}

void GraphNode::set_selected(bool p_selected) {
	if (!is_selectable() || selected == p_selected) {
		return;
	}

	selected = p_selected;
	emit_signal(p_selected ? SNAME("node_selected") : SNAME("node_deselected"));
	queue_redraw();
}

bool GraphNode::is_selected() {
	return selected;
}

void GraphNode::set_drag(bool p_drag) {
	if (p_drag) {
		drag_from = get_position_offset();
	} else {
		emit_signal(SNAME("dragged"), drag_from, get_position_offset()); //useful for undo/redo
	}
}

Vector2 GraphNode::get_drag_from() {
	return drag_from;
}

void GraphNode::set_show_close_button(bool p_enable) {
	if (show_close == p_enable) {
		return;
	}

	show_close = p_enable;
	queue_redraw();
}

bool GraphNode::is_close_button_visible() const {
	return show_close;
}

void GraphNode::_connpos_update() {
	int edgeofs = get_theme_constant(SNAME("port_offset"));
	int sep = get_theme_constant(SNAME("separation"));

	Ref<StyleBox> sb = get_theme_stylebox(SNAME("frame"));
	left_port_cache.clear();
	right_port_cache.clear();
	int vofs = 0;

	int idx = 0;

	for (int i = 0; i < get_child_count(); i++) {
		Control *c = Object::cast_to<Control>(get_child(i));
		if (!c) {
			continue;
		}
		if (c->is_set_as_top_level()) {
			continue;
		}

		Size2i size = c->get_rect().size;

		int y = sb->get_margin(SIDE_TOP) + vofs;
		int h = size.height;

		if (slot_info.has(idx)) {
			if (slot_info[idx].enable_left) {
				PortCache cc;
				cc.position = Point2i(edgeofs, y + h / 2);
				cc.height = h;

				cc.slot_idx = idx;
				cc.type = slot_info[idx].type_left;
				cc.color = slot_info[idx].color_left;

				left_port_cache.push_back(cc);
			}
			if (slot_info[idx].enable_right) {
				PortCache cc;
				cc.position = Point2i(get_size().width - edgeofs, y + h / 2);
				cc.height = h;

				cc.slot_idx = idx;
				cc.type = slot_info[idx].type_right;
				cc.color = slot_info[idx].color_right;

				right_port_cache.push_back(cc);
			}
		}

		vofs += sep;
		vofs += h;
		idx++;
	}

	connpos_dirty = false;
}

int GraphNode::get_connection_input_count() {
	if (connpos_dirty) {
		_connpos_update();
	}

	return left_port_cache.size();
}

int GraphNode::get_connection_input_height(int p_port) {
	if (connpos_dirty) {
		_connpos_update();
	}

	ERR_FAIL_INDEX_V(p_port, left_port_cache.size(), 0);
	return left_port_cache[p_port].height;
}

Vector2 GraphNode::get_connection_input_position(int p_port) {
	if (connpos_dirty) {
		_connpos_update();
	}

	ERR_FAIL_INDEX_V(p_port, left_port_cache.size(), Vector2());
	Vector2 pos = left_port_cache[p_port].position;
	pos.x *= get_scale().x;
	pos.y *= get_scale().y;
	return pos;
}

int GraphNode::get_connection_input_type(int p_port) {
	if (connpos_dirty) {
		_connpos_update();
	}

	ERR_FAIL_INDEX_V(p_port, left_port_cache.size(), 0);
	return left_port_cache[p_port].type;
}

Color GraphNode::get_connection_input_color(int p_port) {
	if (connpos_dirty) {
		_connpos_update();
	}

	ERR_FAIL_INDEX_V(p_port, left_port_cache.size(), Color());
	return left_port_cache[p_port].color;
}

int GraphNode::get_connection_input_slot(int p_port) {
	if (connpos_dirty) {
		_connpos_update();
	}

	ERR_FAIL_INDEX_V(p_port, left_port_cache.size(), -1);
	return left_port_cache[p_port].slot_idx;
}

int GraphNode::get_connection_output_count() {
	if (connpos_dirty) {
		_connpos_update();
	}

	return right_port_cache.size();
}

int GraphNode::get_connection_output_height(int p_port) {
	if (connpos_dirty) {
		_connpos_update();
	}

	ERR_FAIL_INDEX_V(p_port, right_port_cache.size(), 0);
	return right_port_cache[p_port].height;
}

Vector2 GraphNode::get_connection_output_position(int p_port) {
	if (connpos_dirty) {
		_connpos_update();
	}

	ERR_FAIL_INDEX_V(p_port, right_port_cache.size(), Vector2());
	Vector2 pos = right_port_cache[p_port].position;
	pos.x *= get_scale().x;
	pos.y *= get_scale().y;
	return pos;
}

int GraphNode::get_connection_output_type(int p_port) {
	if (connpos_dirty) {
		_connpos_update();
	}

	ERR_FAIL_INDEX_V(p_port, right_port_cache.size(), 0);
	return right_port_cache[p_port].type;
}

Color GraphNode::get_connection_output_color(int p_port) {
	if (connpos_dirty) {
		_connpos_update();
	}

	ERR_FAIL_INDEX_V(p_port, right_port_cache.size(), Color());
	return right_port_cache[p_port].color;
}

int GraphNode::get_connection_output_slot(int p_port) {
	if (connpos_dirty) {
		_connpos_update();
	}

	ERR_FAIL_INDEX_V(p_port, right_port_cache.size(), -1);
	return right_port_cache[p_port].slot_idx;
}

void GraphNode::gui_input(const Ref<InputEvent> &p_ev) {
	ERR_FAIL_COND(p_ev.is_null());

	Ref<InputEventMouseButton> mb = p_ev;
	if (mb.is_valid()) {
		ERR_FAIL_COND_MSG(get_parent_control() == nullptr, "GraphNode must be the child of a GraphEdit node.");

		if (mb->is_pressed() && mb->get_button_index() == MouseButton::LEFT) {
			Vector2 mpos = mb->get_position();
			if (close_rect.size != Size2() && close_rect.has_point(mpos)) {
				//send focus to parent
				get_parent_control()->grab_focus();
				emit_signal(SNAME("close_request"));
				accept_event();
				return;
			}

			Ref<Texture2D> resizer = get_theme_icon(SNAME("resizer"));

			if (resizable && mpos.x > get_size().x - resizer->get_width() && mpos.y > get_size().y - resizer->get_height()) {
				resizing = true;
				resizing_from = mpos;
				resizing_from_size = get_size();
				accept_event();
				return;
			}

			emit_signal(SNAME("raise_request"));
		}

		if (!mb->is_pressed() && mb->get_button_index() == MouseButton::LEFT) {
			resizing = false;
		}
	}

	Ref<InputEventMouseMotion> mm = p_ev;
	if (resizing && mm.is_valid()) {
		Vector2 mpos = mm->get_position();
		Vector2 diff = mpos - resizing_from;

		emit_signal(SNAME("resize_request"), resizing_from_size + diff);
	}
}

void GraphNode::set_overlay(Overlay p_overlay) {
	if (overlay == p_overlay) {
		return;
	}

	overlay = p_overlay;
	queue_redraw();
}

GraphNode::Overlay GraphNode::get_overlay() const {
	return overlay;
}

void GraphNode::set_resizable(bool p_enable) {
	if (resizable == p_enable) {
		return;
	}

	resizable = p_enable;
	queue_redraw();
}

bool GraphNode::is_resizable() const {
	return resizable;
}

void GraphNode::set_draggable(bool p_draggable) {
	draggable = p_draggable;
}

bool GraphNode::is_draggable() {
	return draggable;
}

void GraphNode::set_selectable(bool p_selectable) {
	if (!p_selectable) {
		set_selected(false);
	}
	selectable = p_selectable;
}

bool GraphNode::is_selectable() {
	return selectable;
}

Control::CursorShape GraphNode::get_cursor_shape(const Point2 &p_pos) const {
	if (resizable) {
		Ref<Texture2D> resizer = get_theme_icon(SNAME("resizer"));

		if (resizing || (p_pos.x > get_size().x - resizer->get_width() && p_pos.y > get_size().y - resizer->get_height())) {
			return CURSOR_FDIAGSIZE;
		}
	}

	return Control::get_cursor_shape(p_pos);
}

Vector<int> GraphNode::get_allowed_size_flags_horizontal() const {
	Vector<int> flags;
	flags.append(SIZE_FILL);
	flags.append(SIZE_SHRINK_BEGIN);
	flags.append(SIZE_SHRINK_CENTER);
	flags.append(SIZE_SHRINK_END);
	return flags;
}

Vector<int> GraphNode::get_allowed_size_flags_vertical() const {
	Vector<int> flags;
	flags.append(SIZE_FILL);
	flags.append(SIZE_EXPAND);
	flags.append(SIZE_SHRINK_BEGIN);
	flags.append(SIZE_SHRINK_CENTER);
	flags.append(SIZE_SHRINK_END);
	return flags;
}

void GraphNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_title", "title"), &GraphNode::set_title);
	ClassDB::bind_method(D_METHOD("get_title"), &GraphNode::get_title);
	ClassDB::bind_method(D_METHOD("set_text_direction", "direction"), &GraphNode::set_text_direction);
	ClassDB::bind_method(D_METHOD("get_text_direction"), &GraphNode::get_text_direction);
	ClassDB::bind_method(D_METHOD("set_language", "language"), &GraphNode::set_language);
	ClassDB::bind_method(D_METHOD("get_language"), &GraphNode::get_language);

	ClassDB::bind_method(D_METHOD("set_slot", "slot_index", "enable_left_port", "type_left", "color_left", "enable_right_port", "type_right", "color_right", "custom_icon_left", "custom_icon_right", "draw_stylebox"), &GraphNode::set_slot, DEFVAL(Ref<Texture2D>()), DEFVAL(Ref<Texture2D>()), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("clear_slot", "slot_index"), &GraphNode::clear_slot);
	ClassDB::bind_method(D_METHOD("clear_all_slots"), &GraphNode::clear_all_slots);

	ClassDB::bind_method(D_METHOD("set_slot_enabled_left", "slot_index", "enable"), &GraphNode::set_slot_enabled_left);
	ClassDB::bind_method(D_METHOD("is_slot_enabled_left", "slot_index"), &GraphNode::is_slot_enabled_left);

	ClassDB::bind_method(D_METHOD("set_slot_type_left", "slot_index", "type"), &GraphNode::set_slot_type_left);
	ClassDB::bind_method(D_METHOD("get_slot_type_left", "slot_index"), &GraphNode::get_slot_type_left);

	ClassDB::bind_method(D_METHOD("set_slot_color_left", "slot_index", "color"), &GraphNode::set_slot_color_left);
	ClassDB::bind_method(D_METHOD("get_slot_color_left", "slot_index"), &GraphNode::get_slot_color_left);

	ClassDB::bind_method(D_METHOD("set_slot_enabled_right", "slot_index", "enable"), &GraphNode::set_slot_enabled_right);
	ClassDB::bind_method(D_METHOD("is_slot_enabled_right", "slot_index"), &GraphNode::is_slot_enabled_right);

	ClassDB::bind_method(D_METHOD("set_slot_type_right", "slot_index", "type"), &GraphNode::set_slot_type_right);
	ClassDB::bind_method(D_METHOD("get_slot_type_right", "slot_index"), &GraphNode::get_slot_type_right);

	ClassDB::bind_method(D_METHOD("set_slot_color_right", "slot_index", "color"), &GraphNode::set_slot_color_right);
	ClassDB::bind_method(D_METHOD("get_slot_color_right", "slot_index"), &GraphNode::get_slot_color_right);

	ClassDB::bind_method(D_METHOD("is_slot_draw_stylebox", "slot_index"), &GraphNode::is_slot_draw_stylebox);
	ClassDB::bind_method(D_METHOD("set_slot_draw_stylebox", "slot_index", "enable"), &GraphNode::set_slot_draw_stylebox);

	ClassDB::bind_method(D_METHOD("set_position_offset", "offset"), &GraphNode::set_position_offset);
	ClassDB::bind_method(D_METHOD("get_position_offset"), &GraphNode::get_position_offset);

	ClassDB::bind_method(D_METHOD("set_resizable", "resizable"), &GraphNode::set_resizable);
	ClassDB::bind_method(D_METHOD("is_resizable"), &GraphNode::is_resizable);

	ClassDB::bind_method(D_METHOD("set_draggable", "draggable"), &GraphNode::set_draggable);
	ClassDB::bind_method(D_METHOD("is_draggable"), &GraphNode::is_draggable);

	ClassDB::bind_method(D_METHOD("set_selectable", "selectable"), &GraphNode::set_selectable);
	ClassDB::bind_method(D_METHOD("is_selectable"), &GraphNode::is_selectable);

	ClassDB::bind_method(D_METHOD("set_selected", "selected"), &GraphNode::set_selected);
	ClassDB::bind_method(D_METHOD("is_selected"), &GraphNode::is_selected);

	ClassDB::bind_method(D_METHOD("get_connection_input_count"), &GraphNode::get_connection_input_count);
	ClassDB::bind_method(D_METHOD("get_connection_input_height", "port"), &GraphNode::get_connection_input_height);
	ClassDB::bind_method(D_METHOD("get_connection_input_position", "port"), &GraphNode::get_connection_input_position);
	ClassDB::bind_method(D_METHOD("get_connection_input_type", "port"), &GraphNode::get_connection_input_type);
	ClassDB::bind_method(D_METHOD("get_connection_input_color", "port"), &GraphNode::get_connection_input_color);
	ClassDB::bind_method(D_METHOD("get_connection_input_slot", "port"), &GraphNode::get_connection_input_slot);

	ClassDB::bind_method(D_METHOD("get_connection_output_count"), &GraphNode::get_connection_output_count);
	ClassDB::bind_method(D_METHOD("get_connection_output_height", "port"), &GraphNode::get_connection_output_height);
	ClassDB::bind_method(D_METHOD("get_connection_output_position", "port"), &GraphNode::get_connection_output_position);
	ClassDB::bind_method(D_METHOD("get_connection_output_type", "port"), &GraphNode::get_connection_output_type);
	ClassDB::bind_method(D_METHOD("get_connection_output_color", "port"), &GraphNode::get_connection_output_color);
	ClassDB::bind_method(D_METHOD("get_connection_output_slot", "port"), &GraphNode::get_connection_output_slot);

	ClassDB::bind_method(D_METHOD("set_show_close_button", "show"), &GraphNode::set_show_close_button);
	ClassDB::bind_method(D_METHOD("is_close_button_visible"), &GraphNode::is_close_button_visible);

	ClassDB::bind_method(D_METHOD("set_overlay", "overlay"), &GraphNode::set_overlay);
	ClassDB::bind_method(D_METHOD("get_overlay"), &GraphNode::get_overlay);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "title"), "set_title", "get_title");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "position_offset", PROPERTY_HINT_NONE, "suffix:px"), "set_position_offset", "get_position_offset");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_close"), "set_show_close_button", "is_close_button_visible");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "resizable"), "set_resizable", "is_resizable");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "draggable"), "set_draggable", "is_draggable");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "selectable"), "set_selectable", "is_selectable");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "selected"), "set_selected", "is_selected");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "overlay", PROPERTY_HINT_ENUM, "Disabled,Breakpoint,Position"), "set_overlay", "get_overlay");

	ADD_GROUP("BiDi", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "text_direction", PROPERTY_HINT_ENUM, "Auto,Left-to-Right,Right-to-Left,Inherited"), "set_text_direction", "get_text_direction");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "language", PROPERTY_HINT_LOCALE_ID, ""), "set_language", "get_language");
	ADD_GROUP("", "");

	ADD_SIGNAL(MethodInfo("position_offset_changed"));
	ADD_SIGNAL(MethodInfo("node_selected"));
	ADD_SIGNAL(MethodInfo("node_deselected"));
	ADD_SIGNAL(MethodInfo("slot_updated", PropertyInfo(Variant::INT, "idx")));
	ADD_SIGNAL(MethodInfo("dragged", PropertyInfo(Variant::VECTOR2, "from"), PropertyInfo(Variant::VECTOR2, "to")));
	ADD_SIGNAL(MethodInfo("raise_request"));
	ADD_SIGNAL(MethodInfo("close_request"));
	ADD_SIGNAL(MethodInfo("resize_request", PropertyInfo(Variant::VECTOR2, "new_minsize")));

	BIND_ENUM_CONSTANT(OVERLAY_DISABLED);
	BIND_ENUM_CONSTANT(OVERLAY_BREAKPOINT);
	BIND_ENUM_CONSTANT(OVERLAY_POSITION);
}

GraphNode::GraphNode() {
	title_buf.instantiate();
	set_mouse_filter(MOUSE_FILTER_STOP);
}
