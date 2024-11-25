#ifndef Scene_hpp
#define Scene_hpp
#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <entt/entt.hpp>

#include "vec.h"
#include "Log.hpp"
#include "SceneBase.h"
#include "Observer.h"
#include "SceneGraph.hpp"
#include "CommandQueue.hpp"

#include "ParticleBuffer.hpp"

class Scene : public eeng::SceneBase
{
public:
    bool init(const v2i& windowSize) override;

    void update(float time_s, float deltaTime_s) override;

    void renderUI() override;

    void render(float time_s, ShapeRendererPtr renderer) override;

    void destroy() override;

    class PlayStateManager
    {
        enum class PlayState : int { Play, Stop, Pause } play_state = PlayState::Stop;
    public:
        void play() { play_state = PlayState::Play; }
        void stop() { play_state = PlayState::Stop; }
        void pause() { play_state = PlayState::Pause; }
        bool is_play() { return play_state == PlayState::Play; }
        bool is_stop() { return play_state == PlayState::Stop; }
        bool is_pause() { return play_state == PlayState::Pause; }
        bool is_play_or_pause() { return play_state == PlayState::Play || play_state == PlayState::Pause; }
    };

private:
    std::shared_ptr<entt::registry> registry{};
    std::shared_ptr<sol::state> lua{};

    const std::string script_dir = "../../LuaGame/lua/"; // Todo: Should not be hard coded obviously

    std::vector<entt::entity> entities_pending_destruction;

    linalg::v3f lightPos, eyePos;
    const float nearPlane = 1.0f, farPlane = 10.0f;
    int drawcallCount = 0;
    bool debug_render = false;

    m4f VP, P, V;

    // entt::entity root_entity = entt::entity  {0};
    SceneGraph scenegraph{};

    // Particle buffer
    ParticleBuffer particleBuffer{};

    // Observer
    ConditionalObserver observer{};

    // (Editor) Command queue
    std::shared_ptr<Editor::CommandQueue> cmd_queue{};

    entt::entity create_entity_and_attach_to_scenegraph(entt::entity parent_entity = entt::null);

    void destroy_pending_entities();

    PlayStateManager play_state{};
};

#endif