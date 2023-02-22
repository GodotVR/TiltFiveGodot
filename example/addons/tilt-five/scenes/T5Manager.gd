extends Node 

signal glasses_available
signal glasses_reserved(success)
signal glasses_dropped

enum GlassesEvent {
	E_ADDED         = 1,
	E_LOST          = 2,
	E_AVAILABLE     = 3,
	E_UNAVAILABLE   = 4,
	E_RESERVED      = 5,
	E_DROPPED  		= 6,
	E_TRACKING      = 7,
	E_NOT_TRACKING  = 8,
	E_STOPPED_ON_ERROR = 9
}

const T5Manager = preload("res://addons/tilt-five/tilt_five_manager.gdns")
var tilt_five_manager: T5Manager

export (String) var application_id = "my.game.com"
export (String) var application_version = "0.1.0"
export (String) var default_display_name = "Game: Player One"

var reserved_glasses: Dictionary

# Called when the node enters the scene tree for the first time.
func _ready():
	tilt_five_manager = T5Manager.new()
	add_child(tilt_five_manager)
	tilt_five_manager.connect("glasses_event", self, "on_glasses_event")
	
func start_service() -> bool:
	return tilt_five_manager.start_service(application_id, application_version)

func has_reserved_glasses() -> bool:
	for glasses in reserved_glasses:
		if reserved_glasses[glasses]:
			return true
	return false

func reserve_glasses(display_name := "") -> void:
	if has_reserved_glasses():
		print("Warning: Tilt Five glasses already reserved")
		return
	if display_name.length() == 0:
		display_name = default_display_name
	for try_glasses_id in reserved_glasses:
		tilt_five_manager.connect_glasses(try_glasses_id, display_name)
		while true:
			var result = yield(tilt_five_manager, "glasses_event")
			if result[0] != try_glasses_id: 
				continue
			elif result[1] == GlassesEvent.E_RESERVED:
				reserved_glasses[try_glasses_id] = true
				emit_signal("glasses_reserved", true)
				return
			else: 
				break
		emit_signal("glasses_reserved", false)
			
	
func on_glasses_event(glasses_id, event_num):
	print(glasses_id, " ", event_num)
	match  event_num:
		GlassesEvent.E_AVAILABLE:
			if not reserved_glasses.has(glasses_id):
				reserved_glasses[glasses_id] = false
			emit_signal("glasses_available")
		GlassesEvent.E_UNAVAILABLE:
			reserved_glasses.erase(glasses_id)
		GlassesEvent.E_DROPPED:
			if reserved_glasses.get(glasses_id, false):
				reserved_glasses[glasses_id] = false
				emit_signal("glasses_dropped")
	
		
