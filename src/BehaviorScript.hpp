#ifndef BehaviorScript_hpp
#define BehaviorScript_hpp

#include <cassert>
#include <vector>

// sol is used by
//      For ScriptedBehaviorComponent => its own hpp/cpp
//      Lua event
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp> 
//#include <sol/forward.hpp>

struct BehaviorScript
{
    // Lua object
    sol::table self;
    // Update function of Lua object
    sol::protected_function update;
    sol::protected_function on_collision;
    std::string identifier;
    std::string path;

    // Called via entt callbacks when component is constructed & destroyed
    // sol::function init;
    // sol::function destroy;

#if 0
    BehaviorScript() = default;
    BehaviorScript(const BehaviorScript& other);
    BehaviorScript& operator=(const BehaviorScript& other);
    // BehaviorScript(BehaviorScript&& other) noexcept;
    // BehaviorScript& operator=(BehaviorScript&& other) noexcept;
    ~BehaviorScript();
#endif
};

struct BehaviorScriptFactory
{
    static BehaviorScript create_from_lua_object(
        entt::registry& registry,
        const entt::entity entity,
        const sol::table& script_table,
        const std::string& identifier,
        const std::string& script_path);

    static BehaviorScript create_from_file(
        entt::registry& registry,
        const entt::entity entity,
        std::shared_ptr<sol::state>& lua,
        const std::string& script_dir,
        const std::string& script_name);
};

// === LuaEvent ===============================================================

struct LuaEvent {
    sol::table data;
    std::string event_name;

    LuaEvent(const sol::table& data, const std::string& event_name)
        : data(data), event_name(event_name) {
    }
};

#endif // CoreComponents_hpp
