#include <Godot.hpp>
#include <Node.hpp>
#include <ARVRInterface.hpp>
#include <Ref.hpp>
#include <memory>
#include "GodotT5Service.h"

using godot::String;
using godot::Node;
using godot::Dictionary;
using godot::ARVRInterface;
using godot::Ref;

using GodotT5Integration::GodotT5ObjectRegistry;
using GodotT5Integration::GodotT5Service;

class TiltFiveManager : public Node 
{
    GODOT_CLASS(TiltFiveManager, Node)

public:
    TiltFiveManager();
    ~TiltFiveManager();

    void _init(); // our initializer called by Godot
    void _process(float delta);

    bool start_service(const String application_id, const String application_version);

    void connect_glasses(const String glasses_id, const String display_name);
    void disconnect_glasses(const String glasses_id);

    void set_sRBG_texture(const String glasses_id, const bool is_sRGB);
    void set_upside_down_texture(const String glasses_id, const bool is_upside_down);
    static void _register_methods();

private:
    void add_glasses(int glasses_idx);
    bool try_find_glasses_idx(const String& glasses_id, int& out_glasses_idx);

    std::shared_ptr<GodotT5Service> _t5_service;

    std::vector<std::string> _glasses_ids;

    bool _arvr_interface_initialized = false;
    Ref<ARVRInterface> _arvr_interface;
    int _connected_count = 0;
};

