////////////////////////////////////////////////////////////////////////////////////////////////
// Our main ARVRInterface code for our OpenVR GDNative module

#pragma once
#include <Godot.hpp>
#include <memory>

class TiltFiveService;

extern const godot_arvr_interface_gdnative interface_struct;

typedef struct arvr_data_struct {
	bool is_initialized = false;
	std::shared_ptr<TiltFiveService> service;
} arvr_data_struct;  
