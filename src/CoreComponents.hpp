#ifndef CoreComponents_hpp
#define CoreComponents_hpp

#include <cassert>
#include <array>
#include <queue>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "vec.h"
#include "SparseSet.hpp"

#define EntitySetSize 64

struct CircleColliderGridComponent
{
    struct Circle 
    {
        v2f pos;
        float radius;
    };

    std::array<Circle, EntitySetSize> circles;
    SparseSet<unsigned char, EntitySetSize> active_indices;
     
    // v2f pos[EntitySetSize];
    // float radii[EntitySetSize];
    // bool is_active_flags[EntitySetSize] = { false };

    // int sparse[EntitySetSize]; // sparse
    // int nbr_active = 0;

    int element_count = 0;
    int width = 0;
    bool is_active = true;
    unsigned char layer_bit, layer_mask;

    // [[nodiscard]] std::string to_string() const {
    //     std::stringstream ss;
    //     ss << "{ radii = ";
    //     for (int i = 0; i < count; i++) ss << std::to_string(radii[i]) << ", ";
    //     ss << "{ is_active_flags = ";
    //     for (int i = 0; i < count; i++) ss << std::to_string(is_active_flags[i]) << ", ";
    //     return ss.str();
    // }
};

struct IslandFinderComponent
{
    int core_x, core_y;

    // For flood-fill
    std::vector<bool> visited;
    std::queue<std::pair<int, int>> visit_queue;

    // Exposed to Lua
    std::vector<int> islands;
};

struct QuadGridComponent
{
    // static constexpr auto in_place_delete = true;

    v2f pos[EntitySetSize];
    float sizes[EntitySetSize];
    uint32_t colors[EntitySetSize];
    bool is_active_flags[EntitySetSize] = { false };
    // unsigned char active_indices[EntitySetSize];

    int count = 0, width = 0;
    bool is_active = true;
    // int nbr_active = 0;

    // Todo: pos
    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << "{ size = ";
        for (int i = 0; i < count; i++) ss << std::to_string(sizes[i]) << ", ";
        ss << "{ colors = ";
        for (int i = 0; i < count; i++) ss << std::to_string(colors[i]) << ", ";
        ss << "{ is_active_flags = ";
        for (int i = 0; i < count; i++) ss << std::to_string(is_active_flags[i]) << ", ";
        return ss.str();
    }
};

struct DataGridComponent
{
    float slot1[EntitySetSize] = { 0.0f };
    float slot2[EntitySetSize] = { 0.0f };
    int count = 0, width = 0;
};

// NOT USED
struct QuadComponent
{
    // static constexpr auto in_place_delete = true;

    float w;
    uint32_t color;
    bool is_visible;

    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << "{ w " << std::to_string(w) << ", color " << color << ", is_visible " << is_visible << " }";
        return ss.str();
    }
};

// NOT USED
struct CircleColliderComponent
{
    // static constexpr auto in_place_delete = true;

    float r;
    bool is_active;

    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << "{ r =" << std::to_string(r) << ", is_active " << is_active << " }";
        return ss.str();
    }
};

// ScriptedBehaviorComponent
struct ScriptedBehaviorComponent
{
    // BehaviorScript
    struct BehaviorScript
    {
        // Lua object
        sol::table self;
        // Update function of Lua object
        sol::function update;
        sol::function on_collision;
        std::string identifier;

        // Called via entt callbacks when component is constructed & destroyed
        // sol::function init;
        // sol::function destroy;
    };

    std::vector<BehaviorScript> scripts;

    // sol::table get_script_by_id(const std::string& identifier)
    // {
    //     for (auto& script : scripts)
    //     {
    //         if (script.identifier == identifier)
    //         {
    //             return script.self;
    //         }
    //     }
    //     return sol::lua_nil;
    // }

    // TODO: add_script
    // TODO: add_script_from_file?

    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << "{ scripts = ";
        for (auto& script : scripts) ss << script.identifier << " ";
        ss << "}";
        return ss.str();
    }
};

static_assert(std::is_move_constructible_v<ScriptedBehaviorComponent>);

struct LuaEvent {
    sol::table data;
    std::string event_name;

    LuaEvent(const sol::table& data, const std::string& event_name)
        : data(data), event_name(event_name) {}
};

#endif // CoreComponents_hpp
