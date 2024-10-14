//
//  Command.hpp
//
//  Created by Carl Johan Gribel on 2024-10-14.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef Command_hpp
#define Command_hpp

#include <vector>
#include "Command.hpp"

namespace Editor {

    class CommandQueue
    {
        std::vector<CommandPtr> queue;
        int current_index = -1;

        void add(CommandPtr command)
        {
            queue.push_back(command);
        }

        void execute_pending()
        {
            //
        }

        void clear_executed()
        {
            //
        }
    };

} // namespace Editor

#endif // Command_hpp
