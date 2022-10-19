#pragma once


class TiltFiveManager {

    public:
    TiltFiveManager();
    ~TiltFiveManager();

    bool Initialize();
    void Uninitialize();

    bool IsInitialized();

    void GetRenderTargetSize(int& width, int& height);

    void Process();

};

