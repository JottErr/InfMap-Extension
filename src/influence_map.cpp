#include "influence_map.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace infmap {

InfluenceMap::InfluenceMap() {
	// Start at the smallest valid grid. A fresh InfluenceMap then holds one cell
	// instead of none, so data is never empty and the size limits always hold.
	set_size(Vector2i(MIN_SIZE, MIN_SIZE));
}

void InfluenceMap::set_cell_size(float p_cell_size) {
	ERR_FAIL_COND_MSG(p_cell_size <= 0.0f, vformat("InfluenceMap cell_size must be greater than 0. Got %s.", p_cell_size));

	cell_size = p_cell_size;
}

void InfluenceMap::set_size(const Vector2i &p_size) {
	const Vector2i clamped_size = p_size.clampi(MIN_SIZE, MAX_SIZE);
	if (clamped_size != p_size) {
		WARN_PRINT(vformat("InfluenceMap size %s is out of range and was clamped to %s.", p_size, clamped_size));
	}

	// Both components are at most MAX_SIZE here, so this product cannot overflow.
	const int cell_count = clamped_size.x * clamped_size.y;

	// Assign size only after the step that can fail, so the data invariant holds.
	ERR_FAIL_COND_MSG(data.resize(cell_count) != OK, vformat("InfluenceMap could not allocate %s cells.", cell_count));
	size = clamped_size;
	data.fill(0.0f);
}

void InfluenceMap::set_data(const PackedFloat32Array &p_data) {
	ERR_FAIL_COND_MSG(p_data.size() != data.size(), vformat("InfluenceMap data holds %s values, but size %s needs %s. Apply size before data.", p_data.size(), size, data.size()));

	data = p_data;
}

bool InfluenceMap::is_in_bounds(const Vector2i &p_id) const {
	return p_id.x >= 0 && p_id.y >= 0 && p_id.x < size.x && p_id.y < size.y;
}

void InfluenceMap::set_cell_value(const Vector2i &p_id, float p_value) {
	ERR_FAIL_COND_MSG(!is_in_bounds(p_id), vformat("Cell %s lies outside the InfluenceMap of size %s.", p_id, size));

	data[cell_index(p_id)] = p_value;
}

float InfluenceMap::get_cell_value(const Vector2i &p_id) const {
	ERR_FAIL_COND_V_MSG(!is_in_bounds(p_id), 0.0f, vformat("Cell %s lies outside the InfluenceMap of size %s.", p_id, size));

	return data[cell_index(p_id)];
}

Ref<Image> InfluenceMap::to_image() const {
	// The data invariant guarantees size.x * size.y values, which is exactly the
	// pixel count FORMAT_RF expects, so this cannot fail on a valid map.
	return Image::create_from_data(size.x, size.y, false, Image::FORMAT_RF, data.to_byte_array());
}

void InfluenceMap::update_image(const Ref<Image> &p_image) const {
	ERR_FAIL_COND_MSG(p_image.is_null(), "InfluenceMap cannot write cell values into a null image.");

	p_image->set_data(size.x, size.y, false, Image::FORMAT_RF, data.to_byte_array());
}

void InfluenceMap::_bind_methods() {
	BIND_CONSTANT(MIN_SIZE);
	BIND_CONSTANT(MAX_SIZE);

	ClassDB::bind_method(D_METHOD("set_cell_size", "cell_size"), &InfluenceMap::set_cell_size);
	ClassDB::bind_method(D_METHOD("get_cell_size"), &InfluenceMap::get_cell_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &InfluenceMap::set_size);
	ClassDB::bind_method(D_METHOD("get_size"), &InfluenceMap::get_size);
	ClassDB::bind_method(D_METHOD("set_data", "data"), &InfluenceMap::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &InfluenceMap::get_data);

	ClassDB::bind_method(D_METHOD("is_in_bounds", "id"), &InfluenceMap::is_in_bounds);
	ClassDB::bind_method(D_METHOD("set_cell_value", "id", "value"), &InfluenceMap::set_cell_value);
	ClassDB::bind_method(D_METHOD("get_cell_value", "id"), &InfluenceMap::get_cell_value);

	ClassDB::bind_method(D_METHOD("to_image"), &InfluenceMap::to_image);
	ClassDB::bind_method(D_METHOD("update_image", "image"), &InfluenceMap::update_image);

	// Property names must match the bound setter and getter above. These strings
	// are resolved at runtime, so a typo here fails when Godot loads the extension.
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cell_size", PROPERTY_HINT_RANGE, "0.01,64,0.01,or_greater"), "set_cell_size", "get_cell_size");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "size", PROPERTY_HINT_RANGE, "1,256,1"), "set_size", "get_size");

	// Registered after size, because Godot applies properties in this order when
	// it loads a saved resource and set_data needs the final size to validate
	// against. PROPERTY_USAGE_STORAGE saves the cell values without putting
	// thousands of floats in the inspector.
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "set_data", "get_data");
}

} // namespace infmap
