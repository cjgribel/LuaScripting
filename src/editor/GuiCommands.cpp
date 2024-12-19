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

        entity_copy = context.create_empty_entity(); // context.registry->create(); // context.create_empty_entity
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
        assert(copied_entities.empty());
        assert(root_entity != entt::null);
        assert(context.registry->valid(root_entity));

        // iterate branch from root_entity -> source_entities
        assert(!context.scenegraph.expired());
        auto scenegraph = context.scenegraph.lock();
        // => get_branch_as_vector in SG
        // scenegraph->tree.traverse_depthfirst(root_entity, [&](const entt::entity& entity, size_t) {
        //     source_entities.push_back(entity);
        //     });
        // ->
        source_entities = scenegraph->get_branch_in_level_order(root_entity); // leve-order with leafs in the back
        //source_entities.insert my_vector{ my_stack._Get_container().begin(), my_stack._Get_container().end() };

        // Create copies
        for (auto& entity : source_entities)
        {
            // Copy entity
            auto entity_copy = context.registry->create(); // => context.create_empty_entity
            Editor::clone_entity(context.registry, entity, entity_copy);

            // Determine parent of copied entity
            // if entity == root_entity: Register entity_copy to the same parent
            // else: Find index of entity parent inside source_entities, 
            //      and use this index to fetch entity_parent of entity_copy in copied_entities
            //      (should have been registered at this point since we are traversing in level order)
            //      Register entity_copy to parent_copy

            // Register copied entity
            assert(context.can_register_entity(entity_copy));
            context.register_entity(entity_copy);

            // Not needed
            copied_entities.push_back(entity_copy);
        }
        // DONE HERE?

        // Entity index to parent index map
        std::unordered_map<uint32_t, uint32_t> parent_index_map;
        for (int i = 0; i < source_entities.size(); i++)
        {
            auto parent_entity = scenegraph->tree.get_parent(source_entities[i]); // => function in SG
            auto parent_entity_it = std::find(source_entities.begin(), source_entities.end(), parent_entity);
            assert(parent_entity_it != source_entities.end());
            auto parent_entity_index = std::distance(source_entities.begin(), parent_entity_it);

            parent_index_map[i] = parent_entity_index;
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
            prev_parent_entity = context.get_parent(entity); // scenegraph->get_parent(entity); // context.get_parent?

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


} // namespace Editor