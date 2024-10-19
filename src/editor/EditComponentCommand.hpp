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
            int index{ -1 };         // entering seq. container index
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

        std::string display_name;

        friend class ComponentCommandBuilder;

        void traverse_and_set_meta_type(entt::meta_any& value_any);

    public:
        ComponentCommand()
        {

        }

        void execute() override
        {
            traverse_and_set_meta_type(new_value);
        }

        void undo() override
        {
            traverse_and_set_meta_type(prev_value);
        }

        std::string get_name() override
        {
            return meta_path.entries.back().name;
            // return display_name;
        }
    };

    class ComponentCommandBuilder
    {
        ComponentCommand command;

    public:
        auto& registry(std::weak_ptr<entt::registry> registry) { command.registry = registry; return *this; }
        auto& entity(entt::entity entity) { command.entity = entity; return *this; }
        auto& component(entt::id_type id) { command.component_id = id; return *this; }
        auto& prev_value(const entt::meta_any& value) { command.prev_value = value; return *this; }
        auto& new_value(const entt::meta_any& value) { command.new_value = value; return *this; }

        auto& push_path_data(entt::id_type id, const std::string& name)
        {
            command.meta_path.entries.push_back(
                MetaPath::Entry{ .type = MetaPath::Entry::Type::Data, .data_id = id, .name = name }
            );
            return *this;
        }

        auto& push_path_index(int index, const std::string& name)
        {
            command.meta_path.entries.push_back(
                MetaPath::Entry{ .type = MetaPath::Entry::Type::Index, .index = index, .name = name }
            );
            return *this;
        }

        auto& push_path_key(const entt::meta_any& key_any, const std::string& name)
        {
            command.meta_path.entries.push_back(
                MetaPath::Entry{ .type = MetaPath::Entry::Type::Key, .key_any = key_any, .name = name }
            );
            return *this;
        }

        void pop_path() { command.meta_path.entries.pop_back(); }

        auto& reset()
        {
            assert(!command.meta_path.entries.size() && "Meta path not empty when clearing");
            command = ComponentCommand{};
            return *this;
        }

        ComponentCommand build()
        {
            // Valdiate before returning command instance 

            assert(!command.registry.expired() && "registry pointer expired");
            assert(command.entity != entt::null && "entity invalid");
            assert(command.component_id != 0 && "component id invalid");

            assert(command.prev_value);
            assert(command.new_value);

            assert(command.meta_path.entries.size() && "Meta path empty");
            {
                bool last_was_index_or_key = false;
                for (int i = 0; i < command.meta_path.entries.size(); i++)
                {
                    auto& entry = command.meta_path.entries[i];

                    // First entry must be Data (enter data member of a component)
                    assert(i > 0 || entry.type == MetaPath::Entry::Type::Data);

                    // Check so relevant values are set for each entry type
                    assert(entry.type != MetaPath::Entry::Type::Data || entry.data_id);
                    assert(entry.type != MetaPath::Entry::Type::Index || entry.index > -1);
                    assert(entry.type != MetaPath::Entry::Type::Key || entry.key_any);

                    last_was_index_or_key =
                        entry.type == MetaPath::Entry::Type::Index ||
                        entry.type == MetaPath::Entry::Type::Key;
                }
            }

            // Build a display name
            for (int i = 0; i < command.meta_path.entries.size(); i++)
            {
                // ...
            }

            return command;
        }
    };

} // namespace Editor

#endif /* EditComponentCommand_hpp */
