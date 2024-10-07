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

// === Transform ==============================================================

struct Transform
{
    // If stable pointers to Transform are needed, e.g. in scene graph nodes
    // https://github.com/skypjack/entt/blob/master/docs/md/entity.md
    //static constexpr auto in_place_delete = true;

    float x{ 0.0f }, y{ 0.0f }, angle{ 0.0f };

    // Not meta-registered
    float x_parent{ 0.0f }, y_parent{ 0.0f }, angle_parent{ 0.0f };
    float x_global{ 0.0f }, y_global{ 0.0f }, angle_global{ 0.0f };

    // void compute_global_transform()
    // {
    //     x_global = x * cos(angle_parent) - y * sin(angle_parent) + x_parent;
    //     y_global = x * sin(angle_parent) + y * cos(angle_parent) + y_parent;
    //     angle_global = angle + angle_parent;
    // }

    std::string to_string() const
    {
        std::stringstream ss;
        ss << "Transform { x = " << std::to_string(x)
            << ", y = " << std::to_string(y)
            << ", angle = " << std::to_string(angle) << " }";
        return ss.str();
    }
};


// === HeaderComponent ========================================================

struct HeaderComponent
{
    std::string name;
    // std::string name2;
    // std::string name3;

    std::string to_string() const
    {
        std::stringstream ss;
        ss << "HeaderComponent { name = " << name << " }";
        return  ss.str();
    }
};

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

    std::string to_string() const
    {
        return "CircleColliderGridComponent { ... }";
        //     std::stringstream ss;
        //     ss << "{ radii = ";
        //     for (int i = 0; i < count; i++) ss << std::to_string(radii[i]) << ", ";
        //     ss << "{ is_active_flags = ";
        //     for (int i = 0; i < count; i++) ss << std::to_string(is_active_flags[i]) << ", ";
        //     return ss.str();
    }
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

    std::string to_string() const
    {
        return "IslandFinderComponent { ... }";
    }
};

// === QuadGridComponent ======================================================

struct QuadGridComponent
{
    std::array<v2f, GridSize> positions;
    std::array<float, GridSize> sizes;
    std::array<uint32_t, GridSize> colors;
    std::array<bool, GridSize> is_active_flags = { false };

    // v2f pos[GridSize];
    // float sizes[GridSize];
    // uint32_t colors[GridSize];
    // bool is_active_flags[GridSize] = { false };

    int count = 0, width = 0;
    bool is_active = true;

    std::string to_string() const
    {
        return "QuadGridComponent { ... }";
        // std::stringstream ss;
        // ss << "{ size = ";
        // for (int i = 0; i < count; i++) ss << std::to_string(sizes[i]) << ", ";
        // ss << "{ colors = ";
        // for (int i = 0; i < count; i++) ss << std::to_string(colors[i]) << ", ";
        // ss << "{ is_active_flags = ";
        // for (int i = 0; i < count; i++) ss << std::to_string(is_active_flags[i]) << ", ";
        // return ss.str();
    }
};

// === DataGridComponent ======================================================

struct DataGridComponent
{
    std::array<float, GridSize> slot1 = { 0.0f };
    std::array<float, GridSize> slot2 = { 0.0f };
    int count = 0, width = 0;

        std::string to_string() const
    {
        return "DataGridComponent { ... }";
    }
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
        // std::string src_file;

        // Called via entt callbacks when component is constructed & destroyed
        // sol::function init;
        // sol::function destroy;
    };

    std::vector<BehaviorScript> scripts;

    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << "{ scripts = ";
        for (auto& script : scripts) ss << script.identifier << " ";
        ss << "}";
        return ss.str();
    }
};

static_assert(std::is_move_constructible_v<ScriptedBehaviorComponent>);

// === LuaEvent ===============================================================

struct LuaEvent {
    sol::table data;
    std::string event_name;

    LuaEvent(const sol::table& data, const std::string& event_name)
        : data(data), event_name(event_name) {}
};

// === Meta registration ======================================================

template<>
void register_meta<Transform>(sol::state& lua);

template<>
void register_meta<HeaderComponent>(sol::state& lua);

template<>
void register_meta<CircleColliderGridComponent>(sol::state& lua);

template<>
void register_meta<IslandFinderComponent>(sol::state& lua);

template<>
void register_meta<QuadGridComponent>(sol::state& lua);

template<>
void register_meta<DataGridComponent>(sol::state& lua);

template<>
void register_meta<ScriptedBehaviorComponent>(sol::state& lua);

#endif // CoreComponents_hpp
