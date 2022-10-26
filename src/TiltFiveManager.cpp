#include "TiltFiveManager.h"     
#include <thread>   
#include <VisualServer.hpp>   
#include <Defs.hpp>

#define GLASSES_BUFFER_SIZE    1024

float const defaultFOV = 48.0f;

using godot::VisualServer;
using godot::Image;
using godot::Ref;

void HackMakeCurrent() ;


Glasses::Glasses(const std::string& id) 
: mId(id), frameResult(-1)
{

}

Glasses::~Glasses()
{   
    Release();
    Destroy();
}

bool Glasses::Create(T5_Context context) 
{
    auto result = t5CreateGlasses(context, mId.c_str(), &mGlasses);
    if(result != T5_SUCCESS) 
    {
        LogError("t5CreateGlasses", result);
        return false;
    }
    return true;
}

void Glasses::Destroy() 
{
    t5DestroyGlasses(&mGlasses);
    mGlasses = nullptr;
}

bool Glasses::Acquire() 
{
    if(mGlasses)
    {
        auto result = t5AcquireGlasses(mGlasses, "Godot Application");
        mIsAcquired = (result == T5_SUCCESS);
        if(!mIsAcquired) 
        {
            LogError("t5AcquireGlasses", result);
        }
    }
    
    QueryIPD();
    return mIsAcquired;
}

void Glasses::Release() 
{
    if(mGlasses && mIsAcquired)
    {
        auto result = t5ReleaseGlasses(mGlasses);
        if(result != T5_SUCCESS) 
        {
            LogError("t5ReleaseGlasses", result);
        }
    }
    mIsAcquired = false;

    auto vs = VisualServer::get_singleton();

    mLeftEyeTexture->free();
    mRightEyeTexture->free();
}


bool Glasses::EnsureReady() 
{
    if(mIsReady)
        return true;

    auto result = t5EnsureGlassesReady(mGlasses);
    mIsReady = (result == T5_SUCCESS);
    if(!mIsReady && result != T5_ERROR_TRY_AGAIN) 
    {
        LogError("t5EnsureGlassesReady", result);
        return false;
    }
    return true;
}

Vector2 Glasses::GetRenderTargetSize() 
{
    return Vector2(1216, 768);  
}

bool Glasses::InitializeGraphics() 
{
    if(mIsGraphicsInitialized)
        return true;

    auto result = t5InitGlassesGraphicsContext(mGlasses, kT5_GraphicsApi_Gl, nullptr);
    mIsGraphicsInitialized = (result == T5_SUCCESS);
    if(!mIsGraphicsInitialized) 
    {
        HackMakeCurrent();
        LogError("t5InitGlassesGraphicsContext", result);
        return false;
    }

    auto vs = VisualServer::get_singleton();
    auto texSize = GetRenderTargetSize();
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

    mLeftEyeTexture = Ref<ImageTexture>(ImageTexture::_new());
    mLeftEyeTexture->create_from_image(image, Texture::FLAG_MIRRORED_REPEAT);

    mRightEyeTexture = Ref<ImageTexture>(ImageTexture::_new());
    mRightEyeTexture->create_from_image(image, Texture::FLAG_MIRRORED_REPEAT);

    static bool once = true;
    if(once) 
    {
        auto ltid = VisualServer::get_singleton()->texture_get_texid(mLeftEyeTexture->get_rid());
        auto rtid = VisualServer::get_singleton()->texture_get_texid(mRightEyeTexture->get_rid());

        godot::Godot::print(String("Create L/R texture = ") + String::num_int64(ltid) + " " + String::num_int64(rtid));
        once = false;
    }

    return true;
}


RID Glasses::GetTextureForEye(Eye eye) 
{
    if(eye == Left) 
        return mLeftEyeTexture->get_rid();
    if(eye == Right)
        return mRightEyeTexture->get_rid();
    return RID();
}


bool Glasses::QueryIPD()
{
    auto result = t5GetGlassesFloatParam(mGlasses, 0, kT5_ParamGlasses_Float_IPD, &mIpd);
    if(result != T5_SUCCESS) 
    {
        LogError("t5GetGlassesFloatParam(kT5_ParamGlasses_Float_IPD)", result);
        return false;
    }
    return true;
}

ChangeDetector<bool> poseChange(true);

