#include <OS.hpp>
#include <windows.h>
#include <wingdi.h>

// This is to get around a T5 initialization bug that should be fixed in the next version
void HackMakeCurrent() 
{
    HDC hDC = (HDC)godot::OS::get_singleton()->get_native_handle(godot::OS::WINDOW_VIEW);
    HGLRC hRC = (HGLRC)godot::OS::get_singleton()->get_native_handle(godot::OS::OPENGL_CONTEXT);

    wglMakeCurrent(hDC, hRC);
}