extends ARVRController


# Declare member variables here. Examples:
# var a = 2
# var b = "text"


# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	var trigger = get_joystick_axis(JOY_VR_ANALOG_TRIGGER) * 0.5
	$Pointer.transform.origin = Vector3(0, 0, trigger)
	$Pointer.scale = Vector3(0.02, 0.02, 0.02 + trigger)
	
