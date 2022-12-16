#include "GodotT5Service.h"
#include <assert.h>
#include <Logging.h>
#include <Quat.hpp>
#include <VisualServer.hpp>   
#include <Defs.hpp>

using godot::Quat;
using godot::Image;
using godot::Texture;
using godot::VisualServer;

using T5Integration::Glasses;
using T5Integration::WandButtons;

namespace WandState = T5Integration::WandState;

namespace GodotT5Integration {

GodotT5ObjectRegistry g_godot_t5_services;

GodotT5Service::GodotT5Service()
	: T5Service()
{}

bool GodotT5Service::should_glasses_be_connected(Glasses::Ptr glasses) {
    return !_active_glasses;
}

void GodotT5Service::glasses_were_connected(Glasses::Ptr glasses) {
    if(!_active_glasses)
    {
        _active_glasses = glasses;
        create_textures(_active_glasses);
    }
}

void GodotT5Service::glasses_were_disconnected(Glasses::Ptr glasses) {
    if(_active_glasses == glasses)
    {
        _active_glasses.reset();
        release_textures(_active_glasses);
    }
}

bool GodotT5Service::is_connected()
{
	return _active_glasses && _active_glasses->is_connected();
}

bool GodotT5Service::is_tracking()
{
	return is_connected() && _active_glasses->is_tracking();
}

Vector2 GodotT5Service::get_display_size() {
	if (!_active_glasses) return Vector2(1216, 768);
    int width, height;
    _active_glasses->get_display_size(width, height);
    return Vector2(width, height);
}

float GodotT5Service::get_fov() {
	if (!_active_glasses) return 38.0f;
    return _active_glasses->get_fov();
}


Vector3 GodotT5Service::get_eye_position(Glasses::Eye eye)
{
    assert(_active_glasses);
	float dir = (eye == Glasses::Left ? -1.0f : 1.0f);
	auto ipd = _active_glasses->get_ipd();
	return Vector3(dir * ipd / 2.0f, 0, 0);
}

Transform GodotT5Service::get_eye_transform(Glasses::Eye eye) 
{
	if (!_active_glasses) return Transform();

	Transform eye_pose;
    eye_pose.set_origin(get_eye_position(eye));
	
	Quat orientation;
	Vector3 position;
	_active_glasses->get_glasses_orientation(orientation.x, orientation.y, orientation.z, orientation.w);
	_active_glasses->get_glasses_position(position.x, position.y, position.z);

	Transform headPose;
    headPose.set_origin(position);
	headPose.set_basis(orientation.inverse());
    headPose.rotate(Vector3::RIGHT, -Math_PI / 2.0f);

	return eye_pose * headPose;
}

size_t GodotT5Service::get_num_wands() 
{
	return _active_glasses ? _active_glasses->get_num_wands() : 0;
}


bool GodotT5Service::is_wand_pose_valid(size_t wand_num) 
{
	return _active_glasses ? _active_glasses->is_wand_pose_valid(wand_num) : false;
}

Transform GodotT5Service::get_wand_transform(size_t wand_num) {
	Quat orientation;
	Vector3 position;
	_active_glasses->get_wand_position(wand_num, position.x, position.y, position.z);
	_active_glasses->get_wand_orientation(wand_num, orientation.x, orientation.y, orientation.z, orientation.w);

	Transform wandPose;
    wandPose.set_origin(position);
	wandPose.set_basis(orientation.inverse());
    wandPose.rotate(Vector3::RIGHT, -Math_PI / 2.0f);

	return wandPose;
}

void GodotT5Service::connection_updated()
{
}

void GodotT5Service::tracking_updated()
{
	if (!_active_glasses) return;

    auto num_wands = _active_glasses->get_num_wands();
    while(_wand_controller_id.size() < num_wands) {
        auto new_idx = _wand_controller_id.size();
        _wand_controller_id.push_back(-1);
        _wand_name.push_back(_active_glasses->get_id() + ":" + std::to_string(new_idx) + '\0');
    }

    for(int wand_idx = 0; wand_idx < num_wands; ++wand_idx) {
        update_wand(wand_idx);
    }
}

void GodotT5Service::update_wand(size_t wand_idx) {
    auto controller_id = _wand_controller_id[wand_idx];
    int hand = wand_idx < 2 ? wand_idx + 1 : 0;
    if(_active_glasses->is_wand_state_changed(wand_idx, WandState::CONNECTED)) {
        if(_active_glasses->is_wand_state_set(wand_idx, WandState::CONNECTED)) {
            controller_id = godot::arvr_api->godot_arvr_add_controller(&(_wand_name[wand_idx][0]), hand, true, true);
            _wand_controller_id[wand_idx] = controller_id;
        }
        else if(controller_id > 0) {
            godot::arvr_api->godot_arvr_remove_controller(controller_id);
            return;
        }
    }
    if(_active_glasses->is_wand_state_set(wand_idx, WandState::POSE_VALID)) {
        auto wand_transform = as_c_struct(get_wand_transform(wand_idx));
        godot::arvr_api->godot_arvr_set_controller_transform(controller_id, &wand_transform, true, true);
    }
    if(_active_glasses->is_wand_state_set(wand_idx, WandState::ANALOG_VALID)) {
        float trigger_value;
        _active_glasses->get_wand_trigger(wand_idx, trigger_value);
        godot::arvr_api->godot_arvr_set_controller_axis(controller_id, WAND_ANALOG_TRIGGER, trigger_value, true);
        Vector2 stick;
        _active_glasses->get_wand_stick(wand_idx, stick.x, stick.y);
        godot::arvr_api->godot_arvr_set_controller_axis(controller_id, WAND_ANALOG_X, stick.x, true);
        godot::arvr_api->godot_arvr_set_controller_axis(controller_id, WAND_ANALOG_Y, stick.y, true);
    }
    if(_active_glasses->is_wand_state_set(wand_idx, WandState::BUTTONS_VALID)) {
        WandButtons buttons;
        _active_glasses->get_wand_buttons(wand_idx, buttons);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_A,     buttons.a);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_B,	    buttons.b);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_X,	    buttons.x);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_Y,	    buttons.y);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_1,	    buttons.one);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_2,	    buttons.two);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_STICK, buttons.three);
        godot::arvr_api->godot_arvr_set_controller_button(controller_id, WAND_BUTTON_T5,	buttons.t5);
    }
}

