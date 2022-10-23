#include <Godot.hpp>
#include "ARVRInterface.h"

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
    godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
    godot::Godot::gdnative_terminate(o);
}

void GDN_EXPORT godot_gdnative_singleton() {
	if (godot::arvr_api != NULL) {
		godot::arvr_api->godot_arvr_register_interface(&interface_struct);
	}
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
    godot::Godot::nativescript_init(handle);

}
