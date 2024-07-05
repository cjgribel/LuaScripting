#ifndef transform_hpp
#define transform_hpp
#pragma once

#include <sstream>

struct Transform
{
    // https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system#pointer-stability
    static constexpr auto in_place_delete = true;

    float x, y, rot;

    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << "{ x=" << std::to_string(x) << ", y=" << std::to_string(y) << " }";
        return ss.str();
    }
};

void register_transform(sol::state& lua)
{
    // clang-format off
    lua.new_usertype<Transform>("Transform",
        "type_id", &entt::type_hash<Transform>::value,

        sol::call_constructor,
        sol::factories([](float x, float y, float rot) {
            return Transform{
                .x = x, .y = y, .rot = rot
            };
            }),
        "x", &Transform::x,
        "y", &Transform::y,
        "rot", &Transform::rot,

        sol::meta_function::to_string, &Transform::to_string
    );
    // clang-format on
}

#endif