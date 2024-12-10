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

} // namespace Editor