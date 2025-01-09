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
#include "ChunkRegistry.hpp"
#include "SceneGraph.hpp"
#include "CommandQueue.hpp"
#include "SelectionManager.hpp"

#include "ParticleBuffer.hpp"

using EntitySelection = Editor::SelectionManager<Entity>;

class Scene : public eeng::SceneBase
{
public:

    bool init(const v2i& windowSize) override;

    void update(float time_s, float deltaTime_s) override;

    void renderUI() override;

    void render(float time_s, ShapeRendererPtr renderer) override;

    void destroy() override;

    enum class GamePlayState : int { Play, Stop, Pause };

    struct SaveChunkToFileEvent { std::string chunk_tag; };
    struct SaveAllChunksToFileEvent { int placeholder; };
    struct UnloadChunkEvent { std::string chunk_tag; };
    struct LoadChunkFromFileEvent { std::string path; };
    struct SetGamePlayStateEvent { GamePlayState play_state; };
    struct CreateEntityEvent { Entity parent_entity; };
    struct DestroyEntityEvent { EntitySelection entity_selection; };
    struct CopyEntitySelectionEvent { EntitySelection entity_selection; };
    struct SetParentEntitySelectionEvent { EntitySelection entity_selection; };
    struct UnparentEntitySelectionEvent { EntitySelection entity_selection; };
    struct AddComponentToEntitySelectionEvent { entt::id_type component_id; EntitySelection entity_selection; };
    struct RemoveComponentFromEntitySelectionEvent { entt::id_type component_id; EntitySelection entity_selection; };
    struct AddScriptToEntitySelectionEvent { std::string script_path; EntitySelection entity_selection; };
    struct RemoveScriptFromEntitySelectionEvent { std::string script_path; EntitySelection entity_selection; };

    // Todo: Should not be hard coded obviously
    static inline const std::string script_dir = "../../LuaGame/lua/";
    static inline const std::string save_dir = "../../LuaGame/json/";
private:

    Editor::Context create_context();

    // Content management
    // void assign_entity_to_chunk(entt::registry& registry, entt::entity);
    // void destroy_chunk(const std::string& chunk_tag);
    void save_chunk(const std::string& chunk_tag);
    void save_all_chunks();
    void load_chunk(const std::string& path);

    void OnSaveChunkToFileEvent(const SaveChunkToFileEvent& event);
    void OnSaveAllChunksToFileEvent(const SaveAllChunksToFileEvent& event);
    void OnUnloadChunkEvent(const UnloadChunkEvent& event);
    void OnLoadChunkFromFileEvent(const LoadChunkFromFileEvent& event);
    void OnSetGamePlayStateEvent(const SetGamePlayStateEvent& event);
    void OnCreateEntityEvent(const CreateEntityEvent& event);
    void OnDestroyEntityEvent(const DestroyEntityEvent& event);
    void OnCopyEntitySelectionEvent(const CopyEntitySelectionEvent& event);
    void OnSetParentEntitySelectionEvent(const SetParentEntitySelectionEvent& event);
    void OnUnparentEntitySelectionEvent(const UnparentEntitySelectionEvent& event);
    void OnAddComponentToEntitySelectionEvent(const AddComponentToEntitySelectionEvent& event);
    void OnRemoveComponentFromEntitySelectionEvent(const RemoveComponentFromEntitySelectionEvent& event);
    void OnAddScriptToEntitySelectionEvent(const AddScriptToEntitySelectionEvent& event);
    void OnRemoveScriptFromEntitySelectionEvent(const RemoveScriptFromEntitySelectionEvent& event);

    std::shared_ptr<entt::registry> registry{};
    std::shared_ptr<sol::state> lua{};
    std::shared_ptr<SceneGraph> scenegraph{};

    std::deque<Entity> entities_pending_destruction;

    linalg::v3f lightPos, eyePos;
    const float nearPlane = 1.0f, farPlane = 10.0f;
    int drawcallCount = 0;
    bool debug_render = false;

    m4f VP, P, V;


    // Particle buffer
    ParticleBuffer particleBuffer{};

    // Observer
    ConditionalObserver observer{};

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