#pragma once
#include <TiltFiveNative.h>
#include <type_traits>
#include <chrono>
#include <functional>
#include "Logging.h"

template<typename T>
class StateFlags
{
	static_assert(std::is_integral_v<T>, "class StateFlags T must be a integral type");

	public:

	T _requested = 0;
	T _current = 0;
	T _previous = 0;

	T get_current()
	{
		return _current;
	}

	void set_requested(T state)
	{
		_current &= ~state;
		_requested |= state;
	}

	void set_current(T state)
	{
		_requested &= ~state;
		_current |= state;
	}

	void clear(T state)
	{
		_requested &= ~state;
		_current &= ~state;
	}

	void clear_all(bool clear_changes = true)
	{
		_current = 0;
		_requested = 0;
		if(clear_changes) 
			_previous = 0;
	}

    void clear_requested(T state) 
    {
        _requested &= ~state;
    }

    void clear_all_requested() 
    {
        _requested = 0;
    }

	void reset(T state, bool clear_changes = false)
	{
		_requested = 0;
		_current = state;
		if(clear_changes) 
		    _previous = state;
	}

	bool is_requested(T state) const
	{
		return (_requested & state) == state;
	}

	bool is_current(T state) const
	{
		return (_current & state) == state;
	}

	bool any_changed(T state) const
	{
		return ((_current ^ _previous) & state) != 0;
	}

	T get_changes() const
	{
		return (_current ^ _previous);
	}

	T get_changes_and_reset()
	{
		auto ret = get_changes();
		_previous = _current;
		return ret;
	}
};


class RepeatingActionTimer 
{
    std::chrono::time_point<std::chrono::steady_clock> _last_trigger_time;
    std::chrono::milliseconds _rate;
    std::function<void()> _action;

    public:

    template<typename F>
    void set_action(F&& action) 
    {
        _action = action;
    }

    void set_rate(std::chrono::milliseconds rate) 
    {
        _rate = rate;
    }

    void tick() 
    {
        auto current_time = std::chrono::steady_clock::now();
        if(_last_trigger_time + _rate <= current_time) 
        {
            _last_trigger_time = current_time;
            if(_action)
                _action();
        }
    }
};