void GodotT5Service::send_frame()
{
	if (!_active_glasses) return;

	_active_glasses->send_frame(get_eye_texture(Glasses::Eye::Left), get_eye_texture(Glasses::Eye::Right));
}

intptr_t GodotT5Service::get_eye_texture(Glasses::Eye eye) {
    return eye == Glasses::Eye::Left ?
        VisualServer::get_singleton()->texture_get_texid(_left_eye_texture->get_rid()) :
        VisualServer::get_singleton()->texture_get_texid(_right_eye_texture->get_rid());
}

void GodotT5Service::create_textures(Glasses::Ptr glasses) {
    int width;
    int height;
    glasses->get_display_size(width, height);

    Ref<Image> image = Image::_new();
    godot::Color bg(0,0,0,1);
    image->create(width, height, false, godot::Image::FORMAT_RGBA8);
    image->lock();
    for(int y = 0; y < height; ++y) 
    {
        for(int x = 0; x < width; ++x) 
        {
            image->set_pixel(x,y,bg);
        }
    }
    image->unlock();

    _left_eye_texture = Ref<ImageTexture>(ImageTexture::_new());
    _left_eye_texture->create_from_image(image, Texture::FLAG_MIRRORED_REPEAT);

    _right_eye_texture = Ref<ImageTexture>(ImageTexture::_new());
    _right_eye_texture->create_from_image(image, Texture::FLAG_MIRRORED_REPEAT);
}

void GodotT5Service::release_textures(Glasses::Ptr glasses) {
    _left_eye_texture.unref();
    _right_eye_texture.unref();
}

void GodotT5Math::rotate_vector(float quat_x, float quat_y, float quat_z, float quat_w, float& vec_x, float& vec_y, float& vec_z)  {

    godot::Quat orient(quat_x, quat_y, quat_z, quat_w);
    godot::Vector3 vec(vec_x, vec_y, vec_z);

	vec = orient.xform(vec);

	vec_x = vec.x;
	vec_y = vec.y;
	vec_z = vec.z;
}

void GodotT5Logger::log_error(const char* message, const char* func_name, const char* file_name, int line_num) { 
    godot::Godot::print_error(message, func_name, file_name, line_num);
}

void GodotT5Logger::log_warning(const char* message, const char* func_name, const char* file_name, int line_num) {
    godot::Godot::print_warning(message, func_name, file_name, line_num);
}

void GodotT5Logger::log_string(const char* message) {
    godot::Godot::print(message);
}

GodotT5Service::Ptr GodotT5ObjectRegistry::service() {

		assert(_instance);
		return std::static_pointer_cast<GodotT5Service>(ObjectRegistry::service());    
}


T5Integration::T5Service::Ptr GodotT5ObjectRegistry::get_service() {
	if (!_service)
		_service = std::make_shared<GodotT5Service>();
	return _service;
}

T5Integration::T5Math::Ptr GodotT5ObjectRegistry::get_math() {
	if (!_math)
		_math = std::make_shared<GodotT5Math>();
	return _math;
}

T5Integration::Logger::Ptr GodotT5ObjectRegistry::get_logger() {
	if (!_logger)
		_logger = std::make_shared<GodotT5Logger>();
	return _logger;
}

}