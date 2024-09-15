
#include <entt/entt.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "imgui.h" // If GUI code is here

#include "meta_literals.h" // for entt literals

#include "CoreComponents.hpp"
#include "InspectorState.hpp" // for inspect()

//#include <iostream>

namespace
{
    // bool soltable_inspect_with_luastate(sol::state& lua, void* ptr, Editor::InspectorState& inspector)
    // {
    //     ImGui::TextDisabled("soltable_inspect_with_luastate");
    //     return false;
    // }

    bool solfunction_inspect(void* ptr, Editor::InspectorState& inspector)
    {
        ImGui::TextDisabled("sol::function");
        return false;
    }

    // Specialization? Base template in MetaInspect.hpp (now it's in )?
    bool soltable_inspect_rec(sol::state& lua, sol::table tbl, Editor::InspectorState& inspector)
    {
        // leaf / node?
        // ImGui::TextDisabled("sol::table");
        // return false;

        //auto& lua = *inspector.lua;

        //sol::table* tbl = static_cast<sol::table*>(ptr);
        for (auto& kv : tbl)
        {
            sol::object key = kv.first;
            sol::object value = kv.second;

            // Assuming the key is a string for simplicity
            std::string key_str = key.as<std::string>();

            //ImGui::TextDisabled("%s", key_str.c_str());
            if (inspector.begin_node(key_str.c_str()))
            {
                std::string type_name = lua_typename(lua.lua_state(), static_cast<int>(value.get_type()));

                //std::string indent = "   ";
                //std::cout << key_str << " (type: " << type_name << ")\n";

                if (value.get_type() == sol::type::table) {
                    soltable_inspect_rec(lua, value.as<sol::table>(), inspector);
                }
                else if (value.get_type() == sol::type::function) {
                    //std::cout << indent << "    [function]\n";
                    //solfunction_inspect(&value.as<sol::table>(), inspector);
                    // ImGui::TextDisabled("sol::function");
                }
                else {
                    //std::cout << indent << "    [" << lua["tostring"](value).get<std::string>() << "]\n";
                    inspector.begin_leaf("value");
                    ImGui::Text("%s", lua["tostring"](value).get<std::string>().c_str());
                    inspector.end_leaf();
                }

                inspector.end_node();
            }
        }

        return false;
    }

    bool soltable_inspect(void* ptr, Editor::InspectorState& inspector)
    {
        return soltable_inspect_rec(*inspector.lua, *static_cast<sol::table*>(ptr), inspector);
    }
}

void ScriptedBehaviorComponent_metaregister(sol::state& lua)
{
    // TODO: Where should meta for sol stuff be placed (table, function etc)?
    // sol::table
    entt::meta<sol::table>()
        .type("sol::table"_hs).prop(display_name_hs, "sol::table")
        // inspect
        .func<&soltable_inspect>(inspect_hs)

        ;

    entt::meta<sol::function>()
        .type("sol::function"_hs).prop(display_name_hs, "sol::function")
        // inspect
        .func<&solfunction_inspect>(inspect_hs)

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