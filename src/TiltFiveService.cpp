#include "TiltFiveService.h"     
#include "util.h"
#include "Logging.h"
#include <algorithm>  

#include <VisualServer.hpp>   
#include <Defs.hpp>

#define GLASSES_BUFFER_SIZE    1024

float const defaultFOV = 48.0f;

using godot::VisualServer;
using godot::Image;
using godot::Ref;

void HackMakeCurrent() ;


Glasses::Glasses(const std::string& id) 
: _id(id)
{
    godot::Godot::print("Current state = " + godot::String::num(_state.get_current()));

    _state.set_current(S_UNAVAILABLE);

    _poll_connection.set_rate(_poll_rate_for_connecting);
    _poll_connection.set_action( [this](){ monitor_connection(); });

}

Glasses::~Glasses()
{   
    destroy_handle();
}

bool Glasses::allocate_handle(T5_Context context) 
{
    auto result = t5CreateGlasses(context, _id.c_str(), &_glasses_handle);
    if(result != T5_SUCCESS) 
    {
        T5_ERR_PRINT(result);
        return false;
    }
    _state.set_current(S_CREATED);
    _state.clear(S_UNAVAILABLE);

    return true;
}

void Glasses::destroy_handle() 
{
    _state.clear_all();
    t5DestroyGlasses(&_glasses_handle);
    _glasses_handle = nullptr;
}

void Glasses::monitor_connection() 
{
   if(_state.is_requested(S_READY))
   {
        T5_ConnectionState connectionState;
        
        if(auto result = t5GetGlassesConnectionState(_glasses_handle, &connectionState);
            result != T5_SUCCESS) 
        {
            // Doesn't seem to be anything recoverable here
            _last_error = result;
            T5_ERR_PRINT(result);
            _state.reset(S_ERROR);
            return;
        }

        switch (connectionState) {
            case kT5_ConnectionState_NotExclusivelyConnected: {
                if(!reserve())
                {
                    _poll_connection.set_rate(_poll_rate_for_connecting);
                    return;
                }
            }
            // On success fall through to connecting
            case kT5_ConnectionState_ExclusiveReservation:
            case kT5_ConnectionState_Disconnected: {
                if(!make_ready())
                {
                    _poll_connection.set_rate(_poll_rate_for_connecting);
                    return;
                }
            }    
            // On success fall through to connected
            case kT5_ConnectionState_ExclusiveConnection: {
                _poll_connection.set_rate(_poll_rate_for_monitoring);
                _state.set_current(S_READY);
                _state.set_requested(S_GRAPHICS_INIT);
                break;
            }
        } 
    }
    if(_state.is_requested(S_GRAPHICS_INIT))
    {
        initialize_graphics();
    }
    if(_state.is_requested(S_IPD_SET)) 
    {
        query_ipd();
    }   
    if(_state.is_requested(S_NAME_SET)) 
    {
        query_friendly_name();
    }    
     
    if(_state.any_changed(S_READY | S_GRAPHICS_INIT))
    { 
        if(_state.is_current(S_READY | S_GRAPHICS_INIT)) 
        {
            _state.set_current(S_CONNECTED);
        }
        else 
        {
            _state.clear(S_CONNECTED);
        }
    }
}



void Glasses::connect(std::string application_name) 
{
    TRACE_FN
    _application_name = application_name;
    if(_glasses_handle)
    {
        _state.set_requested(S_READY | S_IPD_SET | S_NAME_SET);
        reserve();
    }
}

void Glasses::disconnect() 
{
    TRACE_FN
    if(_state.is_current(S_READY)) 
    {
        auto result = t5ReleaseGlasses(_glasses_handle);
        if(result != T5_SUCCESS) 
        {
            T5_ERR_PRINT(result);
        }
    }
    _state.clear(S_READY | S_GRAPHICS_INIT);

    auto vs = VisualServer::get_singleton();

    _left_eye_texture.unref();
    _right_eye_texture.unref();
}

