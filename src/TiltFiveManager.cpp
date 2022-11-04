#include "TiltFiveManager.h"
#include "TiltFiveService.h"
#include "Logging.h"

TiltFiveManager::TiltFiveManager() {
}

TiltFiveManager::~TiltFiveManager() {
}

void TiltFiveManager::_init() 
{
    _service = TiltFiveService::get_service();
}

bool TiltFiveManager::start_service(const GD::String application_id, const GD::String application_version)
{
    

    if(!_service) return false;

    this->application_id = application_id;
    this->application_version = application_version;

    return _service->start_service(application_id.ascii().get_data(), application_version.ascii().get_data());
}

void TiltFiveManager::_process(float delta) 
{
    if(!_service) return;

    _service->update_connection();

    auto& events = _service->get_events();
    for(int i = 0; i < events.size(); i++) 
    {
        emit_signal("glasses_event", events[i].glasses_num, (int)events[i].event);
    }
}


void TiltFiveManager::connect_glasses(int glasses_num, const GD::String display_name) 
{
    if(!_service) return;

    _service->connect_glasses(glasses_num, display_name.ascii().get_data());
}

void TiltFiveManager::disconnect_glasses(int glasses_num)
{
    if(!_service) return;

    _service->disconnect_glasses(glasses_num);
}

void TiltFiveManager::_register_methods() {
    register_method("_process", &TiltFiveManager::_process);
    register_method<bool (TiltFiveManager::*) (const GD::String, const GD::String)>("start_service", &TiltFiveManager::start_service);
    register_method("connect_glasses", &TiltFiveManager::connect_glasses);
    register_method("disconnect_glasses", &TiltFiveManager::disconnect_glasses);
    register_property<TiltFiveManager, GD::String>("application_id", &TiltFiveManager::application_id, "com.example.test");
    register_property<TiltFiveManager, GD::String>("application_version", &TiltFiveManager::application_version, "0.1.0");

    register_signal<TiltFiveManager>((char *)"glasses_event", "glasses_num", GODOT_VARIANT_TYPE_INT, "event", GODOT_VARIANT_TYPE_INT);

}