void Glasses::UpdatePose()
{
    auto result = t5GetGlassesPose(mGlasses, &mT5Pose);

    mIsTracking = (result == T5_SUCCESS);
    if(mIsTracking)
    {
        godot::Quat orientation(mT5Pose.rotToGLS_GBD.x, mT5Pose.rotToGLS_GBD.y, mT5Pose.rotToGLS_GBD.z, mT5Pose.rotToGLS_GBD.w);
        
        mHeadTransform.basis = godot::Basis(orientation.inverse());
        mHeadTransform.origin = godot::Vector3(mT5Pose.posGLS_GBD.x, mT5Pose.posGLS_GBD.y, mT5Pose.posGLS_GBD.z);

        mHeadTransform.rotate(Vector3::RIGHT, -Math_PI / 2.0f);
    }
    poseChange.SetValue(mIsTracking);
    if(poseChange.IsChanged()) {
        godot::Godot::print(mIsTracking ? "Tracking started" : "Tracking ended");
    }
}



const Transform Glasses::GetOriginToEyeTransform(Eye eye, float worldScale) 
{
    Transform eyeTransform;

    float dir = (eye == Glasses::Left ? -1.0f : 1.0f);
    eyeTransform.translate((real_t)(dir * mIpd/2.0 * worldScale),0,0);

    return eyeTransform * mHeadTransform.scaled(Vector3(worldScale, worldScale, worldScale));
}


const Transform Glasses::GetEyeToHeadTransform(Eye eye, float worldScale) 
{
    Transform eyeTransform;

    float dir = (eye == Glasses::Left ? -1.0f : 1.0f);
    eyeTransform.translate((real_t)(dir * mIpd/2.0 * worldScale),0,0);

    return eyeTransform;
}


CameraMatrix Glasses::GetProjectionForEye(Eye _, float aspect, float zNear, float zFar) 
{
    CameraMatrix result;
    result.set_perspective(defaultFOV, aspect, zNear, zFar);
    return result;
}

void Glasses::SendFrame() 
{
    
	if (mIsTracking)
	{
		T5_FrameInfo frameInfo;

        auto size = GetRenderTargetSize();

		frameInfo.vci.startY_VCI = -tan(godot::Math::deg2rad(defaultFOV) * 0.5f);
		frameInfo.vci.startX_VCI = frameInfo.vci.startY_VCI * size.x / size.y;
		frameInfo.vci.width_VCI = -2.0f * frameInfo.vci.startX_VCI;
		frameInfo.vci.height_VCI = -2.0f * frameInfo.vci.startY_VCI;

		frameInfo.texWidth_PIX = (int)size.x;
		frameInfo.texHeight_PIX = (int)size.y;

        static bool once = true;
        if(once) 
        {
            auto ltid = VisualServer::get_singleton()->texture_get_texid(mLeftEyeTexture->get_rid());
            auto rtid = VisualServer::get_singleton()->texture_get_texid(mRightEyeTexture->get_rid());

            godot::Godot::print(String("Send L/R texture = ") + String::num_int64(ltid) + " " + String::num_int64(rtid));
            once = false;
        }


		frameInfo.leftTexHandle = (void *)VisualServer::get_singleton()->texture_get_texid(mLeftEyeTexture->get_rid());
		frameInfo.rightTexHandle = (void *)VisualServer::get_singleton()->texture_get_texid(mRightEyeTexture->get_rid());  

        godot::Quat orientation(mT5Pose.rotToGLS_GBD.x, mT5Pose.rotToGLS_GBD.y, mT5Pose.rotToGLS_GBD.z, mT5Pose.rotToGLS_GBD.w);

        auto leftPos =  orientation.xform(Vector3((real_t)-mIpd/2.0f,0,0)) + Vector3(mT5Pose.posGLS_GBD.x, mT5Pose.posGLS_GBD.y, mT5Pose.posGLS_GBD.z);
        auto rightPos = orientation.xform(Vector3((real_t)mIpd/2.0f,0,0)) + Vector3(mT5Pose.posGLS_GBD.x, mT5Pose.posGLS_GBD.y, mT5Pose.posGLS_GBD.z);
        
		frameInfo.posLVC_GBD = { leftPos.x, leftPos.y, leftPos.z };
		frameInfo.rotToLVC_GBD = mT5Pose.rotToGLS_GBD;

		frameInfo.posRVC_GBD = { rightPos.x, rightPos.y, rightPos.z };
		frameInfo.rotToRVC_GBD = mT5Pose.rotToGLS_GBD;

		frameInfo.isUpsideDown = false;
		frameInfo.isSrgb = false;

		frameResult.SetValue(t5SendFrameToGlasses(mGlasses, &frameInfo));

		if (frameResult.IsChanged())
		{ 
            LogError("t5SendFrameToGlasses", frameResult.GetValue());
		}
	}
}

