#include "influence_map.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/templates/local_vector.hpp>

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
	reset_data();
}

void InfluenceMap::reset_data(float p_value) {
	// The data invariant already holds the right length, so no resize is needed.
	data.fill(p_value);
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

void InfluenceMap::radiate_value_at_position(const Vector2i &p_center, int p_radius, const Ref<Curve> &p_curve, float p_magnitude) {
	ERR_FAIL_COND_MSG(p_curve.is_null(), "InfluenceMap cannot radiate without a curve.");
	ERR_FAIL_COND_MSG(p_radius < 0 || p_radius > MAX_RADIUS, vformat("InfluenceMap radius must be between 0 and %s. Got %s.", MAX_RADIUS, p_radius));

	const int radius_squared = p_radius * p_radius;

	// A cell's value depends only on its squared distance to the center, which
	// is a whole number from 0 to radius_squared. Sample the curve once per
	// squared distance up front, so the cell loop below needs no square root
	// and no call into the engine. Radius 0 leaves one entry for the center.
	LocalVector<float> falloff;
	falloff.resize(radius_squared + 1);
	const float min_domain = p_curve->get_min_domain();
	const float domain_per_cell = p_radius > 0 ? p_curve->get_domain_range() / p_radius : 0.0f;
	for (int distance_squared = 0; distance_squared <= radius_squared; distance_squared++) {
		const float offset = min_domain + Math::sqrt(float(distance_squared)) * domain_per_cell;
		falloff[distance_squared] = p_curve->sample_baked(offset) * p_magnitude;
	}

	// The circle's bounding box, clipped to the map. end is exclusive.
	const Vector2i start = (p_center - Vector2i(p_radius, p_radius)).max(Vector2i(0, 0));
	const Vector2i end = (p_center + Vector2i(p_radius + 1, p_radius + 1)).min(size);

	float *cells = data.ptrw();
	for (int y = start.y; y < end.y; y++) {
		const int dy = y - p_center.y;
		float *row = cells + cell_index(Vector2i(0, y));
		for (int x = start.x; x < end.x; x++) {
			const int dx = x - p_center.x;
			const int distance_squared = dx * dx + dy * dy;
			if (distance_squared <= radius_squared) {
				row[x] += falloff[distance_squared];
			}
		}
	}
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
	BIND_CONSTANT(MAX_RADIUS);

	ClassDB::bind_method(D_METHOD("set_cell_size", "cell_size"), &InfluenceMap::set_cell_size);
	ClassDB::bind_method(D_METHOD("get_cell_size"), &InfluenceMap::get_cell_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &InfluenceMap::set_size);
	ClassDB::bind_method(D_METHOD("get_size"), &InfluenceMap::get_size);
	ClassDB::bind_method(D_METHOD("set_data", "data"), &InfluenceMap::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &InfluenceMap::get_data);
	ClassDB::bind_method(D_METHOD("reset_data", "value"), &InfluenceMap::reset_data, DEFVAL(0.0f));

	ClassDB::bind_method(D_METHOD("is_in_bounds", "id"), &InfluenceMap::is_in_bounds);
	ClassDB::bind_method(D_METHOD("set_cell_value", "id", "value"), &InfluenceMap::set_cell_value);
	ClassDB::bind_method(D_METHOD("get_cell_value", "id"), &InfluenceMap::get_cell_value);

	ClassDB::bind_method(D_METHOD("radiate_value_at_position", "center", "radius", "curve", "magnitude"), &InfluenceMap::radiate_value_at_position, DEFVAL(1.0f));

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
