//
//  EditComponentCommand.hpp
//
//  Created by Carl Johan Gribel on 2024-10-14.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef EditComponentCommand_hpp
#define EditComponentCommand_hpp

#include <entt/entt.hpp>
#include "Command.hpp"

namespace Editor {

    struct MetaPath
    {
        struct Entry
        {
            enum class Type : int { None, Data, Index, Key } type = Type::None;

            entt::id_type data_id;  // entering data field
            size_t index;           // entering seq. container index
            entt::meta_any key_any; // entering assoc. container key

            std::string name;       // for debugging
        };
        std::vector<Entry> entries;
    };

    class ComponentCommand : public Command
    {
        std::weak_ptr<entt::registry> registry;
        entt::entity entity = entt::null;;
        entt::id_type component_id = 0;
        MetaPath meta_path{};

        entt::meta_any prev_value{}, new_value{};

        friend class ComponentCommandBuilder;

    public:
        ComponentCommand()
        {

        }

        // During CMD CREATION vs CMD EXECUTION
        void execute() override
        {
            // if (auto registry = registry_.lock()) {
            //     // Perform action on lockedRegistry
            // }
            // else {
            //     // Handle case where the registry is no longer alive
            //     std::cerr << "Registry is no longer available. Command cannot be executed.\n";
            // }
        }

        void undo() override
        {

        }
    };

    class ComponentCommandBuilder
    {
        ComponentCommand command;

    public:
        auto& registry(std::weak_ptr<entt::registry> registry) { command.registry = registry; return *this; }
        auto& entity(entt::entity entity) { command.entity = entity; return *this; }
        auto& component(entt::id_type id) { command.component_id = id; return *this; }
        auto& prev_value(const entt::meta_any& value) { command.old_value = value; return *this; }
        auto& new_value(const entt::meta_any& value) { command.new_value = value; return *this; }

        auto& push_path_data(entt::id_type id)
        {
            command.meta_path.entries.push_back(
                MetaPath::Entry{ .type = MetaPath::Entry::Type::Data, .data_id = id }
            );
            return *this;
        }

        auto& push_path_index(size_t index)
        {
            command.meta_path.entries.push_back(
                MetaPath::Entry{ .type = MetaPath::Entry::Type::Index, .index = index }
            );
            return *this;
        }

        auto& push_path_key(const entt::meta_any& key_any)
        {
            command.meta_path.entries.push_back(
                MetaPath::Entry{ .type = MetaPath::Entry::Type::Index, .key_any = key_any }
            );
            return *this;
        }

        void pop_path() { command.meta_path.entries.pop_back(); }

        auto& clear()
        {
            assert(!command.meta_path.entries.size() && "Meta path not empty when clearing");
            command = ComponentCommand{}; 
            return *this;
        }

        // Validate and build the final EditComponentCommand
        auto build()
        {
            assert(!command.registry.expired() && "registry pointer expired");
            assert(command.entity != entt::null && "entity invalid");
            assert(command.component_id != 0 && "component id invalid");

            assert(command.prev_value);
            assert(command.new_value);

            assert(!command.meta_path.entries.empty() && "Meta path empty");
            // TODO: first must be Data; Index & Key must be followed by Data ...

            return command;
        }
    };

} // namespace Editor

#endif /* EditComponentCommand_hpp */
