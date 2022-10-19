////////////////////////////////////////////////////////////////////////////////////////////////
// Our main ARVRInterface code for our OpenVR GDNative module

#ifndef TILT_FILE_ARVR_INTERFACE_H
#define TILT_FILE_ARVR_INTERFACE_H
#include <Godot.hpp>


extern const godot_arvr_interface_gdnative interface_struct;

typedef struct arvr_data_struct {
	uint32_t width;
	uint32_t height;

	int video_driver;
	int texture_id;
} arvr_data_struct;

#endif /* !TILT_FILE_ARVR_INTERFACE_H */
