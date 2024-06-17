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

struct QuadComponent
{
    // static constexpr auto in_place_delete = true;

    float w;
    uint32_t color;

    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << "{ w =" << std::to_string(w) << " }";
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

        // Called via entt callbacks when component is constructed & destroyed
        // sol::function init;
        // sol::function destroy;

        // template<typename... Args>
        // BehaviorScript()
        // {
        //     (script_files.emplace_back(std::forward<Args>(args)), ...);
        // }
    };

    std::vector<std::string> script_files;
    std::vector<BehaviorScript> scripts;
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
    const float nearPlane = 1.0f, farPlane = 500.0f;
    int drawcallCount = 0;

public:
    bool init() override;

    void update(float time_s, float deltaTime_s) override;

    void renderUI() override;

    void render(
        float time_s,
        int screenWidth,
        int screenHeight,
        ShapeRendererPtr renderer) override;

    void destroy() override;
};

#endif