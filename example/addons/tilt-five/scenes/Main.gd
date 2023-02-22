extends Spatial


# Called when the node enters the scene tree for the first time.
func _ready():
	if $T5Manager.start_service():
		print("Service started")


func _on_T5Manager_glasses_available():
	$T5Manager.reserve_glasses()

func _on_T5Manager_glasses_reserved(success):
	if success:
		get_viewport().arvr = true
