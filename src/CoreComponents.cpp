
#include <entt/entt.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "imgui.h" // If GUI code is here

#include "meta_literals.h" // for entt literals

#include "CoreComponents.hpp"
#include "InspectorState.hpp" // for inspect()

//#include <iostream>

// ========== Transform

// + const (e.g. when used as key) ?
bool inspect_Transform(void* ptr, Editor::InspectorState& inspector)
{
    Transform* t = static_cast<Transform*>(ptr);
    bool mod = false;

    inspector.begin_leaf("x");
    mod |= Editor::inspect_type(t->x, inspector);
    inspector.end_leaf();

    inspector.begin_leaf("y");
    mod |= Editor::inspect_type(t->y, inspector);
    inspector.end_leaf();

    inspector.begin_leaf("angle");
    mod |= Editor::inspect_type(t->rot, inspector);
    inspector.end_leaf();

    return mod;
}

template<>
void register_meta<Transform>(sol::state& lua)
{
    std::cout << "register_meta<Transform>" << std::endl;

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

    // Note: appends meta asssigned to type by register_meta_component() in bond.hpp
    entt::meta<Transform>()
        .type("Transform"_hs).prop(display_name_hs, "Transform")

        .data<&Transform::x>("x"_hs).prop(display_name_hs, "x")
        .data<&Transform::y>("y"_hs).prop(display_name_hs, "y")
        .data<&Transform::rot>("rot"_hs).prop(display_name_hs, "angle")

        // .data<&Transform::x_global>("x_global"_hs).prop(display_name_hs, "x_global")
        // .data<&Transform::y_global>("y_global"_hs).prop(display_name_hs, "y_global")
        // .data<&Transform::rot_global>("rot_global"_hs).prop(display_name_hs, "angle_global")

        // .func<&inspect_Transform>(inspect_hs) // OPTIONAL

        //.func<&vec3_to_json>(to_json_hs)
        //.func < [](nlohmann::json& j, const void* ptr) { to_json(j, *static_cast<const vec3*>(ptr)); }, entt::as_void_t > (to_json_hs)
        //.func < [](const nlohmann::json& j, void* ptr) { from_json(j, *static_cast<vec3*>(ptr)); }, entt::as_void_t > (from_json_hs)
        //        .func<&vec3::to_string>(to_string_hs)
        //.func<&vec3_to_string>(to_string_hs)
        ;
}

// void register_transform(sol::state& lua)
// {
// }

// === HeaderComponent ========================================================

namespace {
    bool HeaderComponent_inspect(void* ptr, Editor::InspectorState& inspector)
{
    return false;
}
}

template<>
void register_meta<HeaderComponent>(sol::state& lua)
{
    // Register to entt::meta

    entt::meta<HeaderComponent>()
        .type("HeaderComponent"_hs).prop(display_name_hs, "Header")
        .data<&HeaderComponent::name>("name"_hs).prop(display_name_hs, "name")

        // Optional meta functions

        // to_string, member version
            //.func<&DebugClass::to_string>(to_string_hs)
        // to_string, lambda version
        .func < [](const void* ptr) {
        return static_cast<const HeaderComponent*>(ptr)->name;
        } > (to_string_hs)
            // inspect
                // .func<&inspect_Transform>(inspect_hs)
            // clone
                //.func<&cloneDebugClass>(clone_hs)
            ;

        // Register to sol

        lua.new_usertype<HeaderComponent>("HeaderComponent",
            "type_id", &entt::type_hash<HeaderComponent>::value,

            sol::call_constructor,
            sol::factories([](const std::string& name) {
                return HeaderComponent{
                    .name = name
                };
                }),
            "name", &HeaderComponent::name

            //sol::meta_function::to_string, &Transform::to_string
            );
}

// void HeaderComponent_metaregister(sol::state& lua)
// {
// }

// =========== ScriptedBehaviorComponent + sol stuff

namespace
{
    std::string sol_object_to_string(const sol::state& lua, const sol::object object)
    {
        return lua["tostring"](object).get<std::string>();
    }

    std::string get_lua_type_name(const sol::state& lua, const sol::object object)
    {
        return lua_typename(lua.lua_state(), static_cast<int>(object.get_type()));
    }
}

namespace Editor {

    /// Inspect sol::function
    template<>
    bool inspect_type<sol::function>(sol::function& t, InspectorState& inspector)
    {
        ImGui::TextDisabled("%s", sol_object_to_string(*inspector.lua, t).c_str());
        return false;
    }

