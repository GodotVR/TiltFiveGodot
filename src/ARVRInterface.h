////////////////////////////////////////////////////////////////////////////////////////////////
// Our main ARVRInterface code for our OpenVR GDNative module

#pragma once
#include <Godot.hpp>

class TiltFiveManager;

extern const godot_arvr_interface_gdnative interface_struct;

typedef struct arvr_data_struct {
	TiltFiveManager* manager;
} arvr_data_struct;  
