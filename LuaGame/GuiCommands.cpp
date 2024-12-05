//
//  EditComponentCommand.cpp
//
//  Created by Carl Johan Gribel on 2024-10-14.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#include <iostream>
#include <cassert>
//#include "MetaSerialize.hpp"
#include "GuiCommands.hpp"
// #include "meta_aux.h"

namespace Editor {

    namespace
    {
        entt::entity create_entity_from_hint(entt::entity entity_parent, entt::entity entity_hint)
        {
            return entt::entity{};
        }
    }

    void CreateEntityCommand::execute()
    {
        if (created_entity == entt::null)
        {
            created_entity = create_func(parent_entity, entt::null);
        }
        else
        {
            auto entity = create_func(parent_entity, created_entity);
            assert(entity == created_entity);
            created_entity = entity;
        }

        // std::cout << "CreateEntityCommand::execute() " << entt::to_integral(created_entity) << std::endl;
    }

    void CreateEntityCommand::undo()
    {
        assert(created_entity != entt::null);
        destroy_func(created_entity);

        // std::cout << "CreateEntityCommand::undo() " << entt::to_integral(created_entity) << std::endl;
    }

    std::string CreateEntityCommand::get_name() const
    {
        return display_name;
    }

    DestroyEntityCommand::DestroyEntityCommand(
        entt::entity entity,
        const Context& context,
        const DestroyEntityFunc&& destroy_func) :
        entity(entity),
        context(context),
        destroy_func(destroy_func)
    {
        display_name = std::string("Destroy Entity ") + std::to_string(entt::to_integral(entity));
    }

    void DestroyEntityCommand::execute()
    {
        // 1. Serialize entity -> nlohmann::json

        assert(entity != entt::null);
        entity_json = Meta::serialize_entity(entity, context.registry);

        // 2. 

        destroy_func(entity);

        std::cout << entity_json.dump() << std::endl;
    }

    void DestroyEntityCommand::undo()
    {
        // 1. Recreate entity
        // Done by deserialize_entity (entity stored in json) --> Similar to CreateEntityCommand::execute()

        Meta::deserialize_entity(entity_json, context);
        // ^ Entity not added to SG (parent needed)

        entity_json = nlohmann::json {};
    }

    std::string DestroyEntityCommand::get_name() const
    {
        return display_name;
    }

} // namespace Editor