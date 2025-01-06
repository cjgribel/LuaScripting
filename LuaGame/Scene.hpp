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

using EntitySelection = Editor::SelectionManager<entt::entity>;

class Scene : public eeng::SceneBase
{
public:

    bool init(const v2i& windowSize) override;

    void update(float time_s, float deltaTime_s) override;

    void renderUI() override;

    void render(float time_s, ShapeRendererPtr renderer) override;

    void destroy() override;

    enum class GamePlayState : int { Play, Stop, Pause };

    struct SetGamePlayStateEvent { GamePlayState play_state; };
    struct UnloadChunkEvent { std::string chunk_tag; };
    struct LoadChunkFromFileEvent { std::string path; };

    struct CreateEntityEvent { entt::entity parent_entity; };
    struct DestroyEntityEvent { EntitySelection entity_selection; };
    // struct CopyEntityEvent { entt::entity entity; }; // EntitySelection entity_selection;
    struct CopyEntitySelectionEvent { EntitySelection entity_selection; }; // TMP
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

    void OnSetGamePlayStateEvent(const SetGamePlayStateEvent& event);
    void OnUnloadChunkEvent(const UnloadChunkEvent& event);
    void OnLoadChunkFromFileEvent(const LoadChunkFromFileEvent& event);
    void OnCreateEntityEvent(const CreateEntityEvent& event);
    void OnDestroyEntityEvent(const DestroyEntityEvent& event);
    // void OnCopyEntityEvent(const CopyEntityEvent& event);
    void OnCopyEntitySelectionEvent(const CopyEntitySelectionEvent& event); // TMP
    void OnSetParentEntitySelectionEvent(const SetParentEntitySelectionEvent& event);
    void OnUnparentEntitySelectionEvent(const UnparentEntitySelectionEvent& event);

    void OnAddComponentToEntitySelectionEvent(const AddComponentToEntitySelectionEvent& event);
    void OnRemoveComponentFromEntitySelectionEvent(const RemoveComponentFromEntitySelectionEvent& event);

    void OnAddScriptToEntitySelectionEvent(const AddScriptToEntitySelectionEvent& event);
    void OnRemoveScriptFromEntitySelectionEvent(const RemoveScriptFromEntitySelectionEvent& event);

    std::shared_ptr<entt::registry> registry{};
    std::shared_ptr<sol::state> lua{};
    std::shared_ptr<SceneGraph> scenegraph{};

    std::deque<entt::entity> entities_pending_destruction;

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

    bool entity_valid(entt::entity entity);

    bool entity_parent_registered(
        entt::entity entity);

    void reparent_entity(entt::entity entity, entt::entity parent_entity);

    /// Sets entity parent in HeaderComponent and registers entity to scene graph
    void set_entity_header_parent(entt::entity entity, entt::entity entity_parent);

    /// Registers entity to scene graph using parent registered in HeaderComponent
    void register_entity(
        entt::entity entity);

    entt::entity create_empty_entity(entt::entity entity_hint);

    entt::entity create_entity(
        const std::string& chunk_tag,
        const std::string& name,
        entt::entity parent_entity,
        entt::entity entity_hint);

    // entt::entity create_entity_hint(
    //     entt::entity hint_entity,
    //     entt::entity parent_entity);

    // entt::entity create_entity(
    //     entt::entity parent_entity);

    void queue_entity_for_destruction(entt::entity entity);
    void destroy_pending_entities();

    ChunkRegistry chunk_registry{};

    GamePlayState play_state{ GamePlayState::Stop };
};

#endif