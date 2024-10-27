//
//  EditComponentCommand.cpp
//
//  Created by Carl Johan Gribel on 2024-10-14.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#include <iostream>
// #include <sstream>
#include <cassert>
// #include "imgui.h"
// #include "MetaInspect.hpp"
// #include "InspectType.hpp"
#include "meta_literals.h"
// #include "meta_aux.h"
#include "MetaSerialize.hpp"

#include "EditComponentCommand.hpp"

namespace Editor {

    void ComponentCommand::traverse_and_set_meta_type(entt::meta_any& value_any)
    {
        using EntryType = MetaPath::Entry::Type;
        assert(!registry.expired());
        auto registry_sp = registry.lock();

#ifdef COMMAND_DEBUG_PRINTS
        auto any_to_string = [](const entt::meta_any any) -> std::string {
            if (auto j = Meta::serialize_any(any); !j.is_null())
                return j.dump();
            return "n/a";
            };

        std::cout << "Executing command: " << get_name() << std::endl;
        std::cout << "\tentity " << entt::to_integral(entity);
        std::cout << ", component type " << component_id << std::endl;
        std::cout << "\tprev value: " << any_to_string(prev_value) << std::endl;
        std::cout << "\tnew value: " << any_to_string(new_value) << std::endl;
        // std::cout << "\tpath:" << std::endl;
        // for (auto& e : meta_path.entries)
        // {
        //     if (e.type == EntryType::Data) std::cout << "\t\tData: data_id = " << e.data_id << std::endl;
        //     if (e.type == EntryType::Index) std::cout << "\t\tIndex: index = " << e.index << std::endl;
        //     if (e.type == EntryType::Key) std::cout << "\t\tKey: key = " << any_to_string(e.key_any) << std::endl;
        // }
#endif   
#if 0
        std::cout << "traverse_and_set_meta_type, entity " << entt::to_integral(entity) << std::endl;
        for (auto& e : meta_path.entries)
        {
            std::cout << "\t";
            if (e.type == EntryType::Data) std::cout << "DATA";
            if (e.type == EntryType::Index) std::cout << "INDEX";
            if (e.type == EntryType::Key) std::cout << "KEY";
            std::cout << ", " << e.name << ", (field) data_id " << e.data_id;
            std::cout << ", (index) index " << e.index;
            // std::cout << " (id_type for float " << entt::type_hash<float>::value() << ")";
            auto xhs = "x"_hs; std::cout << " (id_type for 'x'_hs " << xhs.value() << ")";
            std::cout << std::endl;
        }
        std::cout << "new_value true " << (bool)value_any; // (bool)c.new_value;
        std::cout << " try_cast to float ";
        auto fltptr = value_any.try_cast<float>(); // c.new_value.try_cast<float>();
        if (fltptr) std::cout << *fltptr;
        std::cout << std::endl;
#endif

        // Component
        auto type = registry_sp->storage(component_id);
        assert(type->contains(entity));
        entt::meta_type meta_type = entt::resolve(component_id);
        entt::meta_any meta_any = meta_type.from_void(type->value(entity));

        assert(meta_path.entries.size()); // Must be at least one?
        assert(meta_path.entries[0].type == EntryType::Data); // First must be Data?            
        auto& entry0 = meta_path.entries[0];
        entt::meta_data meta_data = meta_type.data(entry0.data_id);

        // entt::meta_any data_any = meta_data.get(meta_any);

        std::cout << "meta_any.type().info().name() " << meta_any.type().info().name() << std::endl; // DebugClass
        std::cout << "meta_type.info().name " << meta_type.info().name() << std::endl; //DebugClass
        // std::cout << "data_any.type().info().name() " << data_any.type().info().name() << std::endl; // debugvec3
        std::cout << "meta_data.type().info().name() " << meta_data.type().info().name() << std::endl; // debugvec3

        struct Property {
            entt::meta_any meta_any; entt::meta_data meta_data; MetaPath::Entry entry;
        };
        std::stack<Property> prop_stack;
        Property last_prop{ meta_any, meta_data, entry0 };
        prop_stack.push(last_prop);

        int i = 1;
        //entt::meta_any meta_any_ = meta_any;
        //entt::meta_data meta_data_ = meta_data;
        for (;i < meta_path.entries.size(); i++)
        {
            auto& e = meta_path.entries[i];
            if (e.type == EntryType::Data)
            {
                entt::meta_any meta_any;
                // If the previous entry was Index, meta_data will be empty
                // Fetch meta_data from elem_any = the container?
                if (last_prop.entry.type == EntryType::Index)
                    meta_any = last_prop.meta_any.as_sequence_container()[last_prop.entry.index];
                else if (last_prop.entry.type == EntryType::Key)
                    meta_any = last_prop.meta_any.as_associative_container().find(last_prop.entry.key_any)->second;
                else if (last_prop.entry.type == EntryType::Data)
                    meta_any = last_prop.meta_data.get(last_prop.meta_any);
                else { assert(0); }
                assert(meta_any);
                // auto meta_any = last_prop.meta_data.get(last_prop.meta_any); assert(meta_any);
                auto meta_type = entt::resolve(meta_any.type().id());  assert(meta_type);
                auto meta_data = meta_type.data(e.data_id); assert(meta_data);

                last_prop = Property{ meta_any, meta_data, e };
                prop_stack.push(last_prop);
            }
            else if (e.type == EntryType::Index)
            {
                assert(last_prop.entry.type == EntryType::Data);
                auto meta_any = last_prop.meta_data.get(last_prop.meta_any); assert(meta_any); // = container
                ///* Should not exist */ auto meta_type = entt::resolve(meta_any.type().id());  assert(!meta_type);
                //auto meta_data = meta_type.data(e.data_id); assert(meta_data);

                // Forward any to element
                // auto elem_any = meta_any.as_sequence_container()[e.index];

                last_prop = Property{ meta_any, entt::meta_data{}, e };
                prop_stack.push(last_prop);
            }
            else if (e.type == EntryType::Key)
            {
                assert(last_prop.entry.type == EntryType::Data);
                auto meta_any = last_prop.meta_data.get(last_prop.meta_any); assert(meta_any); // = container
                last_prop = Property{ meta_any, entt::meta_data{}, e };
                prop_stack.push(last_prop);
            }
            else { assert(0); }
        }
        entt::meta_any any_new = value_any; // c.new_value; // float -> UVCoord -> debugvec3 -> DebugClass
        while (!prop_stack.empty())
        {
            auto& prop = prop_stack.top();
            std::cout << prop_stack.size() << " prop.meta_any " << prop.meta_any.type().info().name() << std::endl; // 
            std::cout << prop_stack.size() << " any_new " << any_new.type().info().name() << std::endl; // 

            if (prop.entry.type == EntryType::Data)
            {
                // Data
                bool res = prop.meta_data.set(prop.meta_any, any_new); assert(res);
            }
            else if (prop.entry.type == EntryType::Index)
            {
                // TODO: range check

                // Container - meta_any IS the element?
                std::cout << "prop.meta_any " << prop.meta_any.type().info().name() << std::endl; //                     
                // std::cout << "prop.elem_any " << prop.elem_any.type().info().name() << std::endl; // 

                //bool res = prop.elem_any.assign(any_new); assert(res);
                assert(prop.meta_any.type().is_sequence_container());
                auto view = prop.meta_any.as_sequence_container();
                std::cout << "before val=" << any_to_string(view[prop.entry.index])
                    << " new=" << any_to_string(any_new) << std::endl;
                view[prop.entry.index].assign(any_new); // WORKS
                // view[prop.entry.index] = any_new; // DOES NOT WORK
                std::cout << "after val=" << any_to_string(view[prop.entry.index])
                    << " new=" << any_to_string(any_new) << std::endl;
                // int count = 0;
                // for (auto&& v : view)
                // {
                //     if (count++ == prop.index) {v = any_new; std::cout << "set val " << any_new.cast<int>() << std::endl; }
                // }
                //bool res_ = prop.meta_data.set(prop.meta_any, prop.any_new); assert(res_);
                std::cout << "view[prop.index] " << view[prop.entry.index].type().info().name() << std::endl; // 
                std::cout << "view[prop.index] value " << any_to_string(view[prop.entry.index]) << std::endl; // 
            }
            else if (prop.entry.type == EntryType::Key)
            {
                assert(prop.meta_any.type().is_associative_container());
                auto view = prop.meta_any.as_associative_container();
                view.find(prop.entry.key_any)->second.assign(any_new); // WORKS?
                // view[prop.entry.index] = any_new; // DOES NOT WORK?
            }
            else { assert(0); }

            any_new = prop.meta_any;
            prop_stack.pop();
        }
        std::cout << "DONE" << std::endl;
        std::cout << "prop.meta_any " << meta_any.type().info().name() << std::endl; // 
        std::cout << "meta_any_rec " << any_new.type().info().name() << std::endl; // 
        std::cout << " meta_any_rec " << any_to_string(any_new) << std::endl;

        // meta_data.set(meta_any, meta_any_rec);
        // At this point, the type held by any_new is the component type
        // any_new is an updated copy of the component - now assign ut to the actual component
        meta_any.assign(any_new);
    }

