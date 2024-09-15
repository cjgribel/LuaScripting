
#include <entt/entt.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "imgui.h" // If GUI code is here

#include "meta_literals.h" // for entt literals

#include "CoreComponents.hpp"
#include "InspectorState.hpp" // for inspect()

//#include <iostream>

void ScriptedBehaviorComponent_metaregister(sol::state& lua)
{
    // Register to entt::meta

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
        .data<&ScriptedBehaviorComponent::BehaviorScript::identifier>("identifier"_hs).prop(display_name_hs, "identifier")
        // Todo: 'self' (sol::table)
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