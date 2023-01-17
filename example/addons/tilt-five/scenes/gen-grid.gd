extends Node


var box_temp = preload("res://addons/tilt-five/scenes/color-box.tscn")

func _ready():
	for x in range(-8, 8):
		for z in range(-8,8):
			var box_inst = box_temp.instance()
			box_inst.transform.origin = Vector3(x, 0, z)
			box_inst.box_color = Color((x + 8.0)/16.0, (z + 8.0)/16.0, 0, 1)
			add_child(box_inst)

