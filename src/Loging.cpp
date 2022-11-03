
#include "Logging.h"

namespace GD = godot;

void log_tilt_five_error(T5_Result result_code, const char *p_function, const char *p_file, int p_line) {
    GD::Godot::print_error(GD::String("(Tilt Five System)") + t5GetResultMessage(result_code), p_function, p_file, p_line);
}

void log_tilt_five_warning(T5_Result result_code, const char *p_function, const char *p_file, int p_line) {
    GD::Godot::print_warning(GD::String("(Tilt Five System)") + t5GetResultMessage(result_code), p_function, p_file, p_line);
}

void log_toggle(bool newValue, bool& prevVal, const char* msg1, const char* msg2) 
{
    if(newValue != prevVal) 
    {
        prevVal = newValue;
        if(newValue)
            GD::Godot::print(msg1);
        else
            GD::Godot::print(msg2);

    }
}