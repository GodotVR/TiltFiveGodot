extends ARVROrigin



# Called when the node enters the scene tree for the first time.
func _ready():
	print("Root _ready")
	var arvr_interface = ARVRServer.find_interface("TiltFive")
	if arvr_interface and arvr_interface.initialize():
		var vp = get_viewport()
		vp.arvr = true
		vp.keep_3d_linear = true
		OS.vsync_enabled = false

