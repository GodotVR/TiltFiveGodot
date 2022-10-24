extends Spatial



# Called when the node enters the scene tree for the first time.
func _ready():
	print("Root _ready")
	var arvr_interface = ARVRServer.find_interface("TiltFive")
	print(arvr_interface)
	if arvr_interface and arvr_interface.initialize():
		get_viewport().arvr = true


