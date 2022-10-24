#include <OS.hpp>
#include <windows.h>
#include <wingdi.h>


void HackMakeCurrent() 
{
    HDC hDC = (HDC)godot::OS::get_singleton()->get_native_handle(godot::OS::WINDOW_VIEW);
    HGLRC hRC = (HGLRC)godot::OS::get_singleton()->get_native_handle(godot::OS::OPENGL_CONTEXT);

    wglMakeCurrent(hDC, hRC);
}