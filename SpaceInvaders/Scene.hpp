#ifndef Scene_hpp
#define Scene_hpp
#pragma once

// entt-sol2
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <entt/entt.hpp> // -> Scene source
#include <vector>

#include "vec.h"
#include "SceneBase.h"
#include "ParticleBuffer.hpp"

#define EntitySetSize 36
struct CircleColliderSetComponent
{
    v2f pos[EntitySetSize];
    float radii[EntitySetSize];
    float is_active_flags[EntitySetSize];

    int count = 0;
    bool is_active = true;
    unsigned char layer_bit, layer_mask;

    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << "{ radii = ";
        for (int i = 0; i < count; i++) ss << std::to_string(radii[i]) << ", ";
        ss << "{ is_active_flags = ";
        for (int i = 0; i < count; i++) ss << std::to_string(is_active_flags[i]) << ", ";
        return ss.str();
    }
};

struct IslandFinderComponent
{
    // Entity with QuadSetComponent
    // entt::entity colliderset_entity;

    // For floo-fill
    std::vector<bool> visited;
    std::queue<std::pair<int, int>> visit_queue;
    
    // Exposed to Lua
    std::vector<int> islands;
};

struct QuadSetComponent
{
    // static constexpr auto in_place_delete = true;

    v2f pos[EntitySetSize];
    float sizes[EntitySetSize];
    uint32_t colors[EntitySetSize];
    bool is_active_flags[EntitySetSize];
    int count = 0;
    bool is_active = true;

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

    linalg::v3f lightPos, eyePos;
    const float nearPlane = 1.0f, farPlane = 10.0f;
    int drawcallCount = 0;

    m4f VP, P, V;

    ParticleBuffer particleBuffer{};

public:
    bool init(const v2i& windowSize) override;

    void update(float time_s, float deltaTime_s) override;

    void renderUI() override;

    void render(float time_s, ShapeRendererPtr renderer) override;

    void destroy() override;
};

#endif