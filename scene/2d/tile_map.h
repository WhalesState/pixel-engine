/**************************************************************************/
/*  tile_map.h                                                            */
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

#ifndef TILE_MAP_H
#define TILE_MAP_H

#include "scene/2d/node_2d.h"
#include "scene/gui/control.h"
#include "scene/resources/tile_set.h"

class TileSetAtlasSource;

class TerrainConstraint {
private:
	const TileMap *tile_map = nullptr;
	Vector2i base_cell_coords;
	int bit = -1;
	int terrain = -1;

	int priority = 1;

public:
	bool operator<(const TerrainConstraint &p_other) const {
		if (base_cell_coords == p_other.base_cell_coords) {
			return bit < p_other.bit;
		}
		return base_cell_coords < p_other.base_cell_coords;
	}

	String to_string() const {
		return vformat("Constraint {pos:%s, bit:%d, terrain:%d, priority:%d}", base_cell_coords, bit, terrain, priority);
	}

	Vector2i get_base_cell_coords() const {
		return base_cell_coords;
	}

	bool is_center_bit() const {
		return bit == 0;
	}

	HashMap<Vector2i, TileSet::CellNeighbor> get_overlapping_coords_and_peering_bits() const;

	void set_terrain(int p_terrain) {
		terrain = p_terrain;
	}

	int get_terrain() const {
		return terrain;
	}

	void set_priority(int p_priority) {
		priority = p_priority;
	}

	int get_priority() const {
		return priority;
	}

	TerrainConstraint(const TileMap *p_tile_map, const Vector2i &p_position, int p_terrain); // For the center terrain bit
	TerrainConstraint(const TileMap *p_tile_map, const Vector2i &p_position, const TileSet::CellNeighbor &p_bit, int p_terrain); // For peering bits
	TerrainConstraint(){};
};

struct TileMapQuadrant {
	struct CoordsWorldComparator {
		_ALWAYS_INLINE_ bool operator()(const Vector2 &p_a, const Vector2 &p_b) const {
			// We sort the cells by their local coords, as it is needed by rendering.
			if (p_a.y == p_b.y) {
				return p_a.x > p_b.x;
			} else {
				return p_a.y < p_b.y;
			}
		}
	};

	// Dirty list element.
	SelfList<TileMapQuadrant> dirty_list_element;

	// Quadrant coords.
	Vector2i coords;

	// TileMapCells.
	RBSet<Vector2i> cells;
	// We need those two maps to sort by local position for rendering.
	// This is kind of workaround, it would be better to sort the cells directly in the "cells" set instead.
	RBMap<Vector2i, Vector2> map_to_local;
	RBMap<Vector2, Vector2i, CoordsWorldComparator> local_to_map;

	// Debug.
	RID debug_canvas_item;

	// Rendering.
	List<RID> canvas_items;
	HashMap<Vector2i, RID> occluders;

	// Physics.
	List<RID> bodies;

	// Scenes.
	HashMap<Vector2i, String> scenes;

	// Runtime TileData cache.
	HashMap<Vector2i, TileData *> runtime_tile_data_cache;

	void operator=(const TileMapQuadrant &q) {
		coords = q.coords;
		debug_canvas_item = q.debug_canvas_item;
		canvas_items = q.canvas_items;
		occluders = q.occluders;
		bodies = q.bodies;
	}

	TileMapQuadrant(const TileMapQuadrant &q) :
			dirty_list_element(this) {
		coords = q.coords;
		debug_canvas_item = q.debug_canvas_item;
		canvas_items = q.canvas_items;
		occluders = q.occluders;
		bodies = q.bodies;
	}

	TileMapQuadrant() :
			dirty_list_element(this) {
	}
};

class TileMapLayer : public RefCounted {
public:
	enum DataFormat {
		FORMAT_1 = 0,
		FORMAT_2,
		FORMAT_3,
		FORMAT_MAX,
	};

private:
	// Exposed properties.
	String name;
	bool enabled = true;
	Color modulate = Color(1, 1, 1, 1);
	bool y_sort_enabled = false;
	int y_sort_origin = 0;
	int z_index = 0;

