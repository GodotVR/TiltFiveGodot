extends MeshInstance


var box_color : Color = Color(0.5, 0.5, 0.5, 1.0)

func _ready():
	var color_mat = SpatialMaterial.new() 
	color_mat.albedo_color = box_color #Set color of new material
	material_override = color_mat
