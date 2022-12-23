#include <ARVRServer.hpp>
#include "TiltFiveManager.h"
#include "Logging.h"

using T5Integration::log_message;
using T5Integration::GlassesEvent;

using godot::register_method;
using godot::register_signal;
using godot::ARVRServer;

TiltFiveManager::TiltFiveManager() {
}

TiltFiveManager::~TiltFiveManager() {
    if(_t5_service)
        _t5_service->stop_service();
}

void TiltFiveManager::_init() 
{
    _t5_service = GodotT5ObjectRegistry::service();

    _arvr_interface = ARVRServer::get_singleton()->find_interface("TiltFive");
}

bool TiltFiveManager::start_service(const String application_id, const String application_version)
{
    if(!_t5_service || _arvr_interface.is_null()) return false;
    
    return _t5_service->start_service(application_id.ascii().get_data(), application_version.ascii().get_data());
}

void TiltFiveManager::_process(float delta) 
{
    if(!_t5_service) return;

    _t5_service->update_connection();

    auto& events = _t5_service->get_events();
    for(int i = 0; i < events.size(); i++) 
    {
        if(events[i].event == GlassesEvent::E_CONNECTED)
            ++_connected_count;
        else if(events[i].event == GlassesEvent::E_DISCONNECTED)
            --_connected_count;
        else if(events[i].event == GlassesEvent::E_ADDED)
            add_glasses(events[i].glasses_num);
        emit_signal("glasses_event", _glasses_ids[events[i].glasses_num].c_str(), (int)events[i].event);
    }
    if(_connected_count > 0 && !_arvr_interface_initialized) {
        _arvr_interface_initialized = _arvr_interface->initialize();
        log_message("T5 Interface initialized = ", _arvr_interface_initialized);
    }
    else if(_connected_count == 0 && _arvr_interface_initialized) {
        _arvr_interface->uninitialize();
        _arvr_interface_initialized = false;
        log_message("T5 Interface initialized = ", _arvr_interface_initialized);
    }
}

void TiltFiveManager::connect_glasses(const String glasses_id, const String display_name) 
{
    if(!_t5_service) return;

    int glasses_idx;
    if(try_find_glasses_idx(glasses_id, glasses_idx)) {
        _t5_service->connect_glasses(glasses_idx, display_name.ascii().get_data());
    }
    else {
        auto msg = "Unknown glasses id " + glasses_id;
        LOG_ERROR(msg.ascii().get_data());
    }
}

void TiltFiveManager::disconnect_glasses(const String glasses_id)
{
    if(!_t5_service) return;

    int glasses_idx;
    if(try_find_glasses_idx(glasses_id, glasses_idx)) {
        _t5_service->disconnect_glasses(glasses_idx);
    }
    else {
        auto msg = "Unknown glasses id " + glasses_id;
        LOG_ERROR(msg.ascii().get_data());
    }
}

void TiltFiveManager::add_glasses(int glasses_idx) {
    if(_glasses_ids.size() <= glasses_idx)
        _glasses_ids.resize(glasses_idx + 1);
    _glasses_ids[glasses_idx] = _t5_service->get_glasses_id(glasses_idx);
}

bool TiltFiveManager::try_find_glasses_idx(const String& glasses_id, int& out_glasses_idx) {
    for(out_glasses_idx = 0; out_glasses_idx < _glasses_ids.size(); ++out_glasses_idx)
    {
        if(glasses_id ==  _glasses_ids[out_glasses_idx].c_str())
            return true;
    }
    return false;
}

void TiltFiveManager::_register_methods() {
    register_method("_process", &TiltFiveManager::_process);
    register_method<bool (TiltFiveManager::*) (const String, const String)>("start_service", &TiltFiveManager::start_service);
    register_method("connect_glasses", &TiltFiveManager::connect_glasses);
    register_method("disconnect_glasses", &TiltFiveManager::disconnect_glasses);

    register_signal<TiltFiveManager>((char *)"glasses_event", "glasses_id", GODOT_VARIANT_TYPE_STRING, "event", GODOT_VARIANT_TYPE_INT);
}