	// Internal.
	TileMap *tile_map_node = nullptr;
	int layer_index_in_tile_map_node = -1;
	RID canvas_item;
	bool _rendering_quadrant_order_dirty = false;
	HashMap<Vector2i, TileMapCell> tile_map;
	HashMap<Vector2i, TileMapQuadrant> quadrant_map;
	SelfList<TileMapQuadrant>::List dirty_quadrant_list;

	// Rect cache.
	mutable Rect2 rect_cache;
	mutable bool rect_cache_dirty = true;
	mutable Rect2i used_rect_cache;
	mutable bool used_rect_cache_dirty = true;

	// Quadrants management.
	Vector2i _coords_to_quadrant_coords(const Vector2i &p_coords) const;
	HashMap<Vector2i, TileMapQuadrant>::Iterator _create_quadrant(const Vector2i &p_qk);
	void _make_quadrant_dirty(HashMap<Vector2i, TileMapQuadrant>::Iterator Q);
	void _erase_quadrant(HashMap<Vector2i, TileMapQuadrant>::Iterator Q);

	// Per-system methods.
	void _rendering_notification(int p_what);
	void _rendering_update();
	void _rendering_cleanup();
	void _rendering_update_dirty_quadrants(SelfList<TileMapQuadrant>::List &r_dirty_quadrant_list);
	void _rendering_reorder_quadrants(int &r_index);
	void _rendering_create_quadrant(TileMapQuadrant *p_quadrant);
	void _rendering_cleanup_quadrant(TileMapQuadrant *p_quadrant);
	void _rendering_draw_quadrant_debug(TileMapQuadrant *p_quadrant);

	HashMap<RID, Vector2i> bodies_coords; // Mapping for RID to coords.
	void _physics_update_dirty_quadrants(SelfList<TileMapQuadrant>::List &r_dirty_quadrant_list);
	void _physics_cleanup_quadrant(TileMapQuadrant *p_quadrant);
	void _physics_draw_quadrant_debug(TileMapQuadrant *p_quadrant);

	HashSet<Vector2i> instantiated_scenes;
	void _scenes_update_dirty_quadrants(SelfList<TileMapQuadrant>::List &r_dirty_quadrant_list);
	void _scenes_cleanup_quadrant(TileMapQuadrant *p_quadrant);
	void _scenes_draw_quadrant_debug(TileMapQuadrant *p_quadrant);

	// Runtime tile data.
	void _build_runtime_update_tile_data(SelfList<TileMapQuadrant>::List &r_dirty_quadrant_list);

	// Terrains.
	TileSet::TerrainsPattern _get_best_terrain_pattern_for_constraints(int p_terrain_set, const Vector2i &p_position, const RBSet<TerrainConstraint> &p_constraints, TileSet::TerrainsPattern p_current_pattern);
	RBSet<TerrainConstraint> _get_terrain_constraints_from_added_pattern(const Vector2i &p_position, int p_terrain_set, TileSet::TerrainsPattern p_terrains_pattern) const;
	RBSet<TerrainConstraint> _get_terrain_constraints_from_painted_cells_list(const RBSet<Vector2i> &p_painted, int p_terrain_set, bool p_ignore_empty_terrains) const;

public:
	// TileMap node.
	void set_tile_map(TileMap *p_tile_map);
	void set_layer_index_in_tile_map_node(int p_index);

	// Rect caching.
	Rect2 get_rect(bool &r_changed) const;

	// Terrains.
	HashMap<Vector2i, TileSet::TerrainsPattern> terrain_fill_constraints(const Vector<Vector2i> &p_to_replace, int p_terrain_set, const RBSet<TerrainConstraint> &p_constraints); // Not exposed.
	HashMap<Vector2i, TileSet::TerrainsPattern> terrain_fill_connect(const Vector<Vector2i> &p_coords_array, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true); // Not exposed.
	HashMap<Vector2i, TileSet::TerrainsPattern> terrain_fill_path(const Vector<Vector2i> &p_coords_array, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true); // Not exposed.
	HashMap<Vector2i, TileSet::TerrainsPattern> terrain_fill_pattern(const Vector<Vector2i> &p_coords_array, int p_terrain_set, TileSet::TerrainsPattern p_terrains_pattern, bool p_ignore_empty_terrains = true); // Not exposed.

