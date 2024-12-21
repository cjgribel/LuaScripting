//
//  Context.hpp
//
//  Created by Carl Johan Gribel on 2024-10-15.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef Context_hpp
#define Context_hpp

#include <memory>
#include <entt/fwd.hpp>
#include <sol/forward.hpp>
//#include "SceneGraph.hpp"
class SceneGraph;

namespace Editor {

    using CreateEntityFunc = std::function<entt::entity(entt::entity, entt::entity)>;
    using CreateEmptyEntityFunc = std::function<entt::entity()>;
    using DestroyEntityFunc = std::function<void(entt::entity)>;
    using CanRegisterEntityFunc = std::function<bool(entt::entity)>;
    using RegisterEntityFunc = std::function<void(entt::entity)>;
    using ReparentEntityFunc = std::function<void(entt::entity, entt::entity)>;
    using SetEntityHeaderParentFunc = std::function<void(entt::entity, entt::entity)>;
    // using GetParentFunc = std::function<entt::entity(entt::entity)>;
    using EntityValidFunc = std::function<bool(entt::entity)>;

    struct Context
    {
        std::shared_ptr<entt::registry> registry;
        std::shared_ptr<sol::state> lua;
        std::weak_ptr<SceneGraph> scenegraph;

        // shared_ptr<Scene>
        CreateEntityFunc create_entity;
        CreateEmptyEntityFunc create_empty_entity;
        DestroyEntityFunc destroy_entity;
        CanRegisterEntityFunc can_register_entity;
        RegisterEntityFunc register_entity;
        ReparentEntityFunc reparent_entity;
        SetEntityHeaderParentFunc set_entity_header_parent;
        // GetParentFunc get_parent;
        EntityValidFunc entity_valid;

        // shared_ptr<Resources>
    };

} // namespace Editor

#endif // Command_hpp
