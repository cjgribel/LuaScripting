//
//  EditComponentCommand.hpp
//
//  Created by Carl Johan Gribel on 2024-10-14.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef EditComponentCommand_hpp
#define EditComponentCommand_hpp

#include "Command.hpp"
#include <entt/entt.hpp>

namespace Editor {

    class EditComponentCommand : public Command
    {
        // entt::registry <- later

        // entt::entity entity; <- later ?
        
        // entt::id_type comp_id;
        // std::deque<MetaEntry> meta_path;
        // entt::meta_any old_value, new_value;
        // bool is_used = false;

    public:
        EditComponentCommand()
        {

        }

        void execute() override
        {

        }

        void undo() override
        {

        }
    };

} // namespace Editor

#endif /* EditComponentCommand_hpp */
