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
    using DestroyEntityFunc = std::function<void(entt::entity)>;
    using CanRegisterEntityFunc = std::function<bool(entt::entity)>;
    using RegisterEntityFunc = std::function<void(entt::entity)>;

    struct Context
    {
        std::shared_ptr<entt::registry> registry;
        std::shared_ptr<sol::state> lua;
        std::weak_ptr<SceneGraph> scenegraph;

        // shared_ptr<Scene>
        CreateEntityFunc create_entity;
        // + create_empty_entity ???
        DestroyEntityFunc destroy_entity;
        CanRegisterEntityFunc can_register_entity;
        RegisterEntityFunc register_entity;
        // + entity_valid ???

        // shared_ptr<Resources>
    };

} // namespace Editor

#endif // Command_hpp
