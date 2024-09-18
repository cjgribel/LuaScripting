
#include <entt/entt.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "imgui.h" // If GUI code is here

#include "meta_literals.h" // for entt literals

#include "CoreComponents.hpp"
#include "InspectorState.hpp" // for inspect()

//#include <iostream>

    // Inspect sol::function
template<>
bool Editor::inspect_type<sol::function>(sol::function& t, Editor::InspectorState& inspector)
{
    ImGui::TextDisabled("sol::function");
    return false;
}


    // bool soltable_inspect_with_luastate(sol::state& lua, void* ptr, Editor::InspectorState& inspector)
    // {
    //     ImGui::TextDisabled("soltable_inspect_with_luastate");
    //     return false;
    // }


    // With type
    // bool solfunction_inspect_(const sol::function& f, Editor::InspectorState& inspector)
    // {
    //     ImGui::TextDisabled("sol::function");
    //     return false;
    // }

    // bool solfunction_inspect(void* ptr, Editor::InspectorState& inspector)
    // {
    //     return Editor::inspect_type(*static_cast<sol::function*>(ptr), inspector);

    //     // return solfunction_inspect_(*static_cast<sol::function*>(ptr), inspector);
    // }

    // Specialization? Base template in MetaInspect.hpp (now it's in )?
    template<>
    bool Editor::inspect_type<sol::table>(sol::table& tbl, Editor::InspectorState& inspector)
        //bool soltable_inspect_rec(/*sol::state& lua,*/ sol::table tbl, Editor::InspectorState& inspector)
    {
        auto& lua = *inspector.lua;

        const auto sol_object_tostring = [](const sol::state& lua, const sol::object object)
            {
                return lua["tostring"](object).get<std::string>();
            };

        for (auto& kv : tbl)
        {
            sol::object key = kv.first;
            sol::object value = kv.second;

            std::string key_str = sol_object_tostring(lua, key);
            std::string key_str_label = "##" + key_str;
            // Append typename to key string
            //std::string type_name = lua_typename(lua.lua_state(), static_cast<int>(value.get_type()));
            //key_str = key_str + " (" + type_name + ")";

            if (value.get_type() == sol::type::table)
            {
                if (inspector.begin_node(key_str.c_str()))
                {
                    sol::table tbl_nested = value.as<sol::table>();

                    if (Editor::inspect_type(tbl_nested, inspector)) { /* Read-only */ }
                    // soltable_inspect_rec(value.as<sol::table>(), inspector);
                    inspector.end_node();
                }
            }
            else if (value.get_type() == sol::type::number)
            {
                double dbl = value.as<double>();

                inspector.begin_leaf(key_str.c_str());
                if (ImGui::InputDouble(key_str_label.c_str(), &dbl, 0.1, 0.5))
                {
                    // Commit modified value to Lua
                    tbl[key] = dbl;
                }
                inspector.end_leaf();
            }
            //else if (value.get_type() == sol::type::boolean)
            // else if (value.get_type() == sol::type::lightuserdata)
            //else if (value.get_type() == sol::type::lua_nil)
            // else if (value.get_type() == sol::type::none)
            // else if (value.get_type() == sol::type::poly)
            // else if (value.get_type() == sol::type::string)
            // else if (value.get_type() == sol::type::thread)
            // else if (value.get_type() == sol::type::userdata)
            else if (value.get_type() == sol::type::function)
            {
                sol::function func = value.as<sol::function>();

                inspector.begin_leaf(key_str.c_str());
                if (Editor::inspect_type(func, inspector)) { /* Read-only */ }
                inspector.end_leaf();
            }
            else
            {
                inspector.begin_leaf(key_str.c_str());
                ImGui::Text("[tostring] %s", sol_object_tostring(lua, value).c_str());
                inspector.end_leaf();
            }
        }

        return false;
    }

    // bool soltable_inspect(void* ptr, Editor::InspectorState& inspector)
    // {
    //     return soltable_inspect_rec(*static_cast<sol::table*>(ptr), inspector);
    // }
namespace
{
}

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
        .data<&ScriptedBehaviorComponent::BehaviorScript::identifier>("identifier"_hs).prop(display_name_hs, "identifier") // TODO const
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