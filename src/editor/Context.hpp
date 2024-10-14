//
//  Context.hpp
//
//  Created by Carl Johan Gribel on 2024-10-15.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef Context_hpp
#define Context_hpp

//#include <memory> // shared_ptr
#include <entt/fwd.hpp>
#include <sol/forward.hpp>

namespace Editor {

    struct Context
    {
        sol::state* lua;            // shared_ptr ?
        entt::registry* registry;   // shared_ptr ?

        // shared_ptr<Scene>
        // shared_ptr<Resources>
    };

} // namespace Editor

#endif // Command_hpp
