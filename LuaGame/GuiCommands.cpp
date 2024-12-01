//
//  EditComponentCommand.cpp
//
//  Created by Carl Johan Gribel on 2024-10-14.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#include <iostream>
#include <cassert>
//#include "MetaSerialize.hpp"
#include "GuiCommands.hpp"

namespace Editor {

    void CreateEntityCommand::execute()
    {

    }

    void CreateEntityCommand::undo()
    {

    }

    std::string CreateEntityCommand::get_name() const
    {
        return display_name;
    }

} // namespace Editor