	// Not exposed to users.
	TileMapCell get_cell(const Vector2i &p_coords, bool p_use_proxies = false) const;
	int get_effective_quadrant_size() const;

	// For TileMap node's use.
	void notify_canvas_entered();
	void notify_visibility_changed();
	void notify_xform_changed();
	void notify_local_xform_changed();
	void notify_canvas_exited();
	void notify_selected_layer_changed();
	void notify_light_mask_changed();
	void notify_material_changed();
	void notify_use_parent_material_changed();
	void notify_texture_filter_changed();
	void notify_texture_repeat_changed();
	void update_dirty_quadrants();
	void set_tile_data(DataFormat p_format, const Vector<int> &p_data);
	Vector<int> get_tile_data() const;
	void clear_instantiated_scenes();
	void clear_internals(); // Exposed for now to tilemap, but ideally, we should avoid it.
	void recreate_internals(); // Exposed for now to tilemap, but ideally, we should avoid it.

	// --- Exposed in TileMap ---

	// Cells manipulation.
	void set_cell(const Vector2i &p_coords, int p_source_id = TileSet::INVALID_SOURCE, const Vector2i p_atlas_coords = TileSetSource::INVALID_ATLAS_COORDS, int p_alternative_tile = 0);
	void erase_cell(const Vector2i &p_coords);

	int get_cell_source_id(const Vector2i &p_coords, bool p_use_proxies = false) const;
	Vector2i get_cell_atlas_coords(const Vector2i &p_coords, bool p_use_proxies = false) const;
	int get_cell_alternative_tile(const Vector2i &p_coords, bool p_use_proxies = false) const;
	TileData *get_cell_tile_data(const Vector2i &p_coords, bool p_use_proxies = false) const; // Helper method to make accessing the data easier.
	void clear();

	// Patterns.
	Ref<TileMapPattern> get_pattern(TypedArray<Vector2i> p_coords_array);
	void set_pattern(const Vector2i &p_position, const Ref<TileMapPattern> p_pattern);

	// Terrains.
	void set_cells_terrain_connect(TypedArray<Vector2i> p_cells, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true);
	void set_cells_terrain_path(TypedArray<Vector2i> p_path, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true);

	// Cells usage.
	TypedArray<Vector2i> get_used_cells() const;
	TypedArray<Vector2i> get_used_cells_by_id(int p_source_id = TileSet::INVALID_SOURCE, const Vector2i p_atlas_coords = TileSetSource::INVALID_ATLAS_COORDS, int p_alternative_tile = TileSetSource::INVALID_TILE_ALTERNATIVE) const;
	Rect2i get_used_rect() const;

	// Layer properties.
	void set_name(String p_name);
	String get_name() const;
	void set_enabled(bool p_enabled);
	bool is_enabled() const;
	void set_modulate(Color p_modulate);
	Color get_modulate() const;
	void set_y_sort_enabled(bool p_y_sort_enabled);
	bool is_y_sort_enabled() const;
	void set_y_sort_origin(int p_y_sort_origin);
	int get_y_sort_origin() const;
	void set_z_index(int p_z_index);
	int get_z_index() const;

	// In case something goes wrong.
	void force_update();

	// Fixing and clearing methods.
	void fix_invalid_tiles();

	// Find coords for body.
	bool has_body_rid(RID p_physics_body) const;
	Vector2i get_coords_for_body_rid(RID p_physics_body) const; // For finding tiles from collision.
};

class TileMap : public Node2D {
	GDCLASS(TileMap, Node2D);

public:
	enum VisibilityMode {
		VISIBILITY_MODE_DEFAULT,
		VISIBILITY_MODE_FORCE_SHOW,
		VISIBILITY_MODE_FORCE_HIDE,
	};

private:
	friend class TileSetPlugin;

