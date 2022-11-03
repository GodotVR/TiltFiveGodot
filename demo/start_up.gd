extends Node2D

var connected_glasses = -1
var arvr_interface = null

func on_window_size_change():
	$TextureRect.rect_size = OS.window_size

# Called when the node enters the scene tree for the first time.
func _ready():
	$TextureRect.texture = $Viewport.get_texture()
	
	
	get_tree().get_root().connect("size_changed", self, "on_window_size_change")
	on_window_size_change();
	
	arvr_interface = ARVRServer.find_interface("TiltFive")
	
	if arvr_interface:
		TiltFiveManager.connect("glasses_event", self, "on_glasses_event")
		if TiltFiveManager.start_service("com.mygame", "0.1.0"):
			print("service started")

func on_glasses_event(glasses_num, event_num):
	print("glasses event ", glasses_num, ", ", event_num)
	if connected_glasses < 0 and event_num == 3:
		print("Trying to connect ", glasses_num)
		connected_glasses = glasses_num 
		TiltFiveManager.connect_glasses(glasses_num, "Godot Tilt Five")
	elif connected_glasses == glasses_num and event_num == 5:
		print("Trying to start interface ", connected_glasses)
		if arvr_interface.initialize():
			print("Init okay")
			$Viewport.arvr = true
		else:
			print("Failed okay")
			TiltFiveManager.disconnect_glasses(connected_glasses)
		
	
