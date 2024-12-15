//
//  EditComponentCommand.cpp
//
//  Created by Carl Johan Gribel on 2024-10-14.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#include <iostream>
#include <cassert>
// #include "MetaSerialize.hpp"
#include "MetaClone.hpp"
#include "GuiCommands.hpp"
// #include "meta_aux.h"

// Used by Copy command
#include "SceneGraph.hpp"

namespace Editor {

    CreateEntityCommand::CreateEntityCommand(
        // const CreateEntityFunc&& create_func,
        // const DestroyEntityFunc&& destroy_func,
        entt::entity parent_entity,
        const Context& context) :
        // create_func(create_func),
        // destroy_func(destroy_func),
        parent_entity(parent_entity),
        context(context),
        display_name("Create Entity") {
    }

    void CreateEntityCommand::execute()
    {
        if (created_entity == entt::null)
        {
            created_entity = context.create_entity(parent_entity, entt::null);
            // created_entity = create_func(parent_entity, entt::null);
        }
        else
        {
            auto entity = context.create_entity(parent_entity, created_entity);
            // auto entity = create_func(parent_entity, created_entity);
            assert(entity == created_entity);
            created_entity = entity;
        }

        // std::cout << "CreateEntityCommand::execute() " << entt::to_integral(created_entity) << std::endl;
    }

    void CreateEntityCommand::undo()
    {
        assert(created_entity != entt::null);
        context.destroy_entity(created_entity);
        // destroy_func(created_entity);

        // std::cout << "CreateEntityCommand::undo() " << entt::to_integral(created_entity) << std::endl;
    }

    std::string CreateEntityCommand::get_name() const
    {
        return display_name;
    }

    // ------------------------------------------------------------------------

    DestroyEntityCommand::DestroyEntityCommand(
        entt::entity entity,
        const Context& context
        // const DestroyEntityFunc&& destroy_func
    ) :
        entity(entity),
        context(context)
        // destroy_func(destroy_func)
    {
        display_name = std::string("Destroy Entity ") + std::to_string(entt::to_integral(entity));
    }

    void DestroyEntityCommand::execute()
    {
        assert(entity != entt::null);
        entity_json = Meta::serialize_entities(&entity, 1, context.registry);
        context.destroy_entity(entity);
        // destroy_func(entity);
    }

    void DestroyEntityCommand::undo()
    {
        Meta::deserialize_entities(entity_json, context);

        entity_json = nlohmann::json{};
    }

    std::string DestroyEntityCommand::get_name() const
    {
        return display_name;
    }

    // ------------------------------------------------------------------------

    CopyEntityCommand::CopyEntityCommand(
        entt::entity entity,
        const Context& context) :
        entity_source(entity),
        context(context)
    {
        display_name = std::string("Copy Entity ") + std::to_string(entt::to_integral(entity));
    }

    void CopyEntityCommand::execute()
    {
        assert(entity_copy == entt::null);
        assert(entity_source != entt::null);
        assert(context.registry->valid(entity_source)); // context.entity_valid

        entity_copy = context.registry->create(); // context.create_empty_entity
        Editor::clone_entity(context.registry, entity_source, entity_copy);

        assert(context.can_register_entity(entity_copy));
        context.register_entity(entity_copy);

        // assert(entity != entt::null);
        // entity_json = Meta::serialize_entities(&entity, 1, context.registry);
        // destroy_func(entity);
    }

    void CopyEntityCommand::undo()
    {
        assert(entity_copy != entt::null);
        assert(context.registry->valid(entity_copy));

        context.destroy_entity(entity_copy);
        entity_copy = entt::null;

        // Meta::deserialize_entities(entity_json, context);

        // entity_json = nlohmann::json{};
    }

    std::string CopyEntityCommand::get_name() const
    {
        return display_name;
    }

    // ------------------------------------------------------------------------

    CopyEntityBranchCommand::CopyEntityBranchCommand(
        entt::entity entity,
        const Context& context) :
        root_entity(entity),
        context(context)
    {
        display_name = std::string("Copy Entity ") + std::to_string(entt::to_integral(entity));
    }

    void CopyEntityBranchCommand::execute()
    {
        assert(copied_entities.empty());
        assert(root_entity != entt::null);
        assert(context.registry->valid(root_entity));

        // iterate branch from root_entity -> source_entities
        assert(!context.scenegraph.expired());
        auto scenegraph = context.scenegraph.lock();
        // => get_branch_as_vector in SG
        scenegraph->tree.traverse_depthfirst(root_entity, [&](const entt::entity& entity, size_t) {
            source_entities.push_back(entity);
        });

        // Create copies
        for (auto& entity : source_entities)
        {
            auto entity_copy = context.registry->create(); // => context.create_empty_entity
            Editor::clone_entity(context.registry, entity, entity_copy);
            copied_entities.push_back(entity_copy);
        }

        // ParentMap
        std::unordered_map<uint32_t, uint32_t> parent_index_map;
        for (int i = 0; i < source_entities.size(); i++)
        {
            auto parent_tree_index = scenegraph->tree.get_parent_index(source_entities[i]); // => function in SG
            // -> parent_index within source_entities
            // map i -> parent_index
        }

        // Reparent
        // get_parent_of
        for (int i = 0; i < source_entities.size(); i++)
        {
            // set parent of copied_entities[i] to copied_entities[ ParentMap[i] ]
            // set_parent can be a SG function
        }

        // assert(context.can_register_entity(entity_copy));
        // context.register_entity(entity_copy);
    }

    // Destroy all copied_entities. Order does not matter since we are not using commands.
    void CopyEntityBranchCommand::undo()
    {
        // assert(entity_copy != entt::null);
        // assert(context.registry->valid(entity_copy));

        // context.destroy_entity(entity_copy);
        // entity_copy = entt::null;
    }

    std::string CopyEntityBranchCommand::get_name() const
    {
        return display_name;
    }

} // namespace Editor