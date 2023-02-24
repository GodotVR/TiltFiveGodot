extends Spatial

func _ready():
	# Starts looking for available 
	# Tilt Five glasses to use
	# It will signal glasses_available
	# when they are found
	if $T5Manager.start_service():
		print("Service started")

func _on_T5Manager_glasses_available():
	# This will reserve the first pair of 
	# glasses it can for use. It will signal
	# glasses_reserved when done
	$T5Manager.reserve_glasses()

func _on_T5Manager_glasses_reserved(success):
	# The result of calling reserve_glasses()
	# If successful then the glasses may be 
	# rendered to by setting the viewport
	# arvr flag
	if success:
		get_viewport().arvr = true
