//
//  Command.hpp
//
//  Created by Carl Johan Gribel on 2024-10-14.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef CommandQueue_hpp
#define CommandQueue_hpp

#include <vector>
#include "Command.hpp"

namespace Editor {

    class CommandFactory {
    public:
        template <typename CommandType, typename... Args>
        static std::unique_ptr<Command> Create(Args&&... args) {
            return std::make_unique<CommandType>(std::forward<Args>(args)...);
        }
    };

    class CommandQueue
    {
        // std::vector<CommandPtr> queue;
        std::vector<std::unique_ptr<Command>> queue;

        int current_index = -1;

        void add(CommandPtr& command)
        {
            queue.push_back(std::move(command));
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
