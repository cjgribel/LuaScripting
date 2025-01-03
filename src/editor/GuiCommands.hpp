//
//  EditComponentCommand.hpp
//
//  Created by Carl Johan Gribel on 2024-12-01.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef GuiCommands_hpp
#define GuiCommands_hpp

#include <deque>
#include <entt/entt.hpp>
// #include "Scene.hpp"
#include "Command.hpp"
#include "EditComponentCommand.hpp"

#include "MetaSerialize.hpp"
#include "Context.hpp"

namespace Editor {

    class CreateEntityCommand : public Command
    {
        entt::entity created_entity = entt::null;
        entt::entity parent_entity = entt::null;
        Context context;
        std::string display_name;

        // using CreateEntityFunc = std::function<entt::entity(entt::entity, entt::entity)>;
        // using DestroyEntityFunc = std::function<void(entt::entity)>;

        // CreateEntityFunc create_func;
        // DestroyEntityFunc destroy_func;

    public:
        CreateEntityCommand(
            // const CreateEntityFunc&& create_func,
            // const DestroyEntityFunc&& destroy_func,
            entt::entity parent_entity,
            const Context& context);

        void execute() override;

        void undo() override;

        std::string get_name() const override;
    };

    // ------------------------------------------------------------------------

    class DestroyEntityCommand : public Command
    {
        entt::entity entity = entt::null;
        nlohmann::json entity_json{};
        Context context;
        std::string display_name;

        // using RegisterEntityFunc = std::function<void(entt::entity, entt::entity)>;
        // using DestroyEntityFunc = std::function<void(entt::entity)>;

        // DestroyEntityFunc destroy_func;

    public:
        DestroyEntityCommand(
            entt::entity entity,
            const Context& context
            // const DestroyEntityFunc&& destroy_func
        );

        void execute() override;

        void undo() override;

        std::string get_name() const override;
    };

    // --- CopyEntityCommand --------------------------------------------------

    class CopyEntityCommand : public Command
    {
        entt::entity entity_source = entt::null;
        entt::entity entity_copy = entt::null;
        //     nlohmann::json entity_json{};
        Context context;
        std::string display_name;

        using CreateEntityFunc = std::function<entt::entity(entt::entity, entt::entity)>;
        using DestroyEntityFunc = std::function<void(entt::entity)>;
        //     using RegisterEntityFunc = std::function<void(entt::entity, entt::entity)>;
        //     using DestroyEntityFunc = std::function<void(entt::entity)>;

        //     DestroyEntityFunc destroy_func;

    public:
        CopyEntityCommand(
            entt::entity entity,
            const Context& context);
        //         const DestroyEntityFunc&& destroy_func);

        void execute() override;

        void undo() override;

        std::string get_name() const override;
    };

    // --- CopyEntityBranchCommand --------------------------------------------

    class CopyEntityBranchCommand : public Command
    {
        entt::entity root_entity = entt::null;
        std::deque<entt::entity> source_entities; // top-down
        std::deque<entt::entity> copied_entities; // top-down
        Context context;
        std::string display_name;

    public:
        CopyEntityBranchCommand(
            entt::entity entity,
            const Context& context);

        void execute() override;

        void undo() override;

        std::string get_name() const override;
    };

    // --- ReparentEntityBranchCommand ----------------------------------------

    class ReparentEntityBranchCommand : public Command
    {
        entt::entity entity = entt::null;
        entt::entity prev_parent_entity = entt::null;
        entt::entity new_parent_entity = entt::null;
        Context context;
        std::string display_name;

    public:
        ReparentEntityBranchCommand(
            entt::entity entity,
            entt::entity parent_entity,
            const Context& context);

        void execute() override;

        void undo() override;

        std::string get_name() const override;
    };

        // --- UnparentEntityBranchCommand ----------------------------------------

    // class UnparentEntityBranchCommand : public Command
    // {
    //     entt::entity entity = entt::null;
    //     entt::entity prev_parent_entity = entt::null;
    //     // entt::entity new_parent_entity = entt::null;
    //     Context context;
    //     std::string display_name;

    // public:
    //     UnparentEntityBranchCommand(
    //         entt::entity entity,
    //         entt::entity parent_entity,
    //         const Context& context);

    //     void execute() override;

    //     void undo() override;

    //     std::string get_name() const override;
    // };

} // namespace Editor

#endif /* EditComponentCommand_hpp */
