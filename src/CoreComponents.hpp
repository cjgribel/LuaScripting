#ifndef CoreComponents_hpp
#define CoreComponents_hpp

#include <cassert>
#include <array>
#include <queue>

#include "meta_reg.h"

// sol is used by
//      For ScriptedBehaviorComponent => its own hpp/cpp
//      Lua event
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp> 
//#include <sol/forward.hpp>
// Fwwd decl?
// namespace sol {
//     class state;
//     class state_view;
//     class table;
//     // Add other forward declarations as needed
// }

#include "vec.h"
#include "SparseSet.hpp"

using linalg::v2f;
#define GridSize 64

// === HeaderComponent ========================================================

// GET ME HOME
struct HeaderComponent
{
    std::string name;
};



// void HeaderComponent_metaregister(sol::state& lua); // REMOVE

template<>
void register_meta<HeaderComponent>(sol::state& lua);

// === Transform ==============================================================

struct Transform
{
    // If stable pointers to Transform are needed, e.g. in scene graph nodes
    // https://github.com/skypjack/entt/blob/master/docs/md/entity.md
    //static constexpr auto in_place_delete = true;

    float x{ 0.0f }, y{ 0.0f }, rot{ 0.0f };

    // Not meta-registered
    float x_parent{ 0.0f }, y_parent{ 0.0f }, rot_parent{ 0.0f };
    float x_global{ 0.0f }, y_global{ 0.0f }, rot_global{ 0.0f };

    // void compute_global_transform()
    // {
    //     x_global = x * cos(rot_parent) - y * sin(rot_parent) + x_parent;
    //     y_global = x * sin(rot_parent) + y * cos(rot_parent) + y_parent;
    //     rot_global = rot + rot_parent;
    // }

    std::string to_string() const
    {
        std::stringstream ss;
        ss << "Transform { x = " << std::to_string(x)
            << ", y = " << std::to_string(y)
            << ", rot = " << std::to_string(rot) << " }";
        return ss.str();
    }
};

template<>
void register_meta<Transform>(sol::state& lua);

// void register_transform(sol::state& lua);

// === CircleColliderGridComponent ============================================

struct CircleColliderGridComponent
{
    struct Circle
    {
        v2f pos;
        float radius;
    };

    std::array<Circle, GridSize> circles;
    SparseSet<unsigned char, GridSize> active_indices;

    // v2f pos[GridSize];
    // float radii[GridSize];
    // bool is_active_flags[GridSize] = { false };

    // int sparse[GridSize]; // sparse
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

// === IslandFinderComponent ==================================================

struct IslandFinderComponent
{
    int core_x, core_y;

    // For flood-fill
    std::vector<bool> visited;
    std::queue<std::pair<int, int>> visit_queue;

    // Exposed to Lua
    std::vector<int> islands;
};

// === QuadGridComponent ======================================================

struct QuadGridComponent
{
    // static constexpr auto in_place_delete = true;

    v2f pos[GridSize];
    float sizes[GridSize];
    uint32_t colors[GridSize];
    bool is_active_flags[GridSize] = { false };
    // unsigned char active_indices[GridSize];

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

// === DataGridComponent ======================================================

struct DataGridComponent
{
    float slot1[GridSize] = { 0.0f };
    float slot2[GridSize] = { 0.0f };
    int count = 0, width = 0;
};

// === NOT USED ===============================================================

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

// === ScriptedBehaviorComponent ==============================================

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

void ScriptedBehaviorComponent_metaregister(sol::state& lua);

static_assert(std::is_move_constructible_v<ScriptedBehaviorComponent>);

// === LuaEvent ===============================================================

struct LuaEvent {
    sol::table data;
    std::string event_name;

    LuaEvent(const sol::table& data, const std::string& event_name)
        : data(data), event_name(event_name) {}
};

#endif // CoreComponents_hpp