bool Glasses::reserve() 
{   
    auto result = t5AcquireGlasses(_glasses_handle, _application_name.c_str());
    if(result != T5_SUCCESS) 
    {
        if(result == T5_ERROR_ALREADY_CONNECTED)
        {
            // Weird, log it and then go with it
            _last_error = result;
            T5_WARN_PRINT(result);
        }
        else if(result == T5_ERROR_UNAVAILABLE) 
        {
            // Some else has the glasses so stop 
            // trying to connect
            _state.reset(S_UNAVAILABLE);
        }
        else if(result == T5_ERROR_DEVICE_LOST) 
        {
            destroy_handle();
            _last_error = result;
            T5_ERR_PRINT(result);
        }
        else
        {
            _last_error = result;
            T5_ERR_PRINT(result);
            _state.reset(S_ERROR);
        }
        return false;
    }
    return true;
}

bool Glasses::make_ready() 
{
    auto result = t5EnsureGlassesReady(_glasses_handle);    
    if(result != T5_SUCCESS) 
    {
        if(result == T5_ERROR_UNAVAILABLE) 
        {
            // So technically the glasses should be acquired so
            // why is this error happening? 
            // Anyway some else has the glasses so so stop 
            // trying to connect.
            // Assumption: Glasses don't need to be released.
            T5_WARN_PRINT(result);
            _state.reset(S_UNAVAILABLE);
        }
        else if(result == T5_ERROR_DEVICE_LOST) 
        {
            destroy_handle();
            _last_error = result;
            T5_ERR_PRINT(result);
        }
        else if(result != T5_ERROR_TRY_AGAIN)
        {
            // Doesn't seem to be anything recoverable here
            _last_error = result;
            T5_ERR_PRINT(result);
            _state.reset(S_ERROR);
        }
        return false;
    }
    return true;
}

Vector2 Glasses::get_render_target_size() 
{
    return Vector2(1216, 768);  
}

bool Glasses::initialize_graphics() 
{
    TRACE_FN
    if(!_state.is_requested(S_GRAPHICS_INIT))
        return true;

    auto result = t5InitGlassesGraphicsContext(_glasses_handle, kT5_GraphicsApi_Gl, nullptr);
    bool is_graphics_initialized = (result == T5_SUCCESS);
    if(!is_graphics_initialized) 
    {
    
        HackMakeCurrent();
        T5_ERR_PRINT(result);
        return false;
    }

    auto vs = VisualServer::get_singleton();
    auto texSize = get_render_target_size();
    int width = (int)texSize.width;
    int height = (int)texSize.height;

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

    static bool once = true;
    if(once) 
    {
        auto ltid = VisualServer::get_singleton()->texture_get_texid(_left_eye_texture->get_rid());
        auto rtid = VisualServer::get_singleton()->texture_get_texid(_right_eye_texture->get_rid());

        godot::Godot::print(String("Create L/R texture = ") + String::num_int64(ltid) + " " + String::num_int64(rtid));
        once = false;
    }

    _state.set_current(S_GRAPHICS_INIT);

    return true;
}


RID Glasses::get_texture_for_eye(Eye eye) 
{
    if(eye == Left) 
        return _left_eye_texture->get_rid();
    if(eye == Right)
        return _right_eye_texture->get_rid();
    return RID();
}


void Glasses::query_ipd()
{
    if(!_state.is_requested(S_IPD_SET)) return;

    auto result = t5GetGlassesFloatParam(_glasses_handle, 0, kT5_ParamGlasses_Float_IPD, &mIpd);
    if(result != T5_SUCCESS) 
    {
        T5_ERR_PRINT(result);
    }
    else
    {
        _state.set_current(S_IPD_SET);
    }
    return;
}

void Glasses::query_friendly_name()
{
    if(!_state.is_requested(S_NAME_SET)) return;
    std::vector<char> buffer;
    buffer.resize(64);

    for (bool first_time = true; ; first_time = false) 
    {
        size_t bufferSize = buffer.size();
        auto result = t5GetGlassesUtf8Param(_glasses_handle, 0, kT5_ParamGlasses_UTF8_FriendlyName, buffer.data(), &bufferSize);
        if (result == T5_SUCCESS) 
            break;
        else if(result == T5_ERROR_NO_SERVICE)
            return;
        else if (result == T5_ERROR_OVERFLOW && first_time) 
        {
            buffer.resize(bufferSize);
            continue;
        } 

        T5_ERR_PRINT(result);
        _last_error = result;
        return;
    }
    _friendly_name.copy(buffer.data(), buffer.size());
    _state.set_current(S_NAME_SET);
}



