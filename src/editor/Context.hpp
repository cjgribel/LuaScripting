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

namespace Editor {

    using EntityParentRegisteredFunc = std::function<bool(entt::entity)>;
    using RegisterEntityFunc = std::function<void(entt::entity)>;

    struct Context
    {

        std::shared_ptr<entt::registry> registry;
        std::shared_ptr<sol::state> lua;

        EntityParentRegisteredFunc entity_parent_registered_func;
        RegisterEntityFunc register_entity_func;

        // shared_ptr<Scene>
        // shared_ptr<Resources>
    };

} // namespace Editor

#endif // Command_hpp
