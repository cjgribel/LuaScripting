#ifndef SceneBase_h
#define SceneBase_h
#pragma once

#include "ShapeRenderer.hpp"

using ShapeRendererPtr = std::shared_ptr<Renderer::ImPrimitiveRenderer>;

namespace eeng {
    class SceneBase
    {
    protected:
        // Input placeholder
        float axis_x = 0.0f, axis_y = 0.0f;
        bool button_pressed = false;

    public:
        void update_input(
            float controller_x,
            float controller_y,
            bool button_pressed)
        {
            this->axis_x = controller_x;
            this->axis_y = controller_y;
            this->button_pressed = button_pressed;
        }

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