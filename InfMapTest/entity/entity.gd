class_name Entity
extends CharacterBody3D

enum ControlMode { PLAYER, INTELLIGENCE }

const PLAYER_COLOR: Color = Color(0.25, 0.5, 1.0)
const INTELLIGENCE_COLOR: Color = Color(1.0, 0.3, 0.25)

@export var control_mode: ControlMode = ControlMode.PLAYER:
	set(value):
		control_mode = value
		if is_node_ready():
			_apply_control_mode()

@export var speed: float = 2.8

@onready var _mesh := $Mesh as MeshInstance3D


func _ready() -> void:
	_apply_control_mode()


func _physics_process(_delta: float) -> void:
	# In INTELLIGENCE mode the Intelligence child sets the velocity.
	if control_mode == ControlMode.PLAYER:
		velocity = _read_input() * speed
	move_and_slide()


func _read_input() -> Vector3:
	var x: float = float(Input.is_physical_key_pressed(KEY_D)) - float(Input.is_physical_key_pressed(KEY_A))
	var z: float = float(Input.is_physical_key_pressed(KEY_S)) - float(Input.is_physical_key_pressed(KEY_W))
	return Vector3(x, 0.0, z).normalized()


func _apply_control_mode() -> void:
	# The Intelligence chases whoever is in this group.
	if control_mode == ControlMode.PLAYER:
		add_to_group(&"player")
	else:
		remove_from_group(&"player")

	var material := _mesh.material_override as StandardMaterial3D
	material.albedo_color = PLAYER_COLOR if control_mode == ControlMode.PLAYER else INTELLIGENCE_COLOR
