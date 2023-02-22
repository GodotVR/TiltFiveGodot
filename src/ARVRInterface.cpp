////////////////////////////////////////////////////////////////////////////////////////////////
// ARVRInterface code for Tilt Five GDNative module
//
// Based on code written by Bastiaan "Mux213" Olij,

#include "ARVRInterface.h"
#include <Godot.hpp>
#include <OS.hpp>
#include <Transform.hpp>
#include <VisualServer.hpp>
#include <CameraMatrix.hpp>
#include "GodotT5Service.h"
#include "GodotStructCasts.h"

using godot::VisualServer;
using godot::Transform;
using godot::Vector2;
using godot::Vector3;

using GodotT5Integration::GodotT5Service;
using GodotT5Integration::GodotT5ObjectRegistry;
using Eye = GodotT5Integration::Glasses::Eye;

Vector2 g_zero_vector2;
Transform g_ident_transform;

////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers for getting our data from the void* the service calls give us
inline const std::shared_ptr<GodotT5Service> get_t5_service(const void *p_data)
{
	arvr_data_struct *arvr_data = (arvr_data_struct *)p_data;
	
	return arvr_data ? arvr_data->service : std::shared_ptr<GodotT5Service>();
}

////////////////////////////////////////////////////////////////
// Returns the name of this service
godot_string godot_arvr_get_name(const void *p_data) {
	godot_string ret;

	char name[] = "TiltFive";
	godot::api->godot_string_new(&ret);
	godot::api->godot_string_parse_utf8(&ret, name);

	return ret;
}

////////////////////////////////////////////////////////////////
// Returns capabilities for this service
godot_int godot_arvr_get_capabilities(const void *p_data) {
	godot_int ret;

	ret = 2 + 8; // 2 = ARVR_STEREO, 8 = ARVR_EXTERNAL

	return ret;
}

////////////////////////////////////////////////////////////////
// This is an AR feature not used here
godot_bool godot_arvr_get_anchor_detection_is_enabled(const void *p_data) {
	godot_bool ret;

	ret = false; // does not apply here

	return ret;
}

////////////////////////////////////////////////////////////////
// This is an AR feature not used here
void godot_arvr_set_anchor_detection_is_enabled(void *p_data, bool p_enable) {
	// we ignore this, not supported in this service!
}

////////////////////////////////////////////////////////////////
// Informs Godot stereoscopic rendering is required
godot_bool godot_arvr_is_stereo(const void *p_data) {
	godot_bool ret;

	ret = true;

	return ret;
}

////////////////////////////////////////////////////////////////
// Returns whether our service was successfully initialised
godot_bool godot_arvr_is_initialized(const void *p_data) {
	auto t5_service = get_t5_service(p_data);
	if (!t5_service) return false;
	
	return t5_service->is_connected();
}

////////////////////////////////////////////////////////////////
// Every thing should already be initialized from the
// TiltFiveManager. This function is just 
// returning true if there is an active set of glasses.
// NOTE: It probably doesn't matter if there are active
// glasses at this point. What does matter
// is that when the viewport.arvr get set to true
// godot_arvr_commit_for_eye is ready to go
godot_bool godot_arvr_initialize(void *p_data) {
	
	auto t5_service = get_t5_service(p_data);	
	if (!t5_service) return false;
	return t5_service->is_service_started();
}

////////////////////////////////////////////////////////////////
// Once again really doesn't do anything see above
void godot_arvr_uninitialize(void *p_data) 
{
}

////////////////////////////////////////////////////////////////
// Returns the requested size of our render target
// called right before rendering, if the size changes a new
// render target will be constructed.
godot_vector2 godot_arvr_get_render_targetsize(const void *p_data) {	
	auto t5_service = get_t5_service(p_data);	
	if(!t5_service->is_connected()) {
		LOG_ERROR_ONCE("Not connected")
		return as_c_struct(g_zero_vector2);
	}
	
	Vector2 result = t5_service->get_display_size();

	return as_c_struct(result);
}

////////////////////////////////////////////////////////////////
// This is called while rendering to get each eyes view matrix
godot_transform godot_arvr_get_transform_for_eye(void *p_data, godot_int p_eye, godot_transform *originTransform) {	
	auto t5_service = get_t5_service(p_data);
	if(!t5_service->is_connected()) {
		LOG_ERROR_ONCE("Not connected")
		return as_c_struct(g_ident_transform);
	}	

	godot_real world_scale = godot::arvr_api->godot_arvr_get_worldscale();

	Transform eye_transform = t5_service->get_eye_transform(p_eye == 1 ? Eye::Left : Eye::Right);
	eye_transform.scale(Vector3(world_scale, world_scale, world_scale));

	Transform ret = eye_transform * as_cpp_class(*originTransform);

	return as_c_struct(ret);
}

////////////////////////////////////////////////////////////////
// This is called while rendering to get each eyes projection
// matrix
void godot_arvr_fill_projection_for_eye(void *data, godot_real *projection, godot_int eye, godot_real aspect, godot_real z_near, godot_real z_far) {
	auto t5_service = get_t5_service(data);	
	if(!t5_service->is_connected()) {
		LOG_CHECK_POINT_ONCE
		LOG_ERROR_ONCE("Not connected")
		return;
	}	

    CameraMatrix cm;
    cm.set_perspective(t5_service->get_fov(), aspect, z_near, z_far);
	memcpy(projection, cm.matrix, sizeof(cm.matrix));
}

