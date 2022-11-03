#include <Godot.hpp>
#include <Node.hpp>
#include <memory>
#include "TiltFiveService.h"

namespace GD = godot;

class TiltFiveManager : public GD::Node 
{
    GODOT_CLASS(TiltFiveManager, GD::Node)

    std::shared_ptr<TiltFiveService> _service;

    std::vector<uint32_t> _changed_state;

    void start_service();


public:
    GD::String application_id;
    GD::String application_version;


    TiltFiveManager();
    ~TiltFiveManager();

    void _init(); // our initializer called by Godot
    void _process(float delta);

    bool start_service(const GD::String application_id, const GD::String application_version);

    void connect_glasses(int glasses_num, const GD::String display_name);
    void disconnect_glasses(int glasses_num);

    static void _register_methods();
};