    void ComponentCommand::execute()
    {
        traverse_and_set_meta_type(new_value);
    }

    void ComponentCommand::undo()
    {
        traverse_and_set_meta_type(prev_value);
    }

    std::string ComponentCommand::get_name() const
    {
        return display_name;
    }

    ComponentCommandBuilder& ComponentCommandBuilder::registry(std::weak_ptr<entt::registry> registry) { command.registry = registry; return *this; }

    ComponentCommandBuilder& ComponentCommandBuilder::entity(entt::entity entity) { command.entity = entity; return *this; }

    ComponentCommandBuilder& ComponentCommandBuilder::component(entt::id_type id) { command.component_id = id; return *this; }

    ComponentCommandBuilder& ComponentCommandBuilder::prev_value(const entt::meta_any& value) { command.prev_value = value; return *this; }

    ComponentCommandBuilder& ComponentCommandBuilder::new_value(const entt::meta_any& value) { command.new_value = value; return *this; }

    ComponentCommandBuilder& ComponentCommandBuilder::push_path_data(entt::id_type id, const std::string& name)
    {
        command.meta_path.entries.push_back(
            MetaPath::Entry{ .type = MetaPath::Entry::Type::Data, .data_id = id, .name = name }
        );
        return *this;
    }

    ComponentCommandBuilder& ComponentCommandBuilder::push_path_index(int index, const std::string& name)
    {
        command.meta_path.entries.push_back(
            MetaPath::Entry{ .type = MetaPath::Entry::Type::Index, .index = index, .name = name }
        );
        return *this;
    }

