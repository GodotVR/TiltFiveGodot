extends Node2D

func on_window_size_change():
	$TextureRect.rect_size = OS.window_size

# Called when the node enters the scene tree for the first time.
func _ready():
	$TextureRect.texture = $Viewport.get_texture()

	get_tree().get_root().connect("size_changed", self, "on_window_size_change")
	on_window_size_change();
	
