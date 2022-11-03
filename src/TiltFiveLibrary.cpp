#include <Godot.hpp>
#include "ARVRInterface.h"
#include "TiltFiveManager.h"

namespace GD = godot;

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
    GD::Godot::gdnative_init(o);
    
    GD::Godot::print("godot_gdnative_init");
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
    GD::Godot::print("godot_gdnative_terminate");
    GD::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_gdnative_singleton() {
    GD::Godot::print("godot_gdnative_singleton");
	if (GD::arvr_api != NULL) {
		GD::arvr_api->godot_arvr_register_interface(&interface_struct);
	}
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
    GD::Godot::nativescript_init(handle);
    GD::Godot::print("godot_nativescript_init");

    GD::register_class<TiltFiveManager>();
}
