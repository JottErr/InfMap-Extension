## Draws an InfluenceMap as a flat heatmap on the ground plane.
##
## The whole map is one quad with one shader, so the draw cost does not grow
## with the cell count. Cell values reach the GPU as a single channel float
## texture that is rewritten in place.
class_name InfluenceMapView3D
extends MeshInstance3D

const SHADER: Shader = preload("uid://dqv3bb4hfoj8p")

@export var influence_map: InfluenceMap:
	set(value):
		influence_map = value
		_rebuild()

@export var color_ramp: Texture2D = preload("uid://bnysh07qcsygu"):
	set(value):
		color_ramp = value
		_push_params()

@export var value_min: float = 0.0:
	set(value):
		value_min = value
		_push_params()

@export var value_max: float = 1.0:
	set(value):
		value_max = value
		_push_params()

@export var smooth_cells: bool = true:
	set(value):
		smooth_cells = value
		_push_params()

## Re-uploads the texture every frame. Turn it off and call refresh() instead
## when the map changes less often than the frame rate.
@export var auto_refresh: bool = true:
	set(value):
		auto_refresh = value
		set_process(value)

@export var height_offset: float = 0.01:
	set(value):
		height_offset = value
		position.y = value

@export_group("Cell Grid")

@export var show_grid: bool = false:
	set(value):
		show_grid = value
		_push_params()

@export var grid_color: Color = Color(0.0, 0.0, 0.0, 0.25):
	set(value):
		grid_color = value
		_push_params()

@export_range(0.5, 4.0, 0.1) var grid_width: float = 1.0:
	set(value):
		grid_width = value
		_push_params()

@export_group("")

var _image: Image
var _texture: ImageTexture
var _material: ShaderMaterial


func _ready() -> void:
	position.y = height_offset
	set_process(auto_refresh)
	_rebuild()


func _process(_delta: float) -> void:
	refresh()


## Uploads the current cell values. Reuses the image buffer and the GPU texture,
## so a frame costs one memcpy plus a four byte per cell transfer.
func refresh() -> void:
	if influence_map == null or _image == null:
		return

	# ImageTexture.update rejects a change in dimensions, and set_size clears the
	# map anyway, so a resize behind our back has to go through a full rebuild.
	if Vector2i(_image.get_width(), _image.get_height()) != influence_map.get_size():
		_rebuild()
		return

	influence_map.update_image(_image)
	_texture.update(_image)


func _rebuild() -> void:
	if influence_map == null:
		mesh = null
		_image = null
		_texture = null
		return

	_image = influence_map.to_image()
	_texture = ImageTexture.create_from_image(_image)
	mesh = _build_quad(Vector2(influence_map.get_size()) * influence_map.get_cell_size())

	if _material == null:
		_material = ShaderMaterial.new()
		_material.shader = SHADER
	material_override = _material

	_push_params()


func _build_quad(extent: Vector2) -> PlaneMesh:
	var quad: PlaneMesh = PlaneMesh.new()
	quad.size = extent
	quad.center_offset = Vector3(extent.x * 0.5, 0.0, extent.y * 0.5)
	return quad


func _push_params() -> void:
	if _material == null:
		return

	_material.set_shader_parameter("infmap", _texture)
	_material.set_shader_parameter("ramp", color_ramp)
	_material.set_shader_parameter("value_min", value_min)
	_material.set_shader_parameter("value_max", value_max)
	_material.set_shader_parameter("smooth_values", smooth_cells)
	_material.set_shader_parameter("show_grid", show_grid)
	_material.set_shader_parameter("grid_color", grid_color)
	_material.set_shader_parameter("grid_width", grid_width)

	var cells: Vector2 = Vector2.ONE
	if influence_map != null:
		cells = Vector2(influence_map.get_size())
	_material.set_shader_parameter("map_size", cells)
