extends ARVRController

export(Material) var selected_mat
export(Material) var unselected_mat

var stick_pos = Vector3()
var trigger_pos = Vector3()

enum WandControls {
	# Buttons
	WAND_BUTTON_A		= 0,
	WAND_BUTTON_B		= 1,
	WAND_BUTTON_X		= 2,
	WAND_BUTTON_Y		= 3,
	WAND_BUTTON_1		= 4,
	WAND_BUTTON_2		= 5,
	WAND_BUTTON_STICK	= 6,
	WAND_BUTTON_T5		= 7,
	# Axis
	WAND_ANALOG_X 		= 0,
	WAND_ANALOG_Y 		= 1,
	WAND_ANALOG_TRIGGER = 2,
}


# Called when the node enters the scene tree for the first time.
func _ready():
	stick_pos = $Controls/Three.transform.origin
	trigger_pos = $Controls/Trigger.transform.origin
	$Controls/A.material_override = unselected_mat
	$Controls/B.material_override = unselected_mat
	$Controls/X.material_override = unselected_mat
	$Controls/Y.material_override = unselected_mat
	$Controls/One.material_override = unselected_mat
	$Controls/Two.material_override = unselected_mat
	$Controls/Three.material_override = unselected_mat
	$Controls/T5.material_override = unselected_mat
	$Controls/Trigger.material_override = unselected_mat


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	var trigger = get_joystick_axis(WandControls.WAND_ANALOG_TRIGGER) * 0.03
	$Controls/Trigger.transform.origin = trigger_pos + Vector3(0, 0, trigger)
	var x_axis = get_joystick_axis(WandControls.WAND_ANALOG_X) * 0.03
	var y_axis = get_joystick_axis(WandControls.WAND_ANALOG_Y) * 0.03
	$Controls/Three.transform.origin = stick_pos + Vector3(x_axis, 0, -y_axis)


func _on_ARVRController_button_pressed(button):
	match button:
		WandControls.WAND_BUTTON_A:
			$Controls/A.material_override = selected_mat
		WandControls.WAND_BUTTON_B:
			$Controls/B.material_override = selected_mat
		WandControls.WAND_BUTTON_X:
			$Controls/X.material_override = selected_mat
		WandControls.WAND_BUTTON_Y:
			$Controls/Y.material_override = selected_mat
		WandControls.WAND_BUTTON_1:
			$Controls/One.material_override = selected_mat
		WandControls.WAND_BUTTON_2:
			$Controls/Two.material_override = selected_mat
		WandControls.WAND_BUTTON_STICK:
			$Controls/Three.material_override = selected_mat
		WandControls.WAND_BUTTON_T5:
			$Controls/T5.material_override = selected_mat


func _on_ARVRController_button_release(button):
	match button:
		WandControls.WAND_BUTTON_A:
			$Controls/A.material_override = unselected_mat
		WandControls.WAND_BUTTON_B:
			$Controls/B.material_override = unselected_mat
		WandControls.WAND_BUTTON_X:
			$Controls/X.material_override = unselected_mat
		WandControls.WAND_BUTTON_Y:
			$Controls/Y.material_override = unselected_mat
		WandControls.WAND_BUTTON_1:
			$Controls/One.material_override = unselected_mat
		WandControls.WAND_BUTTON_2:
			$Controls/Two.material_override = unselected_mat
		WandControls.WAND_BUTTON_STICK:
			$Controls/Three.material_override = unselected_mat
		WandControls.WAND_BUTTON_T5:
			$Controls/T5.material_override = unselected_mat
