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

namespace Editor {

    void CreateEntityCommand::execute()
    {
        assert(created_entity == entt::null);
        created_entity = create_func(parent_entity);
    }

    void CreateEntityCommand::undo()
    {
        assert(created_entity != entt::null);
        destroy_func(created_entity);
        created_entity = entt::null;
    }

    std::string CreateEntityCommand::get_name() const
    {
        return display_name;
    }

    void DestroyEntityCommand::execute()
    {
        // queue_entity_for_destruction(event.entity);
    }

    void DestroyEntityCommand::undo()
    {
        // assert(entity_json.contains("entity"));
        // entt::entity entity_hint = entity_json["entity"].get<entt::entity>();
        // assert(!context.registry->valid(entity_hint));
        // auto entity = context.registry->create(entity_hint);
        // assert(entity_hint == entity);
    }

    std::string DestroyEntityCommand::get_name() const
    {
        return display_name;
    }

} // namespace Editor