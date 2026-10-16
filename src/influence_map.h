#pragma once

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

	bool is_in_bounds(const godot::Vector2i &p_id) const;

	void set_cell_value(const godot::Vector2i &p_id, float p_value);
	float get_cell_value(const godot::Vector2i &p_id) const;
};

} // namespace infmap
