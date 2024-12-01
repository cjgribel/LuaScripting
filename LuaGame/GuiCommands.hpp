//
//  EditComponentCommand.hpp
//
//  Created by Carl Johan Gribel on 2024-12-01.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef GuiCommands_hpp
#define GuiCommands_hpp

#include <entt/entt.hpp>
// #include "Scene.hpp"
#include "Command.hpp"
#include "EditComponentCommand.hpp"

namespace Editor {

    class CreateEntityCommand : public Command
    {
        //std::weak_ptr<Scene> scene;
        std::weak_ptr<entt::registry>   registry;
        entt::entity entity = entt::null;;
        std::string display_name;

        using CreateFunc = std::function<entt::entity(entt::entity, const std::string&, const std::string&)>;
        CreateFunc create_func;

        friend class Builder;

    public:
        CreateEntityCommand(const CreateFunc&& create_func)
            : create_func(create_func)
        {

        }

        void execute() override;

        void undo() override;

        std::string get_name() const override;
    };

    class CreateEntityCommandBuilder
    {
        CreateEntityCommand command;

    public:
        CreateEntityCommandBuilder& registry(std::weak_ptr<entt::registry> registry) { return *this; }

        CreateEntityCommandBuilder& entity(entt::entity entity) { return *this; }

        CreateEntityCommandBuilder& reset() { return *this; }

        CreateEntityCommand build() { return command; }
    };
} // namespace Editor

#endif /* EditComponentCommand_hpp */