bool Glasses::Update() 
{
    if(!EnsureReady())
        return false;

    if(!InitializeGraphics())
        return false;

    UpdatePose();

    return true;
}



TiltFiveManager::TiltFiveManager() {}

TiltFiveManager::~TiltFiveManager() 
{
    Uninitialize();
}

bool TiltFiveManager::Initialize() 
{
    T5_ClientInfo clientInfo;
    clientInfo.applicationId = "Godot Application";
    clientInfo.applicationVersion = "0.0.1";

    auto result = t5CreateContext(&mContext, &clientInfo, nullptr);

    if(result != T5_SUCCESS)
        return false;

    if(!GetServiceVersion())
        return false;

    if(!GetGlassesList())
        return false;

    for(GlassesPtr glasses : mGlassesList) {
        glasses->Create(mContext);
    }

    for(GlassesPtr glasses : mGlassesList) {
        if(glasses->Acquire()) {
            activeGlasses = glasses;
            break;
        }
    }
    if(!activeGlasses)
        return false;

    isInitialized = true;

    return true;
}

void TiltFiveManager::Uninitialize() 
{
    for(GlassesPtr glasses : mGlassesList) 
    {
        glasses->Release();
        glasses->Destroy();
    }
    mGlassesList.clear();
    activeGlasses = nullptr;
    if(mContext)
        t5DestroyContext(&mContext);
    mContext = nullptr;
}

bool TiltFiveManager::GetServiceVersion()
{
	char versionBuffer[40];
	size_t bufferSize = sizeof(versionBuffer);
    T5_Result result = T5_ERROR_NO_SERVICE;
    int retryCount = 10;
    for(; 
        result == T5_ERROR_NO_SERVICE && retryCount > 0;
	    result = t5GetSystemUtf8Param(mContext, kT5_ParamSys_UTF8_Service_Version, versionBuffer, &bufferSize))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        --retryCount;
    }
    if(result == T5_SUCCESS) {
        mNDKVersion = versionBuffer;
        return true;
    }
    else
    {
        LogError("t5GetSystemUtf8Param", result);
    }

    return false;
}

bool TiltFiveManager::GetGlassesList()
{
    size_t bufferSize = GLASSES_BUFFER_SIZE;
	char glassesListBuffer[GLASSES_BUFFER_SIZE];
    T5_Result result = T5_ERROR_NO_SERVICE;
    int retryCount = 10;
    for(; 
        result == T5_ERROR_NO_SERVICE && retryCount > 0;
        result = t5ListGlasses(mContext, glassesListBuffer, &bufferSize))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        --retryCount;
    }
    if(result == T5_SUCCESS) 
    {
		const char* buffPtr = glassesListBuffer;
		for (;;)
		{
			// Get the length of the string and exit if we've read the
			// terminal string (Zero length)
			size_t remainingBuffLen = GLASSES_BUFFER_SIZE - (buffPtr - glassesListBuffer);
			size_t stringLength = strnlen(buffPtr, remainingBuffLen);
			if (stringLength == 0)
			{
				break;
			}
			else if (stringLength == remainingBuffLen)
			{
				//std::cerr << "Glasses list overflow" << std::endl;
			}

			mGlassesList.push_back(std::make_shared<Glasses>(buffPtr));

			// Advance through the returned values
			buffPtr += stringLength;
			if (buffPtr > (glassesListBuffer + GLASSES_BUFFER_SIZE))
			{
				break;
			}
		}
        return true;
    }
    else
    {
        LogError("t5ListGlasses", result);
    }
    return false;
}


bool TiltFiveManager::IsInitialized()
{
    return isInitialized;
}

void TiltFiveManager::Process()
{
    if(!activeGlasses)
        return;
    if(!activeGlasses->Update())
        return;
}

void LogError(std::string desc, T5_Result err) 
{
    desc = desc + " : " + t5GetResultMessage(err);
    godot::Godot::print(desc.c_str());
}