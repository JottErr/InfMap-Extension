## Animates an InfluenceMap so the 3D view can be checked while it changes.
##
## Many sources orbit the grid. Every frame the map is cleared and each source
## radiates its falloff into it, so overlapping sources add up.
extends Node3D

const CELLS: Vector2i = Vector2i(64, 64)
const CELL_SIZE: float = 0.5
const SOURCE_COUNT: int = 200
const SOURCE_RADIUS: int = 6
const MAGNITUDE: float = 0.2

## Falloff from a source's center (x = 0) to its edge (x = 1).
@export var falloff: Curve

@onready var _view := $InfluenceMapView as InfluenceMapView3D

var _map: InfluenceMap = InfluenceMap.new()
var _time: float = 0.0


func _ready() -> void:
	_map.set_cell_size(CELL_SIZE)
	_map.set_size(CELLS)

	_view.influence_map = _map


func _process(delta: float) -> void:
	_time += delta

	_map.reset_data()

	for i in SOURCE_COUNT:
		var phase: float = _time * 0.1 + TAU * float(i) / float(SOURCE_COUNT) * 21
		var orbit: Vector2 = Vector2(cos(phase), sin(phase * 1.3)) * 0.4
		var center := Vector2i((Vector2(CELLS) * 0.5) + orbit * Vector2(CELLS))
		_map.radiate_value_at_position(center, SOURCE_RADIUS, falloff, MAGNITUDE)
