////////////////////////////////////////////////////////////////////////////////////////////////
// ARVRInterface code for Tilt Five GDNative module
//
// Based on code written by Bastiaan "Mux213" Olij,

#include <OS.hpp>
#include "ARVRInterface.h"
#include "VisualServer.hpp"
#include "TiltFiveService.h"
#include "Logging.h"

using godot::VisualServer;


////////////////////////////////////////////////////////////////////////////////////////////////
// Some functions to go back and forth between the Godot C and CPP types
inline Transform AsCpp(godot_transform& tran) {
	static_assert(sizeof(godot_transform) == sizeof(Transform));
	return *reinterpret_cast<Transform*>(&tran);
}

inline godot_transform AsC(Transform& tran) {
	static_assert(sizeof(godot_transform) == sizeof(Transform));
	return *reinterpret_cast<godot_transform*>(&tran);
}

inline Vector2 AsCpp(godot_vector2& vec) {
	static_assert(sizeof(godot_vector2) == sizeof(Vector2));
	return *reinterpret_cast<Vector2*>(&vec);
}

inline godot_vector2 AsC(Vector2& vec) {
	static_assert(sizeof(godot_vector2) == sizeof(Vector2));
	return *reinterpret_cast<godot_vector2*>(&vec);
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers for getting our data from the void* the interface calls give us
inline const std::shared_ptr<TiltFiveService> GetT5Service(const void *p_data)
{
	arvr_data_struct *arvr_data = (arvr_data_struct *)p_data;
	
	return arvr_data ? arvr_data->service : std::shared_ptr<TiltFiveService>();
}

inline GlassesPtr GetActiveT5Glasses(const void *p_data) {
	arvr_data_struct *arvr_data = (arvr_data_struct *)p_data;
	
	return arvr_data ? (arvr_data->service ? arvr_data->service->get_active_glasses() : GlassesPtr()) : GlassesPtr();
}

////////////////////////////////////////////////////////////////
// Returns the name of this interface
godot_string godot_arvr_get_name(const void *p_data) {
	godot_string ret;

	char name[] = "TiltFive";
	godot::api->godot_string_new(&ret);
	godot::api->godot_string_parse_utf8(&ret, name);

	return ret;
}

////////////////////////////////////////////////////////////////
// Returns capabilities for this interface
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
	// we ignore this, not supported in this interface!
}

////////////////////////////////////////////////////////////////
// Informs Godot stereoscopic rendering is required
godot_bool godot_arvr_is_stereo(const void *p_data) {
	godot_bool ret;

	ret = true;

	return ret;
}

////////////////////////////////////////////////////////////////
// Returns whether our interface was successfully initialised
godot_bool godot_arvr_is_initialized(const void *p_data) {
	auto t5_service = GetT5Service(p_data);
	if (!t5_service) return false;
	
	return t5_service->is_displaying();
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
	TRACE_FN
	auto t5_service = GetT5Service(p_data);	
	if (!t5_service) return false;

	return t5_service->begin_display();
}

////////////////////////////////////////////////////////////////
// Once again really doesn't do anything see above
void godot_arvr_uninitialize(void *p_data) 
{
	TRACE_FN
	auto t5_service = GetT5Service(p_data);	
	if (!t5_service) return;

	return t5_service->end_display();
}

////////////////////////////////////////////////////////////////
// Returns the requested size of our render target
// called right before rendering, if the size changes a new
// render target will be constructed.
godot_vector2 godot_arvr_get_render_targetsize(const void *p_data) {
	auto glasses = GetActiveT5Glasses(p_data); 
	
	Vector2 result(1216, 768);
	if(glasses)
		result = glasses->get_render_target_size();

	return AsC(result);
}

////////////////////////////////////////////////////////////////
// This is called while rendering to get each eyes view matrix
godot_transform godot_arvr_get_transform_for_eye(void *p_data, godot_int p_eye, godot_transform *originTransform) {
	auto glasses = GetActiveT5Glasses(p_data); 


	godot_real worldScale = godot::arvr_api->godot_arvr_get_worldscale();

	Transform eyeTransform;
 	if (glasses) {
		eyeTransform = glasses->get_origin_to_eye_transform( p_eye == 1 ? Glasses::Left : Glasses::Right, worldScale);
	} else {
		Transform referenceFrame = AsCpp(godot::arvr_api->godot_arvr_get_reference_frame());
		eyeTransform.translate((p_eye == 1 ? -0.035f : 0.035f) * worldScale, 0.0f, 0.0f);
		eyeTransform = eyeTransform * referenceFrame;
	};

	Transform ret = eyeTransform * AsCpp(*originTransform);

	return AsC(ret);
}

