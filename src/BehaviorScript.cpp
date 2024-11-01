
//#include <iostream>
#include <entt/entt.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "BehaviorScript.hpp"

#if 0
ScriptedBehaviorComponent::BehaviorScript::BehaviorScript(const ScriptedBehaviorComponent::BehaviorScript& other)
{
    // std::cout << "Copy Constructor called for " << name << "\n";
}

ScriptedBehaviorComponent::BehaviorScript& ScriptedBehaviorComponent::BehaviorScript::operator=(const ScriptedBehaviorComponent::BehaviorScript& other)
{
    if (this == &other) return *this; // Check for self-assignment
    // name = other.name;
    // std::cout << "Copy Assignment Operator called for " << name << "\n";
    return *this;
}

// ScriptedBehaviorComponent::BehaviorScript::BehaviorScript(ScriptedBehaviorComponent::BehaviorScript&& other) noexcept // : name(std::move(other.name)) 
// {
//     //    std::cout << "Move Constructor called for " << name << "\n";
// }

// ScriptedBehaviorComponent::BehaviorScript& ScriptedBehaviorComponent::BehaviorScript::operator=(ScriptedBehaviorComponent::BehaviorScript&& other) noexcept
// {
//     if (this == &other) return *this; // Check for self-assignment
//     //    name = std::move(other.name);
//       //  std::cout << "Move Assignment Operator called for " << name << "\n";
//     return *this;
// }

// Destructor
ScriptedBehaviorComponent::BehaviorScript::~BehaviorScript() {
    // std::cout << "Destructor called for " << name << "\n";
}
#endif

BehaviorScript BehaviorScriptFactory::create_from_lua_object(
    entt::registry& registry,
    const entt::entity entity,
    const sol::table& script_table,
    const std::string& identifier,
    const std::string& script_path)
{
    std::cout << "add_script " << identifier << " entity " << (uint32_t)entity << std::endl;
    //return;
    assert(script_table.valid());

    BehaviorScript script;

    script.self = script_table;

    script.update = script.self["update"];
    assert(script.update.valid());

    script.on_collision = script.self["on_collision"];
    assert(script.on_collision.valid());

    script.identifier = identifier;
    script.path = script_path;

    // -> entityID?
    script.self["id"] = sol::readonly_property([entity] { return entity; });
    // -> registry?
    script.self["owner"] = std::ref(registry); // &registry also seems to work

    // Run script init()
    // if (auto&& f = script.self["init"]; f.valid())
    //     f(script.self);
    // inspect_script(script);
    assert(script.self["init"].valid());
    script.self["init"](script.self);

    return script;

    // Add script to the list of scripts
    // auto& script_comp = registry.get_or_emplace<ScriptedBehaviorComponent>(entity);
    // script_comp.scripts.push_back(script);

    // Print the table's contents
#if 0
    std::cout << "Lua table contents:" << std::endl;
    for (auto& pair : script_table) {
        sol::object key = pair.first;
        sol::object value = pair.second;
        std::cout << "Key: " << key.as<std::string>() << ", Value: ";
        if (value.is<std::string>()) {
            std::cout << value.as<std::string>() << std::endl;
        }
        else if (value.is<int>()) {
            std::cout << value.as<int>() << std::endl;
        }
        else {
            std::cout << "Unknown type" << std::endl;
        }
    }
#endif
    // return script.self;
}

BehaviorScript BehaviorScriptFactory::create_from_file(
    entt::registry& registry,
    const entt::entity entity,
    std::shared_ptr<sol::state>& lua,
    const std::string& script_dir,
    const std::string& script_name
)
{
    const std::string script_path = script_dir + script_name + ".lua";
    // sol::load_result behavior_script = lua.load_file(script_file);
    sol::load_result behavior_script = lua->load_file(script_path);

    assert(behavior_script.valid());
    sol::protected_function script_function = behavior_script;
    return BehaviorScriptFactory::create_from_lua_object(registry, entity, script_function(), script_name, script_path);
}