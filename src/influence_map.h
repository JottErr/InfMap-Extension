#pragma once

#include <godot_cpp/classes/curve.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/vector2i.hpp>

namespace infmap {

class InfluenceMap : public godot::Resource {
	GDCLASS(InfluenceMap, godot::Resource)

public:
	// Grid side length limits, counted in cells. MAX_SIZE holds the cell count
	// at 65536 or below, so size.x * size.y never overflows an int.
	static constexpr int MIN_SIZE = 1;
	static constexpr int MAX_SIZE = 256;

	// Largest radius radiate accepts, counted in cells. It caps the falloff table
	// at MAX_RADIUS * MAX_RADIUS + 1 entries.
	static constexpr int MAX_RADIUS = MAX_SIZE;

private:
	// World units per cell. Converts a world position into a cell id.
	float cell_size = 1.0f;

	// Grid width and height, counted in cells.
	// Invariant: both components stay inside MIN_SIZE..MAX_SIZE.
	godot::Vector2i size;

	// Cell values, row major. Index a cell with x + y * size.x.
	// Invariant: data.size() == size.x * size.y
	godot::PackedFloat32Array data;

	// The only place the row-major index math lives.
	// Caller must check is_in_bounds first.
	int cell_index(const godot::Vector2i &p_id) const {
		return p_id.x + p_id.y * size.x;
	}

protected:
	static void _bind_methods();

public:
	InfluenceMap();
	~InfluenceMap() override = default;

	void set_cell_size(float p_cell_size);
	float get_cell_size() const { return cell_size; }

	// Clamps p_size into MIN_SIZE..MAX_SIZE, reallocates data, and clears every
	// cell to 0.
	void set_size(const godot::Vector2i &p_size);
	godot::Vector2i get_size() const { return size; }

	// Saved with the resource and hidden from the inspector. set_data rejects an
	// array whose length does not match the current size, so size must be
	// applied first. _bind_methods registers size ahead of data for that reason.
	void set_data(const godot::PackedFloat32Array &p_data);
	godot::PackedFloat32Array get_data() const { return data; }

	// Sets every cell to p_value. Keeps the current size.
	void reset_data(float p_value = 0.0f);

	bool is_in_bounds(const godot::Vector2i &p_id) const;

	void set_cell_value(const godot::Vector2i &p_id, float p_value);
	float get_cell_value(const godot::Vector2i &p_id) const;

	// Adds a circle of influence to every cell within p_radius of p_center.
	// p_curve maps the distance to the center onto a value: the curve's min
	// domain is the center, its max domain is the edge. The result is scaled by
	// p_magnitude, so a negative magnitude removes what an earlier call added.
	// Cells outside the map are skipped, so p_center may lie off the map.
	void radiate_value_at_position(const godot::Vector2i &p_center, int p_radius, const godot::Ref<godot::Curve> &p_curve, float p_magnitude = 1.0f);

	// Adds p_other's cells scaled by p_magnitude, with p_other's middle cell
	// (its size / 2) placed on p_center. Cells that fall outside this map are
	// skipped. Meant for stamping a small template many times instead of
	// radiating the same circle again for every source.
	void add_map(const godot::Ref<InfluenceMap> &p_other, const godot::Vector2i &p_center, float p_magnitude = 1.0f);

	// Cell values as a size.x by size.y image, one cell per pixel. FORMAT_RF is
	// a single 32 bit float per pixel in row-major order, which is the layout
	// data already has, so the conversion is one memcpy and no per-cell work.
	godot::Ref<godot::Image> to_image() const;

	// Overwrites an existing image instead of allocating a new one. Pair it with
	// ImageTexture::update to refresh a texture in place, which keeps the GPU
	// side allocation across frames.
	void update_image(const godot::Ref<godot::Image> &p_image) const;
};

} // namespace infmap
