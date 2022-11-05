extends Node2D

enum GlassesEvent {
	E_ADDED         = 1,
	E_LOST          = 2,
	E_AVAILABLE     = 3,
	E_UNAVAILABLE   = 4,
	E_CONNECTED     = 5,
	E_DISCONNECTED  = 6,
	E_TRACKING      = 7,
	E_NOT_TRACKING  = 8,
	E_STOPPED_ON_ERROR = 9
}


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
	print("Event on glasses# ", glasses_num, ", ", event_num)
	if connected_glasses >= 0 and connected_glasses != glasses_num:
		return
	match  event_num:
		GlassesEvent.E_AVAILABLE:
			print("Trying to connect glasses #", glasses_num)
			connected_glasses = glasses_num 
			TiltFiveManager.connect_glasses(glasses_num, "Godot Tilt Five")
		GlassesEvent.E_UNAVAILABLE:
			connected_glasses = -1
		GlassesEvent.E_CONNECTED:
			print("Trying to start interface for glasses #", connected_glasses)
			if arvr_interface.initialize():
				print("Init okay")
				$Viewport.arvr = true
			else:
				print("Failed to start interface")
				connected_glasses = -1
				TiltFiveManager.disconnect_glasses(connected_glasses)
		GlassesEvent.E_TRACKING:
			print("Started tracking glasses #",glasses_num)
		GlassesEvent.E_NOT_TRACKING:
			print("Stopped tracking glasses #",glasses_num)
	
		
	