void Glasses::update_pose()
{
    auto result = t5GetGlassesPose(_glasses_handle, &_last_T5_pose);

    bool isTracking = (result == T5_SUCCESS);

    if(isTracking)
    {
        godot::Quat orientation(_last_T5_pose.rotToGLS_GBD.x, _last_T5_pose.rotToGLS_GBD.y, _last_T5_pose.rotToGLS_GBD.z, _last_T5_pose.rotToGLS_GBD.w);
        
        _head_transform.basis = godot::Basis(orientation.inverse());
        _head_transform.origin = godot::Vector3(_last_T5_pose.posGLS_GBD.x, _last_T5_pose.posGLS_GBD.y, _last_T5_pose.posGLS_GBD.z);

        _head_transform.rotate(Vector3::RIGHT, -Math_PI / 2.0f);
        _state.set_current(S_TRACKING);
    }
    else 
    {
        _state.clear(S_TRACKING);

        if(result != T5_ERROR_TRY_AGAIN) 
        {
            _last_error = result;
            T5_ERR_PRINT(result);
            _state.reset(S_ERROR);
        }
    }
    LOG_TOGGLE(false, isTracking,  "Tracking started" , "Tracking ended");
}

const Transform Glasses::get_origin_to_eye_transform(Eye eye, float worldScale) 
{
    Transform eyeTransform;

    float dir = (eye == Glasses::Left ? -1.0f : 1.0f);
    eyeTransform.translate((real_t)(dir * mIpd/2.0 * worldScale),0,0);

    return eyeTransform * _head_transform.scaled(Vector3(worldScale, worldScale, worldScale));
}

CameraMatrix Glasses::get_projection_for_eye(Eye _, float aspect, float zNear, float zFar) 
{
    CameraMatrix result;
    result.set_perspective(defaultFOV, aspect, zNear, zFar);
    return result;
}

void Glasses::send_frame() 
{
	if (_state.is_current(S_TRACKING | S_CONNECTED))
	{
		T5_FrameInfo frameInfo;

        auto size = get_render_target_size();

		frameInfo.vci.startY_VCI = -tan(godot::Math::deg2rad(defaultFOV) * 0.5f);
		frameInfo.vci.startX_VCI = frameInfo.vci.startY_VCI * size.x / size.y;
		frameInfo.vci.width_VCI = -2.0f * frameInfo.vci.startX_VCI;
		frameInfo.vci.height_VCI = -2.0f * frameInfo.vci.startY_VCI;

		frameInfo.texWidth_PIX = (int)size.x;
		frameInfo.texHeight_PIX = (int)size.y;

        static bool once = true;
        if(once) 
        {
            auto ltid = VisualServer::get_singleton()->texture_get_texid(_left_eye_texture->get_rid());
            auto rtid = VisualServer::get_singleton()->texture_get_texid(_right_eye_texture->get_rid());

            once = false;
        }


		frameInfo.leftTexHandle = (void *)VisualServer::get_singleton()->texture_get_texid(_left_eye_texture->get_rid());
		frameInfo.rightTexHandle = (void *)VisualServer::get_singleton()->texture_get_texid(_right_eye_texture->get_rid());  

        godot::Quat orientation(_last_T5_pose.rotToGLS_GBD.x, _last_T5_pose.rotToGLS_GBD.y, _last_T5_pose.rotToGLS_GBD.z, _last_T5_pose.rotToGLS_GBD.w);

        auto leftPos =  orientation.xform(Vector3((real_t)-mIpd/2.0f,0,0)) + Vector3(_last_T5_pose.posGLS_GBD.x, _last_T5_pose.posGLS_GBD.y, _last_T5_pose.posGLS_GBD.z);
        auto rightPos = orientation.xform(Vector3((real_t)mIpd/2.0f,0,0)) + Vector3(_last_T5_pose.posGLS_GBD.x, _last_T5_pose.posGLS_GBD.y, _last_T5_pose.posGLS_GBD.z);
        
		frameInfo.posLVC_GBD = { leftPos.x, leftPos.y, leftPos.z };
		frameInfo.rotToLVC_GBD = _last_T5_pose.rotToGLS_GBD;

		frameInfo.posRVC_GBD = { rightPos.x, rightPos.y, rightPos.z };
		frameInfo.rotToRVC_GBD = _last_T5_pose.rotToGLS_GBD;

		frameInfo.isUpsideDown = false;
		frameInfo.isSrgb = false;

		T5_Result result = t5SendFrameToGlasses(_glasses_handle, &frameInfo);

        LOG_TOGGLE(false, result == T5_SUCCESS, "Started sending frames", "Stoped sending frames");
        if(result == T5_SUCCESS)
            return;
        T5_ERR_PRINT(result);
        _last_error = result;
        if(result == T5_ERROR_NOT_CONNECTED)
		{ 
            _state.set_requested(S_READY);
		}
        // not sure how we might get here
        else if(result == T5_ERROR_GFX_CONTEXT_INIT_FAIL || result == T5_ERROR_INVALID_GFX_CONTEXT) 
        {
            _state.set_requested(S_GRAPHICS_INIT);
        }
        else 
        {
            _state.reset(S_ERROR);
        }
	}
}