////////////////////////////////////////////////////////////////
// This is called after we render a frame for each eye so we
// can send output. 
void godot_arvr_commit_for_eye(void *data, godot_int eye, godot_rid *render_target, godot_rect2 *rect) {
	auto t5_service = get_t5_service(data);	
	if(!t5_service->is_connected()) {
		LOG_ERROR_ONCE("Not connected")
		return;
	}	

	godot::Rect2 screen_rect = *reinterpret_cast<godot::Rect2*>(rect);

	// This is code for mirroring the output. I've disabled the blitting.  I'm not sure it is 
	// needed in this situation.
	if (eye == 1 && !screen_rect.has_no_area()) {
		// blit as mono, attempt to keep our aspect ratio and center our render buffer
		auto render_size = as_cpp_class(godot_arvr_get_render_targetsize(data));

		float new_height = screen_rect.size.x * (render_size.y / render_size.x);
		if (new_height > screen_rect.size.y) {
			screen_rect.position.y = (0.5f * screen_rect.size.y) - (0.5f * new_height);
			screen_rect.size.y = new_height;
		} else {
			float new_width = screen_rect.size.y * (render_size.x / render_size.y);

			screen_rect.position.x = (0.5f * screen_rect.size.x) - (0.5f * new_width);
			screen_rect.size.x = new_width;
		}

		godot::arvr_api->godot_arvr_blit(0, render_target, (godot_rect2 *)&screen_rect);
	}
	// Tilt Five requires both left and right eye
	// data at once so the underlying functions just waits until 
	// both are finished to send.
	if (t5_service && eye == 2) {
		t5_service->send_frame();
	}
 
}

////////////////////////////////////////////////////////////////
// Process is called by the rendering thread right before we
// render our next frame. Here we obtain our new poses.
void godot_arvr_process(void *data) {
	auto t5_service = get_t5_service(data);	

	t5_service->update_tracking();
}


static bool g_is_interface_initialized = false;
////////////////////////////////////////////////////////////////
// Construct our service so it can be registered
// we do not initialise anything here!
void *godot_arvr_constructor(godot_object *p_instance) {
	arvr_data_struct *arvr_data = new(godot::api->godot_alloc(sizeof(arvr_data_struct))) arvr_data_struct();
	if(arvr_data) 
	{
		arvr_data->service = GodotT5ObjectRegistry::service();
		if(arvr_data->service)
			g_is_interface_initialized = true;
	}

	return arvr_data;
}

////////////////////////////////////////////////////////////////
// Clean up our service. 
// This seems to getting called more than once. It's a little
// disturbing
void godot_arvr_destructor(void *p_data) 
{
	if (p_data != NULL && g_is_interface_initialized) {
		arvr_data_struct *arvr_data = (arvr_data_struct *)p_data;
		arvr_data->~arvr_data_struct();
		godot::api->godot_free(p_data);
		g_is_interface_initialized = false;
	}
}

////////////////////////////////////////////////////////////////
// Return a texture ID for the eye if we manage the final
// output buffer.
int godot_arvr_get_external_texture_for_eye(void *data, int eye) {
	auto t5_service = get_t5_service(data);	
	if(!t5_service->is_connected()) {
		LOG_ERROR_ONCE("Not connected")
		return 0;
	}	

	if(t5_service && t5_service->is_connected()) 
	{
		return t5_service->get_eye_texture(eye == 1 ? Eye::Left : Eye::Right);
	}

	return 0;
}

////////////////////////////////////////////////////////////////
// Receive notifications sent to our ARVROrigin node.
void godot_arvr_notification(void *p_data, int p_what) {
	// nothing to do here for now but we should implement this.
}

////////////////////////////////////////////////////////////////
// Not using this
int godot_arvr_get_camera_feed_id(void *) {
	return 0;
}

////////////////////////////////////////////////////////////////
// Not using this
int godot_arvr_get_external_depth_for_eye(void *p_data, int p_eye) {
	return 0;
}

////////////////////////////////////////////////////////////////
// Structure to provide pointers to our service functions.
const godot_arvr_interface_gdnative interface_struct = {
	GODOTVR_API_MAJOR, GODOTVR_API_MINOR,
	godot_arvr_constructor,
	godot_arvr_destructor,
	godot_arvr_get_name,
	godot_arvr_get_capabilities,
	godot_arvr_get_anchor_detection_is_enabled,
	godot_arvr_set_anchor_detection_is_enabled,
	godot_arvr_is_stereo,
	godot_arvr_is_initialized,
	godot_arvr_initialize,
	godot_arvr_uninitialize,
	godot_arvr_get_render_targetsize,
	godot_arvr_get_transform_for_eye,
	godot_arvr_fill_projection_for_eye,
	godot_arvr_commit_for_eye,
	godot_arvr_process,
	// only available in Godot 3.2+
	godot_arvr_get_external_texture_for_eye,
	godot_arvr_notification,
	godot_arvr_get_camera_feed_id,
	// only available in Godot 3.3+
	godot_arvr_get_external_depth_for_eye
};
