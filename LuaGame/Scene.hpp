#ifndef Scene_hpp
#define Scene_hpp
#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <entt/entt.hpp>

#include "config.h"
#include "vec.h"
#include "Log.hpp"
#include "SceneBase.h"
#include "Observer.h"
#include "SceneGraph.hpp"
#include "CommandQueue.hpp"

#include "ParticleBuffer.hpp"

// TODO --> ChunkRegistry.HPP/CPP
class ChunkRegistry
{
public:
    using EntityVector = std::vector<entt::entity>;

    // Adapter for chunks
    class ChunkIterator
    {
    public:
        using MapType = std::unordered_map<std::string, EntityVector>;
        using iterator = MapType::iterator;
        ChunkIterator(MapType& registry) : registry(registry) {}
        iterator begin() { return registry.begin(); }
        iterator end() { return registry.end(); }

    private:
        MapType& registry;
    };

    // Adapter for entities in a chunk
    class EntityIterator {
    public:
        using iterator = EntityVector::iterator;
        EntityIterator(EntityVector& entities) : entities(entities) {}
        iterator begin() { return entities.begin(); }
        iterator end() { return entities.end(); }
    private:
        EntityVector& entities;
    };

    bool chunk_exists(const std::string& chunk_id) const
    {
        return registry.find(chunk_id) != registry.end();
    }

    EntityIterator chunk(const std::string& chunk_id)
    {
        assert(chunk_exists(chunk_id) && "Chunk does not exist!");
        return EntityIterator(registry.at(chunk_id));
    }

    ChunkIterator chunks() { return ChunkIterator(registry); }

    // Add an entity to a specific chunk
    void addEntity(const std::string& chunk_id, entt::entity entity)
    {
        //assert(chunk_exists(chunk_id) && "Chunk does not exist!");
        assert(!entity_exists(entity) && "Entity already exists in a chunk!");

        registry[chunk_id].push_back(entity);
    }

    void removeEntity(const std::string& chunk_id, entt::entity entity) {
        assert(chunk_exists(chunk_id) && "Chunk does not exist!");

        auto& entities = registry.at(chunk_id);
        auto it = std::find(entities.begin(), entities.end(), entity);
        if (it != entities.end()) {
            entities.erase(it);
        }
    }

    void removeEntity(entt::entity entity) {
        for (auto& [chunk_id, entities] : registry) {
            auto it = std::find(entities.begin(), entities.end(), entity);
            if (it != entities.end()) {
                entities.erase(it);
                return;
            }
        }
    }

    // Check if an entity exists in a specific chunk
    bool entity_exists_in_chunk(const std::string& chunk_id, entt::entity entity) const
    {
        assert(chunk_exists(chunk_id) && "Chunk does not exist!");
        const auto& entities = registry.at(chunk_id);
        return std::find(entities.begin(), entities.end(), entity) != entities.end();
    }

    // Check if an entity exists in any chunk
    bool entity_exists(entt::entity entity) const
    {
        for (const auto& [chunk_id, entities] : registry)
        {
            if (std::find(entities.begin(), entities.end(), entity) != entities.end())
            {
                return true;
            }
        }
        return false;
    }

    // Create a new chunk
    void create_chunk(const std::string& chunk_id)
    {
        assert(!chunk_exists(chunk_id) && "Chunk already exists!");
        registry[chunk_id] = {};
    }

private:
    std::unordered_map<std::string, EntityVector> registry;
};


class Scene : public eeng::SceneBase
{
public:
    enum class GamePlayState : int { Play, Stop, Pause };
    struct SetGamePlayStateEvent { GamePlayState play_state; };
    struct DestroyChunkEvent { std::string chunk_tag; };
    struct LoadFileEvent { std::string path; };
    struct CreateEntityEvent { entt::entity parent_entity; };
    struct DestroyEntityEvent { entt::entity entity; };
    struct CopyEntityEvent { entt::entity entity; };

    bool init(const v2i& windowSize) override;

    void update(float time_s, float deltaTime_s) override;

    void renderUI() override;

    void render(float time_s, ShapeRendererPtr renderer) override;

    void destroy() override;


private:

    // Content management
    // void assign_entity_to_chunk(entt::registry& registry, entt::entity);
    // void destroy_chunk(const std::string& chunk_tag);
    void save_chunk(const std::string& chunk_tag);
    void save_all_chunks();
    void load_json(const std::string& path);



    void OnSetGamePlayStateEvent(const SetGamePlayStateEvent& event);
    void OnDestroyChunkEvent(const DestroyChunkEvent& event);
    void OnLoadFileEvent(const LoadFileEvent& event);
    void OnCreateEntityEvent(const CreateEntityEvent& event);
    void OnDestroyEntityEvent(const DestroyEntityEvent& event);
    void OnCopyEntityEvent(const CopyEntityEvent& event);

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

    entt::entity create_entity(
        const std::string& chunk_tag = "",
        const std::string& name = "",
        entt::entity parent_entity = entt::null);
    void queue_entity_for_destruction(entt::entity entity);
    void destroy_pending_entities();

    ChunkRegistry chunk_registry{};

    GamePlayState play_state{ GamePlayState::Stop };
};

#endif