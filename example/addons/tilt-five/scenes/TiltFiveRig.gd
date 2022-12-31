extends Viewport

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

var connected_glasses = null

func _ready():
	
	TiltFiveManager.connect("glasses_event", self, "on_glasses_event")
	if TiltFiveManager.start_service("com.mygame", "0.1.0"):
		print("service started")
	else:
		print("Failed to start service")

func on_glasses_event(glasses_id, event_num):
	print("Event on glasses ", glasses_id, ", ", GlassesEvent.keys()[event_num-1])
	if connected_glasses and connected_glasses != glasses_id:
		return
	match  event_num:
		GlassesEvent.E_AVAILABLE:
			print("Trying to connect to glasses ", glasses_id)
			connected_glasses = glasses_id 
			TiltFiveManager.connect_glasses(glasses_id, "Godot Tilt Five Demo")
		GlassesEvent.E_UNAVAILABLE:
			connected_glasses = null
		GlassesEvent.E_CONNECTED:
			print("Glasses ", connected_glasses, " connected")
			arvr = true
		GlassesEvent.E_DISCONNECTED:
			connected_glasses = null
		GlassesEvent.E_TRACKING:
			print("Started tracking glasses ",glasses_id)
		GlassesEvent.E_NOT_TRACKING:
			print("Stopped tracking glasses ",glasses_id)
	
		
