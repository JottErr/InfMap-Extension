class_name EntityIntelligence
extends Node


@export var think_interval: float = 0.5

@onready var _entity := get_parent() as Entity

var _time_left: float = 0.0


func _physics_process(delta: float) -> void:
	if _entity.control_mode != Entity.ControlMode.INTELLIGENCE:
		_time_left = 0.0
		return

	_time_left -= delta
	if _time_left > 0.0:
		return
	_time_left = think_interval

	_entity.velocity = get_direction() * _entity.speed


func get_direction() -> Vector3:
	var target := get_tree().get_first_node_in_group(&"player") as Node3D
	if target == null:
		return Vector3.ZERO

	var offset: Vector3 = target.global_position - _entity.global_position
	offset.y = 0.0
	return offset.normalized()
