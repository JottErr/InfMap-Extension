// Draws one InfluenceMap as a flat heatmap on the ground.
//
// Cell values arrive as a single channel float texture, so the fragment cost is
// one fetch per pixel regardless of how many cells the map holds.
shader_type spatial;

render_mode unshaded, cull_disabled, depth_draw_never, blend_mix;

uniform sampler2D infmap : filter_linear, repeat_disable;
uniform sampler2D ramp : source_color, filter_linear, repeat_disable;

// Cell counts per axis. Locates cell centers and cell borders in UV space.
uniform vec2 map_size = vec2(1.0);

uniform float value_min = 0.0;
uniform float value_max = 1.0;

uniform bool smooth_values = true;

uniform bool show_grid = false;
uniform vec4 grid_color : source_color = vec4(0.0, 0.0, 0.0, 0.25);
uniform float grid_width = 1.0;

void fragment() {
	vec2 uv = UV;
	if (!smooth_values) {
		uv = (floor(uv * map_size) + 0.5) / map_size;
	}

	float value = texture(infmap, uv).r;
	float t = clamp((value - value_min) / max(value_max - value_min, 1e-6), 0.0, 1.0);
	vec4 color = texture(ramp, vec2(t, 0.0));

	if (show_grid) {
		// Distance to the nearest cell border, counted in cells, over the size
		// of one pixel in that same unit. Dividing by the derivative holds the
		// line at a constant width on screen however close the camera gets.
		vec2 cell = fract(UV * map_size);
		vec2 border = min(cell, 1.0 - cell) / max(fwidth(UV * map_size) * grid_width, vec2(1e-6));
		float line = 1.0 - clamp(min(border.x, border.y), 0.0, 1.0);

		// Alpha takes the larger of the two so the grid stays visible over the
		// transparent end of the ramp instead of fading out with it.
		color.rgb = mix(color.rgb, grid_color.rgb, grid_color.a * line);
		color.a = max(color.a, grid_color.a * line);
	}

	ALBEDO = color.rgb;
	ALPHA = color.a;
}
