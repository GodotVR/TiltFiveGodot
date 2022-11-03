#pragma once
#include "Godot.hpp"
#include "TiltFiveNative.h"

class DebugFunctionTrace 
{
    godot::String _func_name;
    public:
    DebugFunctionTrace(const char* func_name) 
    : _func_name(func_name)
    {
        godot::Godot::print(_func_name + ": Enter");
    }
    void check_point(int line_num) 
    {
        godot::Godot::print(_func_name + ": Line# " + godot::String::num(line_num));
    }
    ~DebugFunctionTrace() 
    {
        godot::Godot::print(_func_name + ": Exit");
    }
};

#ifdef _MSC_VER 
#define __PRETTY_FUNCTION__ __FUNCSIG__ 
#endif

#define TRACE_FN DebugFunctionTrace xyz_trace(__PRETTY_FUNCTION__);
#define CHECK_POINT_FN xyz_trace.check_point(__LINE__);

void log_tilt_five_error(T5_Result result_code, const char *p_function, const char *p_file, int p_line);
void log_tilt_five_warning(T5_Result result_code, const char *p_function, const char *p_file, int p_line);
void log_toggle(bool current, bool& state, const char* msg1, const char* msg2);

#ifndef LOG_TOGGLE
#define LOG_TOGGLE(INIT, TEST, MSG1, MSG2) { static bool toggle ## __LINE__ = (INIT);  log_toggle((TEST), toggle ## __LINE__, MSG1, MSG2); }
#endif

#ifndef T5_ERR_PRINT
#define T5_ERR_PRINT(result_code) log_tilt_five_error(result_code, __func__, __FILE__, __LINE__)
#endif

#ifndef T5_WARN_PRINT
#define T5_WARN_PRINT(result_code) log_tilt_five_warning(result_code, __func__, __FILE__, __LINE__)
#endif

#define LOG_NUM(VALUE) godot::Godot::print(godot::String(__func__) + ":" + godot::String::num(__LINE__) + " " + #VALUE + " = " + godot::String::num(VALUE));
