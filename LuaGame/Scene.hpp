#ifndef Scene_hpp
#define Scene_hpp
#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <entt/entt.hpp>

#include "SceneTypes.h"
#include "config.h"
#include "vec.h"
#include "Log.hpp"
#include "SceneBase.h"
#include "Observer.h"
#include "ChunkRegistry.hpp"
#include "SceneGraph.hpp"
#include "CommandQueue.hpp"
#include "SelectionManager.hpp"

#include "ParticleBuffer.hpp"

using namespace SceneTypes;
struct ChunkModifiedEvent;

class Scene : public eeng::SceneBase
{
public:

    bool init(const v2i& windowSize) override;

    void update(float time_s, float deltaTime_s) override;

    void renderUI() override;

    void render(float time_s, ShapeRendererPtr renderer) override;

    void destroy() override;

private:

    void event_loop();

    Editor::Context create_context();

    // Content management
    // void assign_entity_to_chunk(entt::registry& registry, entt::entity);
    // void destroy_chunk(const std::string& chunk_tag);
    void save_chunk(const std::string& chunk_id);
    void save_all_chunks();
    void load_chunk(const std::string& path);
    void unload_chunk(const std::string& chunk_id);
    void unload_all_chunks();

    // Saving and loading callbacks
    void OnSaveChunkToFileEvent(const SaveChunkToFileEvent& event);
    void OnSaveAllChunksToFileEvent(const SaveAllChunksToFileEvent& event);
    void OnUnloadChunkEvent(const UnloadChunkEvent& event);
    void OnUnloadAllChunksEvent(const UnloadAllChunksEvent& event);
    void OnLoadChunkFromFileEvent(const LoadChunkFromFileEvent& event);
    // State callbacks
    void OnSetGamePlayStateEvent(const SetGamePlayStateEvent& event);
    // Creation callbacks
    void OnCreateEntityEvent(const CreateEntityEvent& event);
    void OnDestroyEntitySelectionEvent(const DestroyEntitySelectionEvent& event);
    void OnCopyEntitySelectionEvent(const CopyEntitySelectionEvent& event);
    // Entity parenting callbacks
    void OnSetParentEntitySelectionEvent(const SetParentEntitySelectionEvent& event);
    void OnUnparentEntitySelectionEvent(const UnparentEntitySelectionEvent& event);
    // Add & remove component or script callbacks
    void OnAddComponentToEntitySelectionEvent(const AddComponentToEntitySelectionEvent& event);
    void OnRemoveComponentFromEntitySelectionEvent(const RemoveComponentFromEntitySelectionEvent& event);
    void OnAddScriptToEntitySelectionEvent(const AddScriptToEntitySelectionEvent& event);
    void OnRemoveScriptFromEntitySelectionEvent(const RemoveScriptFromEntitySelectionEvent& event);
    // Field update callbacks
    void OnChunkModifiedEvent(const ChunkModifiedEvent& event);
        // Script execution callbacks
    void OnRunScriptEvent(const RunScriptEvent& event);

    std::shared_ptr<entt::registry> registry{};
    std::shared_ptr<sol::state> lua{};
    std::shared_ptr<SceneGraph> scenegraph{};
    std::shared_ptr<ConditionalObserver> observer{};

    std::deque<Entity> entities_pending_destruction;

    linalg::v3f lightPos, eyePos;
    const float nearPlane = 1.0f, farPlane = 10.0f;
    int drawcallCount = 0;
    bool debug_render = false;

    m4f VP, P, V;


    // Particle buffer
    ParticleBuffer particleBuffer{};


    // (Editor) Command queue
    std::shared_ptr<Editor::CommandQueue> cmd_queue{};

    // TODO -> SG ???
    // entt::entity get_entity_parent(
    //     entt::entity entity);

    bool entity_valid(const Entity& entity);

    bool entity_parent_registered(const Entity& entity);

    void reparent_entity(const Entity& entity, const Entity& parent_entity);

    /// Sets entity parent in HeaderComponent and registers entity to scene graph
    void set_entity_header_parent(const Entity& entity, const Entity& entity_parent);

    /// Registers entity to scene graph using parent registered in HeaderComponent
    void register_entity(const Entity& entity);

    Entity create_empty_entity(const Entity& entity_hint);

    Entity create_entity(
        const std::string& chunk_tag,
        const std::string& name,
        const Entity& parent_entity,
        const Entity& entity_hint);

    // entt::entity create_entity_hint(
    //     entt::entity hint_entity,
    //     entt::entity parent_entity);

    // entt::entity create_entity(
    //     entt::entity parent_entity);

    void queue_entity_for_destruction(const Entity&);
    void destroy_pending_entities();

    ChunkRegistry chunk_registry{};

    GamePlayState play_state{ GamePlayState::Stop };
};

#endif