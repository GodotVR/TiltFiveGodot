#pragma once
#include <Godot.hpp>
#include <RID.hpp>
#include <Texture.hpp>
#include <CameraMatrix.hpp>
#include <ImageTexture.hpp>   
#include <Ref.hpp>   
#include <TiltFiveNative.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <atomic>
#include "util.h"
#include "ChangeDetector.h"

using namespace std::chrono_literals;

using godot::Texture;
using godot::Transform;
using godot::RID;
using godot::Vector2;

class TiltFiveService;

using GlassesFlags = StateFlags<uint32_t>;

class Glasses 
{
    friend TiltFiveService;

    public:

    enum States  {

        // Internal States
        S_READY           = 0x00000001,
        S_GRAPHICS_INIT   = 0x00000002,
        S_IPD_SET         = 0x00000004,
        S_NAME_SET        = 0x00000008,

        // Public states
        S_CREATED         = 0x00000100,
        S_UNAVAILABLE     = 0x00000200,
        S_TRACKING        = 0x00000400,
        S_CONNECTED       = 0x00000800,
        S_ERROR           = 0x00008000
    };

    enum Eye 
    {
        Mono,
        Left,
        Right
    };

    private:


    std::string _id;
    std::string _application_name;
    std::string _friendly_name;
    T5_Glasses _glasses_handle = nullptr;

    T5_GlassesPose _last_T5_pose;
    Transform _head_transform;
    double mIpd = 0.059f;

    Ref<ImageTexture> _left_eye_texture;
    Ref<ImageTexture> _right_eye_texture;

    GlassesFlags _state;

    std::chrono::milliseconds _poll_rate_for_connecting = 100ms;
    std::chrono::milliseconds _poll_rate_for_monitoring = 2s;
    RepeatingActionTimer _poll_connection;

    T5_Result _last_error;

    void monitor_connection();
    bool reserve();
    bool make_ready();
    bool initialize_graphics();
    void query_ipd();
    void query_friendly_name();

    void update_pose();

    public:

    Glasses(const std::string& id);
    ~Glasses();

    bool is_id(const std::string& id) { return _id == id; }
    bool is_tracking() { return _state.is_current(S_TRACKING); }
    bool is_active() { return _state.is_current(S_CONNECTED); }
    
    uint32_t get_current_state() { return _state.get_current(); }
    uint32_t get_changed_state() { return _state.get_changes_and_reset(); }

    const Transform get_origin_to_eye_transform(Eye eye, float worldScale);

    bool allocate_handle(T5_Context context);
    void destroy_handle();
    void connect(std::string application_name);
    void disconnect();

    Vector2 get_render_target_size();
    RID get_texture_for_eye(Eye eye);
    CameraMatrix get_projection_for_eye(Eye eye, float aspect, float zNear, float zFar);

    void send_frame();

    bool update_connection();
    bool update_tracking();
};

using GlassesPtr = std::shared_ptr<Glasses>;

struct GlassesEvent {
    enum EType
    {
        E_NONE          = 0,
        E_ADDED         = 1,
        E_LOST          = 2,
        E_AVAILABLE     = 3,
        E_UNAVAILABLE   = 4,
        E_CONNECTED     = 5,
        E_DISCONNECTED  = 6,
        E_TRACKING      = 7,
        E_NOT_TRACKING  = 8,
        E_STOPPED_ON_ERROR = 9
    };
    GlassesEvent(int num, EType evt) 
    : glasses_num(num), event(evt)
    {}

    int glasses_num;
    EType event;
};

class TiltFiveService 
{ 
    private:
    static std::weak_ptr<TiltFiveService> instance;


    bool _is_service_started = false;

    std::string _application_id;
    std::string _application_version;

    T5_Context _context = nullptr;
    std::string _ndk_version;
    std::vector<GlassesPtr> _glasses_list;
    GlassesPtr _active_glasses;

    bool _is_started = false;
    bool _is_displaying = false;

    std::chrono::milliseconds _poll_rate_for_monitoring = 2s;
    RepeatingActionTimer _poll_glasses;

    T5_Result _last_error;

    std::vector<GlassesEvent> _events;

    void query_ndk_version();
    void query_glasses_list();

    public:

    TiltFiveService();
    ~TiltFiveService();

    static std::shared_ptr<TiltFiveService> get_service();

    bool start_service(const char* applicationID, const char* applicationVersion);
    void stop_service();
    bool is_service_started();

    void connect_glasses(int glasses_num, std::string display_name);
    void disconnect_glasses(int glasses_num) ;

    size_t get_glasses_count() { return _glasses_list.size(); }
    GlassesPtr get_glasses(size_t idx) { return _glasses_list[idx]; }

    GlassesPtr get_active_glasses() { return _active_glasses; }

    bool begin_display();
    void end_display();
    bool is_displaying();

    const std::vector<GlassesEvent> get_events();

    void update_connection();
    void update_tracking();
};
