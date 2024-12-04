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

#include "MetaSerialize.hpp"

namespace Editor {

    class CreateEntityCommand : public Command
    {
        entt::entity created_entity = entt::null;
        entt::entity parent_entity = entt::null;
        std::string display_name = "Create Entity";

        using CreateEntityFunc = std::function<entt::entity(entt::entity, entt::entity)>;
        using DestroyEntityFunc = std::function<void(entt::entity)>;

        CreateEntityFunc create_func;
        DestroyEntityFunc destroy_func;

    public:
        CreateEntityCommand(
            const CreateEntityFunc&& create_func,
            const DestroyEntityFunc&& destroy_func,
            entt::entity parent_entity) :
            create_func(create_func),
            destroy_func(destroy_func),
            parent_entity(parent_entity) { }

        void execute() override;

        void undo() override;

        std::string get_name() const override;
    };

    class DestroyEntityCommand : public Command
    {
        //std::weak_ptr<Scene> scene;
        // std::weak_ptr<entt::registry>   registry;
        entt::entity entity_destroyed = entt::null;
        nlohmann::json entity_serialized;
        std::string display_name = "Destroy Entity";

        using CreateEntityFunc = std::function<entt::entity(entt::entity, entt::entity)>;
        using DestroyEntityFunc = std::function<void(entt::entity)>;

        CreateEntityFunc create_func;
        DestroyEntityFunc destroy_func;

    public:
        DestroyEntityCommand(const CreateEntityFunc&& create_func)
            : create_func(create_func)
        { }

        void execute() override;

        void undo() override;

        std::string get_name() const override;
    };

} // namespace Editor

#endif /* EditComponentCommand_hpp */
