#include "TiltFiveManager.h"
#include "Logging.h"

using godot::register_method;
using godot::register_signal;

TiltFiveManager::TiltFiveManager() {
}

TiltFiveManager::~TiltFiveManager() {
}

void TiltFiveManager::_init() 
{
    _t5_interface = GodotT5ObjectRegistry::service();
}

bool TiltFiveManager::start_service(const String application_id, const String application_version)
{
    if(!_t5_interface) return false;

    this->application_id = application_id;
    this->application_version = application_version;

    return _t5_interface->start_service(application_id.ascii().get_data(), application_version.ascii().get_data());
}

void TiltFiveManager::_process(float delta) 
{
    if(!_t5_interface) return;

    _t5_interface->update_connection();

    auto& events = _t5_interface->get_events();
    for(int i = 0; i < events.size(); i++) 
    {
        emit_signal("glasses_event", events[i].glasses_num, (int)events[i].event);
    }
}


void TiltFiveManager::connect_glasses(int glasses_num, const String display_name) 
{
    if(!_t5_interface) return;

    _t5_interface->connect_glasses(glasses_num, display_name.ascii().get_data());
}

void TiltFiveManager::disconnect_glasses(int glasses_num)
{
    if(!_t5_interface) return;

    _t5_interface->disconnect_glasses(glasses_num);
}

void TiltFiveManager::_register_methods() {
    register_method("_process", &TiltFiveManager::_process);
    register_method<bool (TiltFiveManager::*) (const String, const String)>("start_service", &TiltFiveManager::start_service);
    register_method("connect_glasses", &TiltFiveManager::connect_glasses);
    register_method("disconnect_glasses", &TiltFiveManager::disconnect_glasses);

    register_signal<TiltFiveManager>((char *)"glasses_event", "glasses_num", GODOT_VARIANT_TYPE_INT, "event", GODOT_VARIANT_TYPE_INT);

}