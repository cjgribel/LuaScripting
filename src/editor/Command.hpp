//
//  Command.hpp
//
//  Created by Carl Johan Gribel on 2024-10-14.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef Command_hpp
#define Command_hpp

#include <memory>

namespace Editor {

    class Command
    {
    public:
        virtual void execute() = 0;
        virtual void undo() = 0;
    };

    using CommandPtr = std::unique_ptr<Command>;

} // namespace Editor

#endif // Command_hpp
