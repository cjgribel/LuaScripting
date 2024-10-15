//
//  Command.hpp
//
//  Created by Carl Johan Gribel on 2024-10-14.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef Command_hpp
#define Command_hpp

#include <memory>
#include <entt/entt.hpp>
#include "Context.hpp"

namespace Editor {

    struct Selection
    {
        entt::entity entity;
    };

    // Is this a pure Editor thingy?
    class Command
    {
    public:
        virtual void execute(/*Context& context, const Selection& selection*/) = 0;
        virtual void undo() = 0;
    };

    using CommandPtr = std::unique_ptr<Command>;

} // namespace Editor

#endif // Command_hpp
