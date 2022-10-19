#include "TiltFiveManager.h"

TiltFiveManager::TiltFiveManager() {}
TiltFiveManager::~TiltFiveManager() {}

bool TiltFiveManager::Initialize() 
{
    return true;
}

void TiltFiveManager::Uninitialize() 
{
}

bool TiltFiveManager::IsInitialized()
{
    return true;
}


void TiltFiveManager::GetRenderTargetSize(int& width, int& height)
{
    width = 1216;
    height = 768;
}

void TiltFiveManager::Process()
{

}