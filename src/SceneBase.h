#ifndef SceneBase_h
#define SceneBase_h
#pragma once

#include "ShapeRenderer.hpp"

using ShapeRendererPtr = std::shared_ptr<Renderer::ImPrimitiveRenderer>;

namespace eeng {
    class SceneBase
    {
    public:
        virtual bool init() = 0;

        virtual void update(float time_s, float deltaTime_s) = 0;

        virtual void renderUI() = 0;

        virtual void render(
            float time_s,
            int screenWidth,
            int screenHeight,
            ShapeRendererPtr renderer) = 0;

        virtual void destroy() = 0;
    };
}

#endif