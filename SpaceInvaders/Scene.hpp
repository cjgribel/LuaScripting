#ifndef Scene_hpp
#define Scene_hpp
#pragma once

#include <entt/entt.hpp> // -> Scene source
#include "vec.h"
#include "SceneBase.h"

class Scene : public eeng::SceneBase
{
protected:
    entt::registry registry;
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