bool Glasses::update_connection() 
{
    _poll_connection.tick();

    return true;
}

bool Glasses::update_tracking() 
{
    update_pose();

    return true;
}


std::weak_ptr<TiltFiveService> TiltFiveService::instance;

std::shared_ptr<TiltFiveService> TiltFiveService::get_service() 
{
    std::shared_ptr<TiltFiveService> service;
    if(instance.expired()) 
    {
        service = std::make_shared<TiltFiveService>();
        instance = service;
    }
    else
    {
        service = instance.lock();
    }
    return service;
}


TiltFiveService::TiltFiveService() 
{
    TRACE_FN
    _poll_glasses.set_rate(_poll_rate_for_monitoring);
    _poll_glasses.set_action( [this]() { query_glasses_list(); } );
}

TiltFiveService::~TiltFiveService() 
{
    TRACE_FN
    stop_service();
}


bool TiltFiveService::start_service(const char* application_id, const char* application_version) 
{
    TRACE_FN
    if(_is_started) return true;

    T5_ClientInfo clientInfo;
    clientInfo.applicationId = application_id;
    clientInfo.applicationVersion = application_version;

    auto result = t5CreateContext(&_context, &clientInfo, nullptr);

    if(result != T5_SUCCESS)
    {
        CHECK_POINT_FN
        T5_ERR_PRINT(result);
        return false;
    }

    _is_started = true;
    return true;
}

void TiltFiveService::stop_service() 
{
    if(!_is_started) return;
    _is_started = false;
    
    for(GlassesPtr glasses : _glasses_list) 
    {
        glasses->disconnect();
        glasses->destroy_handle();
    }
    _glasses_list.clear();
    _active_glasses = nullptr;
    if(_context)
        t5DestroyContext(&_context);
    _context = nullptr;
}



void TiltFiveService::update_connection() 
{
    if(_ndk_version.empty())
        query_ndk_version();

    _poll_glasses.tick();

    for(int i = 0; i < _glasses_list.size(); ++i) 
    {
        _glasses_list[i]->update_connection();
    }
}

