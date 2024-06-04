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

class Scene : public eeng::SceneBase
{
protected:
    sol::state lua{}; //(sol::c_call<decltype(&my_panic), &my_panic>);
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