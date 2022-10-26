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
#include "ChangeDetector.h"

using godot::Texture;
using godot::Transform;
using godot::RID;
using godot::Vector2;


class Glasses 
{
    public:

    enum Eye 
    {
        Mono,
        Left,
        Right
    };

    private:
    std::string mId;
    T5_Glasses mGlasses = nullptr;
    bool mIsAcquired = false;
    bool mIsReady = false;
    bool mIsGraphicsInitialized = false;
    bool mIsTracking = false;

    T5_GlassesPose mT5Pose;
    Transform mHeadTransform;
    double mIpd = 0.059f;

    Ref<ImageTexture> mLeftEyeTexture;
    Ref<ImageTexture> mRightEyeTexture;

    //RID mLeftEyeTexture;
    //RID mRightEyeTexture;

    ChangeDetector<T5_Result> frameResult;

    bool EnsureReady();
    bool InitializeGraphics();
    bool QueryIPD();

    void UpdatePose();

    public:

    Glasses(const std::string& id);
    ~Glasses();

    bool IsTracking() { return mIsTracking; }

    const Transform& GetHeadTransform() { return mHeadTransform; }
    const Transform GetEyeToHeadTransform(Eye eye, float worldScale);
    
    bool Create(T5_Context context);
    void Destroy();
    bool Acquire();
    void Release();

    Vector2 GetRenderTargetSize();
    RID GetTextureForEye(Eye eye);
    CameraMatrix GetProjectionForEye(Eye eye, float aspect, float zNear, float zFar);

    void SendFrame();

    bool IsReadyToDisplay() { return mIsReady && mIsGraphicsInitialized; }

    bool Update();
};

using GlassesPtr = std::shared_ptr<Glasses>;

class TiltFiveManager 
{    
    private:
    T5_Context mContext = nullptr;
    std::string mNDKVersion;
    std::vector<GlassesPtr> mGlassesList;
    bool isInitialized = false;

    bool GetServiceVersion();
    bool GetGlassesList();

    public:
    GlassesPtr activeGlasses;

    TiltFiveManager();
    ~TiltFiveManager();

    bool Initialize();
    void Uninitialize();

    bool IsInitialized();

    void Process();
};

void LogError(std::string msg, T5_Result err);