    ComponentCommandBuilder& ComponentCommandBuilder::push_path_key(const entt::meta_any& key_any, const std::string& name)
    {
        command.meta_path.entries.push_back(
            MetaPath::Entry{ .type = MetaPath::Entry::Type::Key, .key_any = key_any, .name = name }
        );
        return *this;
    }

    ComponentCommandBuilder& ComponentCommandBuilder::pop_path()
    {
        command.meta_path.entries.pop_back();
        return *this;
    }

    ComponentCommandBuilder& ComponentCommandBuilder::reset()
    {
        assert(!command.meta_path.entries.size() && "Meta path not empty when clearing");
        command = ComponentCommand{};
        return *this;
    }

    ComponentCommand ComponentCommandBuilder::build()
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
                assert(entry.type != MetaPath::Entry::Type::None);
                assert(entry.type != MetaPath::Entry::Type::Data || entry.data_id);
                assert(entry.type != MetaPath::Entry::Type::Index || entry.index > -1);
                assert(entry.type != MetaPath::Entry::Type::Key || entry.key_any);

                last_was_index_or_key =
                    entry.type == MetaPath::Entry::Type::Index ||
                    entry.type == MetaPath::Entry::Type::Key;
            }
        }

        // Build a display name for the command
        {
            // Gather meta path
            command.display_name = entt::resolve(command.component_id).info().name();
            for (auto& entry : command.meta_path.entries)
            {
                if (entry.type == MetaPath::Entry::Type::Data) {
                    command.display_name += "::" + entry.name;
                }
                else if (entry.type == MetaPath::Entry::Type::Index) {
                    command.display_name += "[" + std::to_string(entry.index) + "]";
                }
                else if (entry.type == MetaPath::Entry::Type::Key) {
                    if (auto j_new = Meta::serialize_any(entry.key_any); !j_new.is_null())
                        command.display_name += "[" + j_new.dump() + "]";
                    else
                        command.display_name += "[]";
                }
            }
            // Serialize new value
            if (auto j_new = Meta::serialize_any(command.new_value); !j_new.is_null())
                command.display_name += " = " + j_new.dump();
        }

        return command;
    }

} // namespace Editor