	// A compatibility enum to specify how is the data if formatted.
	mutable TileMapLayer::DataFormat format = TileMapLayer::FORMAT_3;

	static constexpr float FP_ADJUST = 0.00001;

	// Properties.
	Ref<TileSet> tile_set;
	int quadrant_size = 16;
	bool collision_animatable = false;
	VisibilityMode collision_visibility_mode = VISIBILITY_MODE_DEFAULT;

	// Layers.
	LocalVector<Ref<TileMapLayer>> layers;
	int selected_layer = -1;

	void _clear_internals();
	void _recreate_internals();

	bool pending_update = false;

	Transform2D last_valid_transform;
	Transform2D new_transform;

	void _tile_set_changed();
	bool _tile_set_changed_deferred_update_needed = false;
	void _tile_set_changed_deferred_update();

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	void _notification(int p_what);
	static void _bind_methods();

#ifndef DISABLE_DEPRECATED
	Rect2i _get_used_rect_bind_compat_78328();
	static void _bind_compatibility_methods();
#endif

public:
	static Vector2i transform_coords_layout(const Vector2i &p_coords, TileSet::TileOffsetAxis p_offset_axis, TileSet::TileLayout p_from_layout, TileSet::TileLayout p_to_layout);

#ifdef TOOLS_ENABLED
	virtual Rect2 _edit_get_rect() const override;
#endif

	// Called by TileMapLayers.
	void queue_update_dirty_quadrants();
	void _update_dirty_quadrants();

	void set_tileset(const Ref<TileSet> &p_tileset);
	Ref<TileSet> get_tileset() const;

	void set_quadrant_size(int p_size);
	int get_quadrant_size() const;

	static void draw_tile(RID p_canvas_item, const Vector2 &p_position, const Ref<TileSet> p_tile_set, int p_atlas_source_id, const Vector2i &p_atlas_coords, int p_alternative_tile, int p_frame = -1, Color p_modulation = Color(1.0, 1.0, 1.0, 1.0), const TileData *p_tile_data_override = nullptr, real_t p_animation_offset = 0.0);

	// Layers management.
	int get_layers_count() const;
	void add_layer(int p_to_pos);
	void move_layer(int p_layer, int p_to_pos);
	void remove_layer(int p_layer);

	void set_layer_name(int p_layer, String p_name);
	String get_layer_name(int p_layer) const;
	void set_layer_enabled(int p_layer, bool p_visible);
	bool is_layer_enabled(int p_layer) const;
	void set_layer_modulate(int p_layer, Color p_modulate);
	Color get_layer_modulate(int p_layer) const;
	void set_layer_y_sort_enabled(int p_layer, bool p_enabled);
	bool is_layer_y_sort_enabled(int p_layer) const;
	void set_layer_y_sort_origin(int p_layer, int p_y_sort_origin);
	int get_layer_y_sort_origin(int p_layer) const;
	void set_layer_z_index(int p_layer, int p_z_index);
	int get_layer_z_index(int p_layer) const;
	void set_selected_layer(int p_layer_id); // For editor use.
	int get_selected_layer() const;

	void set_collision_animatable(bool p_enabled);
	bool is_collision_animatable() const;

	// Debug visibility modes.
	void set_collision_visibility_mode(VisibilityMode p_show_collision);
	VisibilityMode get_collision_visibility_mode();

