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

class ChunkRegistry {
public:
    using EntityVector = std::vector<entt::entity>;

    // Iteration adapter for a single chunk
    class ChunkIterator {
    public:
        using iterator = EntityVector::iterator;

        ChunkIterator(EntityVector& entities) : entities(entities) {}

        iterator begin() { return entities.begin(); }
        iterator end() { return entities.end(); }

    private:
        EntityVector& entities;
    };

    // Access entities for a specific chunk
    ChunkIterator chunk(const std::string& chunk_tag) {
        auto it = registry.find(chunk_tag);
        if (it == registry.end()) {
            throw std::runtime_error("Chunk not found: " + chunk_tag);
        }
        return ChunkIterator(it->second);
    }

    // Add an entity to a specific chunk
    void addEntity(const std::string& chunk_id, entt::entity entity) {
        registry[chunk_id].push_back(entity);
    }

private:
    std::unordered_map<std::string, EntityVector> registry;
};

class Scene : public eeng::SceneBase
{
public:
    enum class GamePlayState : int { Play, Stop, Pause };
    struct GamePlayStateEvent { GamePlayState play_state; };
    
    bool init(const v2i& windowSize) override;

    void update(float time_s, float deltaTime_s) override;

    void renderUI() override;

    void render(float time_s, ShapeRendererPtr renderer) override;

    void destroy() override;


private:

    // Content management
    void remove_chunk(const std::string& chunk_tag);
    void load_json(const std::string& path);
    void save_chunk(const std::string& chunk_tag);
    void save_all_chunks();


    void OnGamePlayStateChanged(const GamePlayStateEvent& event);

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

    ChunkRegistry chunk_registry {};

    GamePlayState play_state { GamePlayState::Stop };
};

#endif