    /// Inspect sol::table
    template<>
    bool inspect_type<sol::table>(sol::table& tbl, InspectorState& inspector)
    {
        auto& lua = *inspector.lua;
        bool mod = false;

        for (auto& [key, value] : tbl)
        {
            std::string key_str = sol_object_to_string(lua, key);
            std::string key_str_label = "##" + key_str;

            // Note: value.is<sol::table>() is true also for sol::type::userdata and possibly other lua types
            // This form,
            //      value.get_type() == sol::type::table
            // seems more precise when detecting types, even though it isn't type-safe

            if (value.get_type() == sol::type::table)
            {
                if (inspector.begin_node(key_str.c_str()))
                {
                    sol::table tbl_nested = value.as<sol::table>();

                    mod |= Editor::inspect_type(tbl_nested, inspector);
                    inspector.end_node();
                }
            }
            else if (value.get_type() == sol::type::userdata)
            {
                if (inspector.begin_node(key_str.c_str()))
                {
                    sol::userdata ud = value.as<sol::userdata>();

                    // Check if the userdata has an `__index` metamethod that acts like a table
                    sol::optional<sol::table> metatable = ud[sol::metatable_key];
                    if (metatable && metatable->get<sol::object>("__index").is<sol::table>()) {
                        sol::table index_table = metatable->get<sol::table>("__index");
                        Editor::inspect_type(index_table, inspector);
                    }
                    else {
                        ImGui::TextDisabled("[tostring] %s", sol_object_to_string(lua, value).c_str());
                    }
                    inspector.end_node();
                }
            }
            else
            {
                inspector.begin_leaf(key_str.c_str());

                if (value.get_type() == sol::type::function)
                {
                    sol::function func = value.as<sol::function>();
                    mod |= Editor::inspect_type(func, inspector);
                }
                else if (value.get_type() == sol::type::number)
                {
                    double dbl = value.as<double>();
                    if (ImGui::InputDouble(key_str_label.c_str(), &dbl, 0.1, 0.5))
                    {
                        // Commit modified value to Lua
                        tbl[key] = dbl;
                        mod = true;
                    }
                }
                else if (value.get_type() == sol::type::boolean)
                {
                    bool b = value.as<bool>();
                    if (ImGui::Checkbox(key_str_label.c_str(), &b))
                    {
                        // Commit modified value to Lua
                        tbl[key] = b;
                        mod = true;
                    }
                }
                else if (value.get_type() == sol::type::string)
                {
                    std::string str = value.as<std::string>();
                    if (inspect_type(str, inspector))
                    {
                        // Commit modified value to Lua
                        tbl[key] = str;
                        mod = true;
                    }
                }
                else
                {
                    // Fallback: display object as a string
                    // Applies to
                    // sol::type::lightuserdata
                    // sol::type::lua_nil
                    // sol::type::none
                    // sol::type::poly
                    // sol::type::thread

                    ImGui::TextDisabled("%s", sol_object_to_string(lua, value).c_str());
                }
                inspector.end_leaf();
            }
        }

        return mod;
    }

}

// specialize?
void ScriptedBehaviorComponent_metaregister(sol::state& lua)
{
    // TODO: Where should meta for sol stuff be placed (table, function etc)?
    // sol::table
    entt::meta<sol::table>()
        .type("sol::table"_hs).prop(display_name_hs, "sol::table")
        // inspect
        //.func<&soltable_inspect>(inspect_hs)
        // inspect v2
        .func < [](void* ptr, Editor::InspectorState& inspector) {return Editor::inspect_type(*static_cast<sol::table*>(ptr), inspector); } > (inspect_hs)
        ;

    // sol::function
    entt::meta<sol::function>()
        .type("sol::function"_hs).prop(display_name_hs, "sol::function")
        // inspect
        //.func<&solfunction_inspect>(inspect_hs)
        // inspect v2 - 'widget not implemented' for BehaviorScript::update, on_collision
        .func < [](void* ptr, Editor::InspectorState& inspector) {return Editor::inspect_type(*static_cast<sol::function*>(ptr), inspector); } > (inspect_hs)
        ;

    // ScriptedBehaviorComponent
    entt::meta<ScriptedBehaviorComponent>()
        .type("ScriptedBehaviorComponent"_hs).prop(display_name_hs, "ScriptedBehaviorComponent")
        .data<&ScriptedBehaviorComponent::scripts>("scripts"_hs).prop(display_name_hs, "scripts")

        // Optional meta functions

        // to_string, member version
//        .func<&DebugClass::to_string>(to_string_hs)
        // to_string, lambda version
//        .func < [](const void* ptr) { return static_cast<const HeaderComponent*>(ptr)->name; } > (to_string_hs)
            // inspect
                // .func<&inspect_Transform>(inspect_hs)
            // clone
                //.func<&cloneDebugClass>(clone_hs)
        ;

    entt::meta<ScriptedBehaviorComponent::BehaviorScript>()
        .type("BehaviorScript"_hs).prop(display_name_hs, "BehaviorScript")

        .data<&ScriptedBehaviorComponent::BehaviorScript::identifier>("identifier"_hs)
        .prop(display_name_hs, "identifier")
        .prop(readonly_hs, true)
        // sol stuff
        .data<&ScriptedBehaviorComponent::BehaviorScript::self>("self"_hs).prop(display_name_hs, "table")
        .data<&ScriptedBehaviorComponent::BehaviorScript::update>("update"_hs).prop(display_name_hs, "update")
        .data<&ScriptedBehaviorComponent::BehaviorScript::on_collision>("on_collision"_hs).prop(display_name_hs, "on_collision")
        ;

    // Done by registerScriptedBehaviorComponent,
    // but hould be done here instead
#if 0

    // Register to sol
    lua.new_usertype<HeaderComponent>("HeaderComponent",
        "type_id", &entt::type_hash<HeaderComponent>::value,

        sol::call_constructor,
        sol::factories([](const std::string& name) {
            return HeaderComponent{
                .name = name
            };
            }),
        "name", &HeaderComponent::name

        //sol::meta_function::to_string, &Transform::to_string
    );
#endif
}