	// Cells accessors.
	void set_cell(int p_layer, const Vector2i &p_coords, int p_source_id = TileSet::INVALID_SOURCE, const Vector2i p_atlas_coords = TileSetSource::INVALID_ATLAS_COORDS, int p_alternative_tile = 0);
	void erase_cell(int p_layer, const Vector2i &p_coords);
	int get_cell_source_id(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;
	Vector2i get_cell_atlas_coords(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;
	int get_cell_alternative_tile(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;
	// Helper method to make accessing the data easier.
	TileData *get_cell_tile_data(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;

	// Patterns.
	Ref<TileMapPattern> get_pattern(int p_layer, TypedArray<Vector2i> p_coords_array);
	Vector2i map_pattern(const Vector2i &p_position_in_tilemap, const Vector2i &p_coords_in_pattern, Ref<TileMapPattern> p_pattern);
	void set_pattern(int p_layer, const Vector2i &p_position, const Ref<TileMapPattern> p_pattern);

	// Terrains (Not exposed).
	HashMap<Vector2i, TileSet::TerrainsPattern> terrain_fill_constraints(int p_layer, const Vector<Vector2i> &p_to_replace, int p_terrain_set, const RBSet<TerrainConstraint> &p_constraints);
	HashMap<Vector2i, TileSet::TerrainsPattern> terrain_fill_connect(int p_layer, const Vector<Vector2i> &p_coords_array, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true);
	HashMap<Vector2i, TileSet::TerrainsPattern> terrain_fill_path(int p_layer, const Vector<Vector2i> &p_coords_array, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true);
	HashMap<Vector2i, TileSet::TerrainsPattern> terrain_fill_pattern(int p_layer, const Vector<Vector2i> &p_coords_array, int p_terrain_set, TileSet::TerrainsPattern p_terrains_pattern, bool p_ignore_empty_terrains = true);

	// Terrains (exposed).
	void set_cells_terrain_connect(int p_layer, TypedArray<Vector2i> p_cells, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true);
	void set_cells_terrain_path(int p_layer, TypedArray<Vector2i> p_path, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains = true);

	// Not exposed to users.
	TileMapCell get_cell(int p_layer, const Vector2i &p_coords, bool p_use_proxies = false) const;
	int get_effective_quadrant_size(int p_layer) const;

	virtual void set_y_sort_enabled(bool p_enable) override;

	Vector2 map_to_local(const Vector2i &p_pos) const;
	Vector2i local_to_map(const Vector2 &p_pos) const;

	bool is_existing_neighbor(TileSet::CellNeighbor p_cell_neighbor) const;
	Vector2i get_neighbor_cell(const Vector2i &p_coords, TileSet::CellNeighbor p_cell_neighbor) const;

	TypedArray<Vector2i> get_used_cells(int p_layer) const;
	TypedArray<Vector2i> get_used_cells_by_id(int p_layer, int p_source_id = TileSet::INVALID_SOURCE, const Vector2i p_atlas_coords = TileSetSource::INVALID_ATLAS_COORDS, int p_alternative_tile = TileSetSource::INVALID_TILE_ALTERNATIVE) const;
	Rect2i get_used_rect() const;

	// Override some methods of the CanvasItem class to pass the changes to the quadrants CanvasItems.
	virtual void set_light_mask(int p_light_mask) override;
	virtual void set_material(const Ref<Material> &p_material) override;
	virtual void set_use_parent_material(bool p_use_parent_material) override;
	virtual void set_texture_filter(CanvasItem::TextureFilter p_texture_filter) override;
	virtual void set_texture_repeat(CanvasItem::TextureRepeat p_texture_repeat) override;

	// For finding tiles from collision.
	Vector2i get_coords_for_body_rid(RID p_physics_body);
	// For getting their layers as well.
	int get_layer_for_body_rid(RID p_physics_body);

	// Fixing and clearing methods.
	void fix_invalid_tiles();

	// Clears tiles from a given layer.
	void clear_layer(int p_layer);
	void clear();

	// Force a TileMap update.
	void force_update(int p_layer = -1);

	// Helpers?
	TypedArray<Vector2i> get_surrounding_cells(const Vector2i &coords);
	void draw_cells_outline(Control *p_control, const RBSet<Vector2i> &p_cells, Color p_color, Transform2D p_transform = Transform2D());

	// Virtual function to modify the TileData at runtime.
	GDVIRTUAL2R(bool, _use_tile_data_runtime_update, int, Vector2i);
	GDVIRTUAL3(_tile_data_runtime_update, int, Vector2i, TileData *);

	// Configuration warnings.
	PackedStringArray get_configuration_warnings() const override;

	TileMap();
	~TileMap();
};

VARIANT_ENUM_CAST(TileMap::VisibilityMode);

#endif // TILE_MAP_H
