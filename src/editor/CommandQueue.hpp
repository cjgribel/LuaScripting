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

        int current_index = 0; // Index awaiting execution

    public:
        void add(CommandPtr&& command)
        {
            remove_pending();
            queue.push_back(std::move(command));
        }

        size_t size() { return queue.size(); }

        size_t get_current_index()
        {
            return current_index;
        }

        bool is_executed(size_t index)
        {
            return index < current_index;
        }

        std::string get_name(size_t index)
        {
            assert(index >= 0 && index < queue.size());
            return queue[index]->get_name();
        }

        bool commands_pending()
        {
            return current_index >= 0 && current_index < queue.size();
        }

        void remove_pending()
        {
            if (!commands_pending()) return;
            queue.erase(queue.begin() + current_index, queue.end());
        }

        void execute_next()
        {
            if (!commands_pending()) return;
            queue[current_index++]->execute();
        }

        void execute_pending()
        {
            while (commands_pending()) execute_next();
        }

        bool can_undo()
        {
            assert(current_index <= queue.size());
            return current_index > 0;
        }

        void undo_last()
        {
            if (!can_undo()) return;
            queue[--current_index]->undo();
        }

        void clear_executed()
        {
            //
        }
    };

} // namespace Editor

#endif // Command_hpp