////////////////////////////////////////////////////////////////
// This is called while rendering to get each eyes projection
// matrix
void godot_arvr_fill_projection_for_eye(void *p_data, godot_real *p_projection, godot_int p_eye, godot_real p_aspect, godot_real p_z_near, godot_real p_z_far) {
	auto glasses = GetActiveT5Glasses(p_data); 

	CameraMatrix cm;

	if(glasses) {
		cm = glasses->get_projection_for_eye(p_eye == 1 ? Glasses::Left : Glasses::Right, p_aspect, p_z_near, p_z_far);
	}
	else {
		cm.set_perspective(48.0f, p_aspect, p_z_near, p_z_far);
	}	

	memcpy(p_projection, cm.matrix, sizeof(cm.matrix));
}

////////////////////////////////////////////////////////////////
// This is called after we render a frame for each eye so we
// can send output. 
void godot_arvr_commit_for_eye(void *p_data, godot_int p_eye, godot_rid *p_render_target, godot_rect2 *p_screen_rect) {
	auto glasses = GetActiveT5Glasses(p_data); 

	godot::Rect2 screen_rect = *(godot::Rect2 *)p_screen_rect;

	// This is code for mirroring the output. I've disabled the blitting.  I'm not sure it is 
	// needed in this situation.
	if (p_eye == 1 && !screen_rect.has_no_area()) {
		// blit as mono, attempt to keep our aspect ratio and center our render buffer
		godot_vector2 rs = godot_arvr_get_render_targetsize(p_data);
		godot::Vector2 *render_size = (godot::Vector2 *)&rs;

		float new_height = screen_rect.size.x * (render_size->y / render_size->x);
		if (new_height > screen_rect.size.y) {
			screen_rect.position.y = (0.5f * screen_rect.size.y) - (0.5f * new_height);
			screen_rect.size.y = new_height;
		} else {
			float new_width = screen_rect.size.y * (render_size->x / render_size->y);

			screen_rect.position.x = (0.5f * screen_rect.size.x) - (0.5f * new_width);
			screen_rect.size.x = new_width;
		}

		//godot::arvr_api->godot_arvr_blit(0, p_render_target, (godot_rect2 *)&screen_rect);
	}
	// Tilt Five requires both left and right eye
	// data at once so the underlying functions just waits until 
	// both are finished to send.
	if (glasses && p_eye == 2) {
		glasses->send_frame();
	}
 
}

////////////////////////////////////////////////////////////////
// Process is called by the rendering thread right before we
// render our next frame. Here we obtain our new poses.
void godot_arvr_process(void *p_data) {
	auto t5_service = GetT5Service(p_data);	

	t5_service->update_tracking();
}

////////////////////////////////////////////////////////////////
// Construct our interface so it can be registered
// we do not initialise anything here!
void *godot_arvr_constructor(godot_object *p_instance) {
	TRACE_FN

	arvr_data_struct *arvr_data = new(godot::api->godot_alloc(sizeof(arvr_data_struct))) arvr_data_struct();
	if(arvr_data) 
	{
		arvr_data->service = TiltFiveService::get_service();
		if(arvr_data->service)
			arvr_data->is_initialized = true;
	}

	return arvr_data;
}

////////////////////////////////////////////////////////////////
// Clean up our interface. 
// This seems to getting called more than once. It's a little
// disturbing
void godot_arvr_destructor(void *p_data) {
	TRACE_FN
	if (p_data != NULL) {
		arvr_data_struct *arvr_data = (arvr_data_struct *)p_data;
		if(!arvr_data->is_initialized)
			return;
		
		arvr_data->is_initialized = false;
		arvr_data->~arvr_data_struct();
		godot::api->godot_free(p_data);
	}
}

////////////////////////////////////////////////////////////////
// Return a texture ID for the eye if we manage the final
// output buffer.
int godot_arvr_get_external_texture_for_eye(void *p_data, int p_eye) {

	auto glasses = GetActiveT5Glasses(p_data);

	if(glasses && glasses->is_active()) 
	{
		return VisualServer::get_singleton()
			->texture_get_texid(
				glasses->get_texture_for_eye(p_eye == 1 ? Glasses::Left : Glasses::Right)
			);
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
// Structure to provide pointers to our interface functions.
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
