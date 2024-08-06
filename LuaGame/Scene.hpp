#ifndef Scene_hpp
#define Scene_hpp
#pragma once

// entt-sol2
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <entt/entt.hpp>
#include <vector>

#include "vec.h"
#include "Log.hpp"
#include "SceneBase.h"
#include "Observer.h"
#include "ParticleBuffer.hpp"

// Sparse set
#define EntitySetSize 64

#include <cassert>
#include <array>

template<class T, int N>
struct SparseSet
{
    std::array<T, N> dense;
    std::array<int, N> sparse;
    int count = 0;

    SparseSet()
    {
        sparse.fill(-1); // Initialize all elements to -1
    }

    bool contains(int index) const
    {
        assert(index >= 0 && index < N && "Index out of bounds");
        return sparse[index] != -1;
    }

    void add(int index)
    {
        assert(count < N && "EntitySetSize limit reached");
        assert(index >= 0 && index < N && "Index out of bounds");

        if (contains(index))
            return; // already in the set

        sparse[index] = count;
        dense[count] = index;
        count++;
    }

    void remove(int index) 
    {
        assert(index >= 0 && index < N && "Index out of bounds");

        if (!contains(index)) return;

        int index_to_remove = sparse[index];
        int last_index = count - 1;

        if (index_to_remove != last_index) 
        {
            // Back-swap
            dense[index_to_remove] = dense[last_index];
            sparse[dense[last_index]] = index_to_remove;
        }

        // Invalidate the removed element
        sparse[index] = -1;
        count--;
    }

    /// @brief Get index value from the dense map at an index
    /// @param index Dense map index
    /// @return index value
    T get_dense(int index) const
    {
        assert(index >= 0 && index < count && "Index out of bounds");
        return dense[index];
    }

    /// @brief Get number of elements in the dense map
    /// @return Number of added indices 
    int get_dense_count() const
    {
        return count;
    }
};

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

inline void my_panic(sol::optional<std::string> maybe_msg)
{
    std::cerr << "Lua is in a panic state and will now abort() the application" << std::endl;
    if (maybe_msg) {
        const std::string& msg = maybe_msg.value();
        std::cerr << "\terror message: " << msg << std::endl;
    }
    // When this function exits, Lua will exhibit default behavior and abort()
}

class Scene : public eeng::SceneBase
{
protected:
    sol::state lua{};

    entt::registry registry{};
    std::vector<entt::entity> entities_pending_destruction;

    linalg::v3f lightPos, eyePos;
    const float nearPlane = 1.0f, farPlane = 10.0f;
    int drawcallCount = 0;
    bool debug_render = false;

    m4f VP, P, V;

    ConditionalObserver observer;
    ParticleBuffer particleBuffer{};

public:
    bool init(const v2i& windowSize) override;

    void update(float time_s, float deltaTime_s) override;

    void renderUI() override;

    void render(float time_s, ShapeRendererPtr renderer) override;

    void destroy() override;
};

#endif