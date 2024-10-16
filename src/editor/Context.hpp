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

    struct Context
    {
        std::shared_ptr<entt::registry> registry;
        std::shared_ptr<sol::state> lua;

        // shared_ptr<Scene>
        // shared_ptr<Resources>
    };

} // namespace Editor

#endif // Command_hpp
