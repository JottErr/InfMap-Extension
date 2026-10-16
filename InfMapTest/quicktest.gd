extends Node


@export var infmap: InfluenceMap


func _ready() -> void:
	print(infmap)
	infmap.set_cell_value(Vector2i(2, 4), 3.5)
	print(infmap.get_cell_value(Vector2i(2, 4)))
	print(infmap.get_cell_value(Vector2i(2, 5)))
	print(infmap.get_cell_value(Vector2i(64, 64)))
	print(infmap.get_cell_value(Vector2i(65, 63)))
	print(infmap.get_cell_value(Vector2i(-1, 60)))
	infmap.set_size(Vector2i(64, 64))
	print(infmap.get_cell_value(Vector2i(2, 4)))
	


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass
