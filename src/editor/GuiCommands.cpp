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

    // --- CopyEntityCommand (REMOVE) --------------------------------------------------

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

        entity_copy = context.create_empty_entity(entt::null); // context.registry->create(); // context.create_empty_entity
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

    // --- CopyEntityBranchCommand --------------------------------------------

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
        //assert(copied_entities.empty());
        assert(root_entity != entt::null);
        assert(context.entity_valid(root_entity));

        // Obtain entity branch
        assert(!context.scenegraph.expired());
        auto scenegraph = context.scenegraph.lock();
        source_entities = scenegraph->get_branch_topdown(root_entity);

        // Hints for copied entites:
        // either no hints (not a redo) or previously copied & destroyed entities (undo-redo)
        auto entity_hints = copied_entities;
        if (entity_hints.empty()) entity_hints.assign(source_entities.size(), entt::null);
        copied_entities.clear();

        // Create copies top-down and resolve new parents
        for (int i = 0; i < source_entities.size(); i++)
        {
            auto& source_entity = source_entities[i];
            
            // Copy entity
            entt::entity entity_copy = context.create_empty_entity(entity_hints[i]);
            Editor::clone_entity(context.registry, source_entity, entity_copy);

            // Update entity parent for all except the root entity
            if (source_entity != root_entity)
            {
                // Find index of source entity's parent within the branch
                auto source_entity_parent = scenegraph->get_parent(source_entity);
                int source_entity_parent_index = -1;
                // The parent is located in the [0, i[ range
                for (int j = 0; j < i; j++)
                {
                    if (source_entities[j] == source_entity_parent) source_entity_parent_index = j;
                }
                assert(source_entity_parent_index > -1);

                // Use index to obtain parent of the copied entity
                auto& new_parent = copied_entities[source_entity_parent_index];
                // Now set new parent in the entity header
                context.set_entity_header_parent(entity_copy, new_parent);
            }

            // Register copied entity
            assert(context.can_register_entity(entity_copy));
            context.register_entity(entity_copy);

            copied_entities.push_back(entity_copy);
        }
    }

    void CopyEntityBranchCommand::undo()
    {
        // Destroy bottom-up
        for (auto entity_it = copied_entities.rbegin();
            entity_it != copied_entities.rend();
            entity_it++)
        {
            context.destroy_entity(*entity_it);
        }
    }

    std::string CopyEntityBranchCommand::get_name() const
    {
        return display_name;
    }

    // --- ReparentEntityBranchCommand --------------------------------------------

    ReparentEntityBranchCommand::ReparentEntityBranchCommand(
        entt::entity entity,
        entt::entity parent_entity,
        const Context& context) :
        entity(entity),
        new_parent_entity(parent_entity),
        context(context)
    {
        display_name = std::string("Reparent Entity ")
            + std::to_string(entt::to_integral(entity))
            + " to "
            + std::to_string(entt::to_integral(parent_entity));
    }

    void ReparentEntityBranchCommand::execute()
    {
        assert(!context.scenegraph.expired());
        auto scenegraph = context.scenegraph.lock();

        if (scenegraph->is_root(entity))
            prev_parent_entity = entt::null;
        else
            prev_parent_entity = scenegraph->get_parent(entity);

        context.reparent_entity(entity, new_parent_entity);
    }

    void ReparentEntityBranchCommand::undo()
    {
        // assert(!context.scenegraph.expired());
        // auto scenegraph = context.scenegraph.lock();

        context.reparent_entity(entity, prev_parent_entity);
    }

    std::string ReparentEntityBranchCommand::get_name() const
    {
        return display_name;
    }

    // --- UnparentEntityBranchCommand --------------------------------------------

    // UnparentEntityBranchCommand::UnparentEntityBranchCommand(
    //     entt::entity entity,
    //     entt::entity parent_entity,
    //     const Context& context) :
    //     entity(entity),
    //     // new_parent_entity(parent_entity),
    //     context(context)
    // {
    //     display_name = std::string("Unparent Entity ") + std::to_string(entt::to_integral(entity));
    // }

    // void UnparentEntityBranchCommand::execute()
    // {
    //     assert(!context.scenegraph.expired());
    //     auto scenegraph = context.scenegraph.lock();

    //     if (scenegraph->is_root(entity))
    //     {
    //         return;
    //         // prev_parent_entity = entt::null;
    //     }
    //     // else
    //         prev_parent_entity = scenegraph->get_parent(entity);

    //     // scenegraph->reparent(entity, new_parent_entity);
    //     scenegraph->unparent(entity);
    // }

    // void UnparentEntityBranchCommand::undo()
    // {
    //     assert(!context.scenegraph.expired());
    //     auto scenegraph = context.scenegraph.lock();

    //     if (prev_parent_entity == entt::null)
    //     {
    //         assert(scenegraph->is_root(entity));
    //         // scenegraph->unparent(entity);
    //         return;
    //     }
    //     // else
    //         scenegraph->reparent(entity, prev_parent_entity);
    // }

    // std::string UnparentEntityBranchCommand::get_name() const
    // {
    //     return display_name;
    // }




                // if (auto prop = type.prop("is_component"_hs); prop && prop.value().cast<bool>()) {
                //     std::cout << "Component type: " << type.name() << "\n";
                //     // Add `type.name()` or similar to the combo box
                // }

                // storage->contains(entity) <- Check first

                // Note: "emplace"_hs is already registered for a Lua-specific 

                // -> AddComponentEvent

                // Create & add component

                // push (deserialize_entity)
                // Without last arg
                //      context.registry->storage(id)->push(entity, any.data());
                // Note needed since since push will default-initialize
                //      entt::meta_any any = meta_type.construct();

                // Remove
                //auto storage = inspector.context.registry->storage(id);
                //storage->remove(entity);


} // namespace Editor