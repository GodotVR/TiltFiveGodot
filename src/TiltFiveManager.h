#include <Godot.hpp>
#include <Node.hpp>
#include <memory>
#include "GodotT5Service.h"

using godot::String;
using godot::Node;

using GodotT5Integration::GodotT5ObjectRegistry;
using GodotT5Integration::GodotT5Service;

class TiltFiveManager : public Node 
{
    GODOT_CLASS(TiltFiveManager, Node)

    std::shared_ptr<GodotT5Service> _t5_interface;

    std::vector<uint32_t> _changed_state;

    void start_service();


public:
    String application_id;
    String application_version;


    TiltFiveManager();
    ~TiltFiveManager();

    void _init(); // our initializer called by Godot
    void _process(float delta);

    bool start_service(const String application_id, const String application_version);

    void connect_glasses(int glasses_num, const String display_name);
    void disconnect_glasses(int glasses_num);

    static void _register_methods();
};

