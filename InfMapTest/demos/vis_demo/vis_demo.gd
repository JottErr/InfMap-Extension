## Animates an InfluenceMap so the 3D view can be checked while it changes.
##
## Three sources orbit the grid and stamp a falloff into it every frame. Only
## the cells inside each radius are touched and the whole array goes over in one
## set_data call, so the per frame work stays far below the cell count.
extends Node3D

const CELLS: Vector2i = Vector2i(64, 64)
const CELL_SIZE: float = 0.5
const SOURCE_COUNT: int = 200
const SOURCE_RADIUS: int = 6

@onready var _view := $InfluenceMapView as InfluenceMapView3D

var _map: InfluenceMap = InfluenceMap.new()
var _values: PackedFloat32Array = PackedFloat32Array()
var _time: float = 0.0


func _ready() -> void:
	# cell_size before size is not required, but size has to be applied before
	# any data: set_data rejects an array that does not match the current size.
	_map.set_cell_size(CELL_SIZE)
	_map.set_size(CELLS)

	_values.resize(CELLS.x * CELLS.y)
	_view.influence_map = _map


func _process(delta: float) -> void:
	_time += delta

	_values.fill(0.0)
	
	for i in SOURCE_COUNT:
		var phase: float = _time * 0.1 + TAU * float(i) / float(SOURCE_COUNT) * 12
		var orbit: Vector2 = Vector2(cos(phase), sin(phase * 1.3)) * 0.4
		_stamp((Vector2(CELLS) * 0.5) + orbit * Vector2(CELLS)) 
	
	_map.set_data(_values)

 
# Writes a linear falloff around center, keeping whichever value is already
# there if it is higher, so overlapping sources pile up instead of erasing one
# another in whatever order they happen to be stamped.
func _stamp(center: Vector2) -> void:
	var min_x: int = maxi(int(center.x) - SOURCE_RADIUS, 0)
	var max_x: int = mini(int(center.x) + SOURCE_RADIUS, CELLS.x - 1)
	var min_y: int = maxi(int(center.y) - SOURCE_RADIUS, 0)
	var max_y: int = mini(int(center.y) + SOURCE_RADIUS, CELLS.y - 1)
	
	
	for y in range(min_y, max_y + 1):
		var row: int = y * CELLS.x
		for x in range(min_x, max_x + 1):
			var falloff: float = 1.0 - (Vector2(x, y).distance_to(center) / float(SOURCE_RADIUS))
			var index: int = row + x
			_values[index] = maxf(_values[index], clampf(falloff, 0.0, 1.0))