const std::vector<GlassesEvent> TiltFiveService::get_events() 
{
    _events.clear();
    for(int i = 0; i < _glasses_list.size(); ++i) 
    {
        auto changes = _glasses_list[i]->get_changed_state();
        auto current_state = _glasses_list[i]->get_current_state();
        if((changes & Glasses::S_CREATED) == Glasses::S_CREATED) 
        {
            _events.push_back(GlassesEvent(i, current_state & Glasses::S_CREATED ? GlassesEvent::E_ADDED : GlassesEvent::E_LOST));
        }
        if((changes & Glasses::S_UNAVAILABLE) == Glasses::S_UNAVAILABLE) 
        {
            _events.push_back(GlassesEvent(i, current_state & Glasses::S_UNAVAILABLE ? GlassesEvent::E_UNAVAILABLE : GlassesEvent::E_AVAILABLE));
        }
        if((changes & Glasses::S_CONNECTED) == Glasses::S_CONNECTED) 
        {
            if(current_state & Glasses::S_CONNECTED) 
            {
                _active_glasses = _glasses_list[i];
                _events.push_back(GlassesEvent(i, GlassesEvent::E_CONNECTED));
            }
            else
            {
                _active_glasses.reset();
                _events.push_back(GlassesEvent(i, GlassesEvent::E_DISCONNECTED));

            }
        }
        if((changes & Glasses::S_TRACKING) == Glasses::S_TRACKING) 
        {
            _events.push_back(GlassesEvent(i, current_state & Glasses::S_TRACKING ? GlassesEvent::E_TRACKING : GlassesEvent::E_NOT_TRACKING));
        }
        if((changes & Glasses::S_ERROR) == Glasses::S_ERROR) 
        {
            // There is currently no way to recover from the error state
            _events.push_back(GlassesEvent(i, current_state & Glasses::S_ERROR ? GlassesEvent::E_STOPPED_ON_ERROR : GlassesEvent::E_AVAILABLE));
        }
        
    }
    return _events;

}

void TiltFiveService::query_ndk_version()
{
	char versionBuffer[32];
	size_t bufferSize = sizeof(versionBuffer);
    T5_Result result = t5GetSystemUtf8Param(_context, kT5_ParamSys_UTF8_Service_Version, versionBuffer, &bufferSize);

    if(result == T5_SUCCESS) {
        _ndk_version = versionBuffer;
    }
    else if(result != T5_ERROR_NO_SERVICE)
    {
        _ndk_version = "unknown";
        T5_ERR_PRINT(result);
    }
}

void TiltFiveService::query_glasses_list()
{
    std::vector<char> buffer;
    buffer.resize(64);

    for (bool first_time = true; ; first_time = false) 
    {
        size_t bufferSize = buffer.size();
        T5_Result result = t5ListGlasses(_context, buffer.data(), &bufferSize);
        if (result == T5_SUCCESS) 
            break;
        else if(result == T5_ERROR_NO_SERVICE)
            return;
        else if (result == T5_ERROR_OVERFLOW && first_time) 
        {
            buffer.resize(bufferSize);
            continue;
        } 

        T5_ERR_PRINT(result);
        _last_error = result;
        return;
    }

    std::string_view str_view(buffer.data(), buffer.size());
    std::vector<std::string> parsedIdList;

	while (!str_view.empty())
	{
        auto pos = str_view.find_first_of('\0');
        if(pos == 0 || pos == std::string_view::npos)
            break;
        parsedIdList.emplace_back(str_view.substr(0,pos));
        str_view.remove_prefix(pos);
    }
    for(auto& id : parsedIdList) 
    {
        auto found = std::find_if(
            _glasses_list.cbegin(), 
            _glasses_list.cend(), 
            [id](auto& gls) {return gls->is_id(id); });

        if(found == _glasses_list.cend()) 
        {   
            auto new_glasses = std::make_shared<Glasses>(id);
            if(new_glasses->allocate_handle(_context))
                _glasses_list.push_back(new_glasses);
        }
    }
}


bool TiltFiveService::is_service_started()
{
    return _is_started;
}

void TiltFiveService::connect_glasses(int glasses_num, std::string display_name) 
{
    if(glasses_num >= 0 && _glasses_list.size()) 
    {
        _glasses_list[glasses_num]->connect(display_name);
    }
}

void TiltFiveService::disconnect_glasses(int glasses_num)
{
    if(glasses_num >= 0 && _glasses_list.size()) 
    {
        _glasses_list[glasses_num]->disconnect();
    } 
}


bool TiltFiveService::begin_display()
{
    TRACE_FN
    _is_displaying = _is_started && _active_glasses;
    LOG_NUM(_is_displaying)
    return _is_displaying;
}

void TiltFiveService::end_display()
{
    TRACE_FN
    _is_displaying = false;
    _active_glasses->disconnect();

}

bool TiltFiveService::is_displaying()
{
    return _is_displaying;
}

void TiltFiveService::update_tracking()
{
    if(!_is_displaying || !_active_glasses)
        return;
    _active_glasses->update_tracking();
}
