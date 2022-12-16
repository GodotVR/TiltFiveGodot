#pragma once
#include <Godot.hpp>
#include <Transform.hpp>

using godot::Transform;
using godot::Vector2;
using godot::Vector3;

////////////////////////////////////////////////////////////////////////////////////////////////
// Some functions to go back and forth between the Godot C and CPP types
inline Transform as_cpp_class(const godot_transform& tran) {
	static_assert(sizeof(godot_transform) == sizeof(Transform));
	return *reinterpret_cast<const Transform*>(&tran);
}

inline godot_transform as_c_struct(const Transform& tran) {
	static_assert(sizeof(godot_transform) == sizeof(Transform));
	return *reinterpret_cast<const godot_transform*>(&tran);
}

inline Vector2 as_cpp_class(const godot_vector2& vec) {
	static_assert(sizeof(godot_vector2) == sizeof(Vector2));
	return *reinterpret_cast<const Vector2*>(&vec);
}

inline godot_vector2 as_c_struct(const Vector2& vec) {
	static_assert(sizeof(godot_vector2) == sizeof(Vector2));
	return *reinterpret_cast<const godot_vector2*>(&vec);
}
