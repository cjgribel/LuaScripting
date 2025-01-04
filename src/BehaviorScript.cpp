
//#include <iostream>
// #include <entt/entt.hpp>

// #define SOL_ALL_SAFETIES_ON 1
// #include <sol/sol.hpp>

#include "BehaviorScript.hpp"

namespace {
    // void copy(BehaviorScript& dst, const BehaviorScript& src)
    // {
    //     dst.update = dst.self["update"];
    //     assert(dst.update.valid());

    //     dst.on_collision = dst.self["on_collision"];
    //     assert(dst.on_collision.valid());

    //     // entity
    //     // FOR NOW: copy entity from rhs
    //     //script.self["id"] = sol::readonly_property([entity] { return entity; });
    //     self["id"] = sol::readonly_property([&] { return rhs.self["id"](); });

    //     // Rebind original's registry to copy
    //     //script.self["owner"] = std::ref(registry); // &registry also seems to work
    //     {
    //         auto registry_ref = rhs.self["owner"];
    //         assert(registry_ref.valid());
    //         auto rebind_func = registry_ref["rebind"];
    //         assert(rebind_func.valid());
    //         rebind_func(self);
    //     }

    //     // Run script init()
    //     // if (auto&& f = script.self["init"]; f.valid())
    //     //     f(script.self);
    //     // inspect_script(script);
    //     assert(self["init"].valid());
    //     self["init"](self);
    // }
}

#if defined(BEHAVIORSCRIPT_EXPLICIT_DEEPCOPY) or defined(BEHAVIORSCRIPT_EXPLICIT_SHALLOWCOPY)

BehaviorScript::BehaviorScript(const BehaviorScript& rhs)
    : identifier(rhs.identifier), path(rhs.path)
{
    std::cout << "copyctor BehaviorScript(const BehaviorScript& other)" << std::endl;;

#ifdef BEHAVIORSCRIPT_EXPLICIT_SHALLOWCOPY)
    // SHALLOW
    self = rhs.self;
    update = rhs.update;
    on_collision = rhs.on_collision;
#endif
#ifdef BEHAVIORSCRIPT_EXPLICIT_DEEPCOPY)
    // DEEP
    sol::state_view lua = rhs.self.lua_state();

    // Load script using rhs' path
    sol::load_result behavior_script = lua.load_file(rhs.path);
    assert(behavior_script.valid());
    sol::protected_function script_function = behavior_script; // <- store this generator function?

    // Copy / transfer rhs -> 
    self = script_function();

    update = self["update"];
    assert(update.valid());

    on_collision = self["on_collision"];
    assert(on_collision.valid());

    // entity
    // FOR NOW: copy entity from rhs
    //script.self["id"] = sol::readonly_property([entity] { return entity; });
    {
        sol::protected_function id_func = rhs.self["id"];
        assert(id_func.valid());
        entt::entity entity = id_func();
        self["id"] = sol::readonly_property([&] { return entity; });

        // CHECK
        entt::entity new_entity = self["id"]();
        std::cout << "copy entity " << entt::to_integral(entity) << ", " << entt::to_integral(new_entity) << std::endl;
    }

    // Rebind original's registry to copy
    //script.self["owner"] = std::ref(registry); // &registry also seems to work
    {
        auto registry_ref = rhs.self["owner"];      assert(registry_ref.valid());
        //if (registry_ref.get_type() == sol::type::userdata) std::cout << "registry_ref is userdata\n";
        auto rebind_func = registry_ref["rebind"];  assert(rebind_func.valid());
        //if (rebind_func.get_type() == sol::type::function) std::cout << "rebind_func is function\n";
        rebind_func(registry_ref, self);

        // CHECK
        auto new_registry_ref = self["owner"];
        assert(new_registry_ref.valid());
        auto new_rebind = self["owner"]["rebind"];
        assert(new_rebind.valid());
    }

    // Run script init()
    // if (auto&& f = script.self["init"]; f.valid())
    //     f(script.self);
    // inspect_script(script);
    //    assert(self["init"].valid());
    //    self["init"](self);

        // + TRANFER DATA FROM COPIED

#endif
}

BehaviorScript& BehaviorScript::operator=(const BehaviorScript& rhs)
{
    // assert(0);

#if 1
    // SHALLOW
    self = rhs.self;
    update = rhs.update;
    on_collision = rhs.on_collision;
    identifier = rhs.identifier;
    path = rhs.path;
#endif

    std::cout << "copyassg BehaviorScript::operator=(const BehaviorScript& other)" << std::endl;;

    if (this == &rhs) return *this;

    // Anything to do when removing the current content?

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

#if 0
// Destructor
BehaviorScript::~BehaviorScript() {
    // std::cout << "Destructor called for " << name << "\n";
}
#endif
#endif

BehaviorScript BehaviorScriptFactory::create_from_lua_object(
    entt::registry& registry,
    const entt::entity entity,
    const sol::table& script_table,
    const std::string& identifier,
    const std::string& script_path)
{
    // std::cout << "BehaviorScriptFactory::create_from_lua_object " << identifier << " entity " << (uint32_t)entity << std::endl;
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
    // script.self["init"](script.self);

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
    sol::state_view lua,
    const std::string& script_path,
    const std::string& script_name
)
{
    //const std::string script_path = script_dir + script_name + ".lua";
    // sol::load_result behavior_script = lua.load_file(script_file);
    sol::load_result behavior_script = lua.load_file(script_path);

    assert(behavior_script.valid());
    sol::protected_function script_function = behavior_script;
    return BehaviorScriptFactory::create_from_lua_object(registry, entity, script_function(), script_name, script_path);
}