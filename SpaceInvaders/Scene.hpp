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

struct CircleComponent
{
    // static constexpr auto in_place_delete = true;

    float r;

    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << "{ r =" << std::to_string(r) << " }";
        return ss.str();
    }
};

// ScriptedBehaviorComponent
struct ScriptComponent
{
    // BehaviorScript
    struct Script
    {
        sol::table self;
        // sol::function init;
        // sol::function destroy;
        sol::function update;
    };
    std::vector<Script> scripts;
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
    // sol::state lua{}; //(sol::c_call<decltype(&my_panic), &my_panic>);
    sol::state lua{ (sol::c_call<decltype(&my_panic), &my_panic>) };
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