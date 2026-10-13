#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/vector2i.hpp>

namespace infmap {

class InfluenceMap : public godot::Resource {
	GDCLASS(InfluenceMap, godot::Resource)

private:
	// World units per cell. Converts a world position into a cell id.
	float cell_size = 1.0f;

	// Grid width and height, counted in cells.
	godot::Vector2i size;

	// Cell values, row major. Index a cell with x + y * size.x.
	// Invariant: data.size() == size.x * size.y
	godot::PackedFloat32Array data;

protected:
	static void _bind_methods();

public:
	InfluenceMap() = default;
	~InfluenceMap() override = default;

};

} // namespace infmap
