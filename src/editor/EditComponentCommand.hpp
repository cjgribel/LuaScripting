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
        struct MetaEntry
        {
            enum class Type : int { None, Data, Index, Key } type = Type::None;
            entt::id_type data_id;  // enter data field
            size_t index;           // enter seq. container index
            entt::meta_any key_any; // enter assoc. container key

            std::string name = "(no name)";
        };

               // entt::registry <- later
        // entt::entity entity; <- later ?

        // MetaCommandDescriptor
         entt::id_type comp_id;
         std::vector<MetaEntry> meta_path;
         entt::meta_any old_value, new_value;
        // bool is_used = false;

    public:
        EditComponentCommand()
        {

        }

        // During CMD CREATION vs CMD EXECUTION
        void execute() override
        {

        }

        void undo() override
        {

        }
    };

} // namespace Editor

#endif /* EditComponentCommand_hpp */
