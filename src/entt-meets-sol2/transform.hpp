#ifndef transform_hpp
#define transform_hpp
#pragma once

#include <sstream>

#include "imgui.h" // If GUI code is here

#include "meta_literals.h" // for entt literals

#include "InspectorState.hpp" // for inspect()

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

/// inspect v3f
// bool inspect_Transform(Transform& t)
bool inspect_Transform(void* ptr, Editor::InspectorState& inspector)
{
    Transform* t = static_cast<Transform*>(ptr);
    bool mod = false;

    // TODO: use specializations for float

    inspector.begin_leaf("x");
    mod |= ImGui::InputFloat("", &t->x, 1.0f);
    inspector.end_leaf();

    inspector.begin_leaf("y");
    mod |= ImGui::InputFloat("", &t->y, 1.0f);
    inspector.end_leaf();

    inspector.begin_leaf("angle");
    mod |= ImGui::InputFloat("", &t->rot, 1.0f);
    inspector.end_leaf();

    return mod;
}

void register_transform(sol::state& lua)
{
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

    // Note: append meta sssigned to type by register_meta_component() in bond.hpp
    entt::meta<Transform>()
        .type("Transform"_hs).prop(display_name_hs, "Transform")
        .data<&Transform::x>("x"_hs).prop(display_name_hs, "x")
        .data<&Transform::y>("y"_hs).prop(display_name_hs, "y")
        .data<&Transform::rot>("rot"_hs).prop(display_name_hs, "angle")
        .func<&inspect_Transform>(inspect_hs) // USE ME
        //.func<&vec3_to_json>(to_json_hs)
        //.func < [](nlohmann::json& j, const void* ptr) { to_json(j, *static_cast<const vec3*>(ptr)); }, entt::as_void_t > (to_json_hs)
        //.func < [](const nlohmann::json& j, void* ptr) { from_json(j, *static_cast<vec3*>(ptr)); }, entt::as_void_t > (from_json_hs)
        //        .func<&vec3::to_string>(to_string_hs)
        //.func<&vec3_to_string>(to_string_hs)
        ;
}

#endif