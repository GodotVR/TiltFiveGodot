#pragma once
////////////////////////////////////////////////////////////////////////////////////////////////
// Our main ARVRInterface code for our OpenVR GDNative module

#include <Godot.hpp>
#include <memory>

namespace GodotT5Integration {
class GodotT5Service;
}

extern const godot_arvr_interface_gdnative interface_struct;

typedef struct arvr_data_struct {
	std::shared_ptr<GodotT5Integration::GodotT5Service> service;
} arvr_data_struct;  
