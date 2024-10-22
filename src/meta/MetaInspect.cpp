//
//  MetaInspect.cpp
//  engine_core_2024
//
//  Created by Carl Johan Gribel on 2024-08-08.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <cassert>
#include "imgui.h"
#include "meta_literals.h"
#include "meta_aux.h"

#include "MetaInspect.hpp"
#include "InspectType.hpp"
#include "EditComponentCommand.hpp"

// #define INSPECT_DEBUG_PRINT
#define USE_COMMANDS

namespace Editor {

    namespace
    {
        void generate_command(auto& cmd_queue_wptr, auto& cmd_builder)
        {
            assert(!cmd_queue_wptr.expired());
            auto cmd_queue_sptr = cmd_queue_wptr.lock();
            cmd_queue_sptr->add(CommandFactory::Create<ComponentCommand>(cmd_builder.build()));
        }
    }

#ifdef USE_COMMANDS
    struct MetaEntry
    {
        enum class Type : int { Data, Index, Key } type;

        entt::id_type data_id;  // enter data field
        size_t index;           // enter seq. container index
        entt::meta_any key_any; // enter assoc. container key

        std::string name = "(no name)";
    };

    struct MetaCommandDescriptor
    {
        entt::entity entity;
        entt::id_type comp_id;
        //entt::meta_type component_meta_type;
        //std::string propertyName; // path
        std::deque<MetaEntry> meta_path;
        entt::meta_any old_value, new_value;
        bool is_used = false;
    };
    static inline MetaCommandDescriptor meta_command{};
    static inline std::vector<MetaCommandDescriptor> issued_commands{};

    static void execute_impl(entt::meta_any& meta_any, entt::meta_data meta_data)
    {
        // entt::meta_data meta_data; // get

        // //meta_data.set(meta_any, data_any);
        // bool res = meta_data.set(meta_any, c.new_value); assert(res);
    }

#endif

    std::string get_entity_name(
        std::shared_ptr<entt::registry>& registry,
        entt::entity entity,
        entt::meta_type meta_type_with_name)
    {
        //assert(registry.valid(entity));
        auto entity_str = std::to_string(entt::to_integral(entity));

        // DEBUG
        if (!registry->valid(entity)) entity_str = entity_str + " [invalid]";

        // No meta type to use
        if (!meta_type_with_name) return entity_str;

        // Check if name data field exists
        entt::meta_data meta_data = meta_type_with_name.data("name"_hs);
        if (!meta_data) return entity_str;

        // Find storage for component type
        auto storage = registry->storage(meta_type_with_name.id());
        if (!storage) return entity_str;
        if (!storage->contains(entity)) return entity_str;

        // Instantiate component
        auto v = storage->value(entity);
        auto comp_any = meta_type_with_name.from_void(v);
        if (!comp_any) return entity_str;

        // Get data value
        auto data = meta_data.get(comp_any);

        // Fetch name from component
        auto name_ptr = data.template try_cast<std::string>();
        // Cast failed
        if (!name_ptr) return entity_str;

        // Does NOT return the correct string
//        return *name_ptr + std::string("###") + entity_str;

        return data.template cast<std::string>() + "###" + entity_str;
    }

    bool inspect_enum_any(
        entt::meta_any& any,
        InspectorState& inspector)
    {
        entt::meta_type meta_type = entt::resolve(any.type().id());
        assert(meta_type);
        assert(meta_type.is_enum());

        // Cast to underlying enum meta type
        auto any_conv = cast_to_underlying_type(meta_type, any);

        // Gather enum entries and determine which is the current one
        auto enum_entries = gather_meta_enum_entries(any);
        auto cur_entry = std::find_if(enum_entries.begin(), enum_entries.end(), [&any_conv](auto& e) {
            return e.second == any_conv;
            });
        assert(cur_entry != enum_entries.end());

        // Build combo
        auto selected_entry = cur_entry;
        if (ImGui::BeginCombo("##enum", cur_entry->first.c_str()))
        {
            for (auto it = enum_entries.begin(); it != enum_entries.end(); it++)
            {
                const bool isSelected = (it->second == cur_entry->second);
                if (ImGui::Selectable(it->first.c_str(), isSelected))
                    selected_entry = it;

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // Update current enum if needed
        if (selected_entry != cur_entry)
        {
            // assert(any.assign(selected_entry->second));
            bool ret = any.assign(selected_entry->second);
            assert(ret);
            return true;
        }
        return false;
    }

    bool inspect_any(
        entt::meta_any& any,
        InspectorState& inspector,
        ComponentCommandBuilder& cmd_builder)
    {
        assert(any);
        bool mod = false; // TODO: not used yet
#ifdef INSPECT_DEBUG_PRINTS
        std::cout << "inspect_any " << meta_type_name(any.type()) << std::endl;
#endif

        if (entt::meta_type meta_type = entt::resolve(any.type().id()); meta_type)
        {
            if (entt::meta_func meta_func = meta_type.func(inspect_hs); meta_func)
            {
                // Inspection meta function signatures:
                //      bool(void* ptr, Editor::InspectorState& inspector)
                //      bool(const void* ptr, Editor::InspectorState& inspector) ?

#ifdef USE_COMMANDS
                // Invoke inspection meta function on a copy of the object
                auto copy_any = any;
                auto res_any = meta_func.invoke({}, copy_any.data(), entt::forward_as_meta(inspector));
                assert(res_any && "Failed to invoke inspect meta function");

                // Issue command if a change is detected
                auto res_ptr = res_any.try_cast<bool>();
                assert(res_ptr && "inspect meta function is expected to return bool");
                if (*res_ptr)
                {
                    meta_command.old_value = any;
                    meta_command.new_value = copy_any;
                    assert(!meta_command.is_used); meta_command.is_used = true;
                    issued_commands.push_back(meta_command);

                    // Build & issue command
                    cmd_builder.prev_value(any).new_value(copy_any);
                    generate_command(inspector.cmd_queue, cmd_builder);
                    // assert(!inspector.cmd_queue.expired());
                    // auto cmd_queue = inspector.cmd_queue.lock();
                    // cmd_queue->add(CommandFactory::Create<ComponentCommand>(cmd_builder.build()));

                    mod = true;
                }
#else
                // Invoke the inspection meta function on the object
                auto res_any = meta_func.invoke({}, any.data(), entt::forward_as_meta(inspector));
                assert(res_any && "Failed to invoke inspect meta function");
                auto res_ptr = res_any.try_cast<bool>();
                assert(res_ptr && "inspect meta function expected to return bool");
                mod |= *res_ptr;
#endif
            }
            else if (meta_type.is_enum())
            {
#ifdef USE_COMMANDS
                auto copy_any = any;
                if (inspect_enum_any(copy_any, inspector))
                {
                    meta_command.old_value = any;
                    meta_command.new_value = copy_any;
                    assert(!meta_command.is_used); meta_command.is_used = true;
                    issued_commands.push_back(meta_command);

                    // Build & issue command
                    cmd_builder.prev_value(any).new_value(copy_any);
                    generate_command(inspector.cmd_queue, cmd_builder);
                    // assert(!inspector.cmd_queue.expired());
                    // auto cmd_queue = inspector.cmd_queue.lock();
                    // //auto cmd = cmd_builder.build();
                    // cmd_queue->add(CommandFactory::Create<ComponentCommand>(cmd_builder.build()));

                    mod = true;
                }
#else
                mod |= inspect_enum_any(any, inspector);
#endif
            }
            else
            {
                // inspect() not available: traverse data members
                for (auto&& [id, meta_data] : meta_type.data())
                {
                    std::string key_name = meta_data_name(id, meta_data);
#ifdef INSPECT_DEBUG_PRINT
                    std::cout << "inspecting data field: " << key_name << std::endl;
#endif
                    // if (inspector.is_disabled())
                    ImGui::SetNextItemOpen(true);

                    if (inspector.begin_node(key_name.c_str()))
                    {
#ifdef USE_COMMANDS
                        // Push command meta path
                        MetaEntry meta_entry{};
                        meta_entry.type = MetaEntry::Type::Data; meta_entry.data_id = id; meta_entry.name = key_name;
                        meta_command.meta_path.push_back(meta_entry);

                        cmd_builder.push_path_data(id, key_name);
#endif
                        // Obtain copy of data value
                        entt::meta_any data_any = meta_data.get(any);
                        // Check & set readonly
                        bool readonly = get_meta_data_prop<bool, ReadonlyDefault>(meta_data, readonly_hs);
                        if (readonly) inspector.begin_disabled();

                        // Inspect
                        mod |= inspect_any(data_any, inspector, cmd_builder);
#ifndef USE_COMMANDS
                        // Update data of the current object
                        meta_data.set(any, data_any);
#endif
                        // Unset readonly
                        if (readonly) inspector.end_disabled();
#ifdef USE_COMMANDS
                        // Pop meta command path
                        meta_command.meta_path.pop_back();

                        cmd_builder.pop_path();
#endif
                        inspector.end_node();
                    }
#ifdef INSPECT_DEBUG_PRINT
                    std::cout << "DONE inspecting data field" << key_name << std::endl;
#endif
                }
            }
            return mod;
        }

        // any is not a meta type

        if (any.type().is_sequence_container())
        {
            auto view = any.as_sequence_container();
            assert(view && "as_sequence_container() failed");
#ifdef INSPECT_DEBUG_PRINT
            std::cout << "is_sequence_container: " << meta_type_name(any.type()) << ", size " << view.size() << std::endl;
#endif
            int count = 0;
            for (auto&& v : view)
            {
                inspector.begin_leaf((std::string("#") + std::to_string(count)).c_str());
                {
#ifdef USE_COMMANDS
                    // Push command meta path
                    MetaEntry meta_entry{};
                    meta_entry.type = MetaEntry::Type::Index; meta_entry.index = count; meta_entry.name = std::to_string(count);
                    meta_command.meta_path.push_back(meta_entry);

                    cmd_builder.push_path_index(count, std::to_string(count));

                    // ImGui::SetNextItemWidth(-FLT_MIN);
                    mod |= inspect_any(v, inspector, cmd_builder);

                    // Pop meta command path
                    meta_command.meta_path.pop_back();

                    cmd_builder.pop_path();
#else
                    // ImGui::SetNextItemWidth(-FLT_MIN);
                    mod |= inspect_any(v, inspector, cmd_builder); // Will change the actual element
#endif
                }
                inspector.end_leaf();
                count++;
            }
        }

        else if (any.type().is_associative_container())
        {
            auto view = any.as_associative_container();
            assert(view && "as_associative_container() failed");
#ifdef INSPECT_DEBUG_PRINT
            std::cout << "is_associative_container: " << meta_type_name(any.type()) << ", size " << view.size() << std::endl;
#endif
            int count = 0;
            for (auto&& [key_any, mapped_any] : view)
            {
                // Displaying key, value and the element itself as expandable nodes
                ImGui::SetNextItemOpen(true);
                if (inspector.begin_node((std::string("#") + std::to_string(count)).c_str()))
                {
                    ImGui::SetNextItemOpen(true);
                    if (inspector.begin_node("[key]"))
                    {
                        // Note: keys are read-only, and primitive key types
                        // will cast to const (and display as disabled if a 
                        // const specialization is defined). When meta types 
                        // are used as keys however, this does not seem to work
                        // the same way (they cast to non-const - but cannot be
                        // modified).
                        // Workaround: disable all key inspections explicitly
                        inspector.begin_disabled();
                        inspect_any(key_any, inspector, cmd_builder); // key_any holds a const object
                        inspector.end_disabled();
                        inspector.end_node();
                    }
                    if (view.mapped_type())
                    {
#ifdef USE_COMMANDS
                        // Push command meta path
                        MetaEntry meta_entry{};
                        meta_entry.type = MetaEntry::Type::Key; meta_entry.key_any = key_any; meta_entry.name = std::to_string(count);
                        meta_command.meta_path.push_back(meta_entry);

                        cmd_builder.push_path_key(key_any, std::to_string(count));
#endif

                        // Inspect mapped value
                        ImGui::SetNextItemOpen(true);
                        if (inspector.begin_node("[value]"))
                        {
                            mod |= inspect_any(mapped_any, inspector, cmd_builder);
                            inspector.end_node();
                        }
#ifdef USE_COMMANDS
                        // Pop meta command path
                        meta_command.meta_path.pop_back();

                        cmd_builder.pop_path();
#endif
                    }

                    inspector.end_node();
                }
                count++;
            }
        }

        else
        {
            // Try casting the meta_any to a primitive type and perform the inspection
            bool res = try_apply(any, [&](auto& value) {
#ifdef USE_COMMANDS
                // Inspect a copy of the value and issue command if a change is detected
                auto value_copy = value;
                if (Editor::inspect_type(value_copy, inspector))
                {
                    meta_command.old_value = any;
                    meta_command.new_value = value_copy;
                    assert(!meta_command.is_used); meta_command.is_used = true;
                    issued_commands.push_back(meta_command);
                    //

                    // Build & issue command
                    cmd_builder.prev_value(any).new_value(value_copy);
                    generate_command(inspector.cmd_queue, cmd_builder);
                    // assert(!inspector.cmd_queue.expired());
                    // auto cmd_queue = inspector.cmd_queue.lock();
                    // //auto cmd = cmd_builder.build();
                    // cmd_queue->add(CommandFactory::Create<ComponentCommand>(cmd_builder.build()));

                    mod = true;
                }
#else
                // Inspect the value
                Editor::inspect_type(value, inspector);
#endif
                });
            if (!res)
                throw std::runtime_error(std::string("Unable to cast type ") + meta_type_name(any.type()));
        }
        
#ifdef INSPECT_DEBUG_PRINT
        std::cout << "DONE inspect_any " << meta_type_name(any.type()) << std::endl;
#endif
        return mod;
    }

    bool inspect_entity(
        entt::entity entity,
        InspectorState& inspector)
    {
        bool mod = false;
#ifdef USE_COMMANDS
        // meta command: clear issued
        issued_commands.clear();

#endif
        ComponentCommandBuilder cmd_builder;

        auto& registry = inspector.context.registry;
        assert(entity != entt::null);
        assert(registry->valid(entity));

        for (auto&& [id, type] : registry->storage())
        {
            if (!type.contains(entity)) continue;

            if (entt::meta_type meta_type = entt::resolve(id); meta_type)
            {
                auto type_name = meta_type_name(meta_type);

                if (inspector.begin_node(type_name.c_str()))
                {
#ifdef USE_COMMANDS
                    // Reset meta command for each component type
                    meta_command = MetaCommandDescriptor{};
                    meta_command.entity = entity; meta_command.comp_id = id;

                    cmd_builder.reset().
                        registry(inspector.context.registry)
                        .entity(entity)
                        .component(id);
#endif

                    auto comp_any = meta_type.from_void(type.value(entity));
                    mod |= inspect_any(comp_any, inspector, cmd_builder);
                    inspector.end_node();
                }
            }
            else
            {
                //All types exposed to Lua are going to have a meta type
                assert(false && "Meta-type required");
            }
        }

#ifdef USE_COMMANDS
        // #define EXECUTE_COMMAND
#ifdef EXECUTE_COMMAND
        // check issued command (typically one?)
        for (auto& c : issued_commands)
        {
            if (!c.is_used)
            {
                std::cout << "Command not used" << std::endl;
                continue;
            }

            std::cout << "meta_command.is_used, entity " << entt::to_integral(c.entity) << std::endl;
            for (auto& e : c.meta_path)
            {
                std::cout << "\t";
                if (e.type == MetaEntry::Type::Data) std::cout << "DATA";
                if (e.type == MetaEntry::Type::Index) std::cout << "INDEX";
                if (e.type == MetaEntry::Type::Key) std::cout << "KEY";
                std::cout << ", " << e.name << ", (field) data_id " << e.data_id;
                std::cout << ", (index) index " << e.index;
                // std::cout << " (id_type for float " << entt::type_hash<float>::value() << ")";
                auto xhs = "x"_hs; std::cout << " (id_type for 'x'_hs " << xhs.value() << ")";
                std::cout << std::endl;
            }
            std::cout << "new_value true " << (bool)c.new_value;
            std::cout << " try_cast to float ";
            auto fltptr = c.new_value.try_cast<float>();
            if (fltptr) std::cout << *fltptr;
            std::cout << std::endl;

            // EXECUTE command ...
            // new_value is actually old value ...
            // enum class Type : int { Data, Index, Key } type;
            // entt::id_type data_id;  // enter data field
            // size_t index;           // enter seq. container index
            // entt::meta_any key_any; // enter assoc. container key
            // std::string name = "(no name)";

            // execute(registry, c);

            // auto type = registry.storage(e.data_id);

            //entt::meta_type meta_type = c.component_meta_type;
            //entt::meta_any meta_any;

            const auto any_to_string = [](const entt::meta_any& any) -> std::string {
                if (auto ptr = any.try_cast<float>(); ptr) return std::to_string(*ptr);
                if (auto ptr = any.try_cast<int>(); ptr) return std::to_string(*ptr);
                if (auto ptr = any.try_cast<bool>(); ptr) return std::to_string(*ptr);
                return std::string("[any not cast]");
                };

            // Component
            auto type = registry->storage(c.comp_id);
            assert(type->contains(c.entity));
            entt::meta_type meta_type = entt::resolve(c.comp_id);
            entt::meta_any meta_any = meta_type.from_void(type->value(entity));

            assert(c.meta_path.size()); // Must be at least one?
            assert(c.meta_path[0].type == MetaEntry::Type::Data); // First must be Data?            
            auto& entry0 = c.meta_path[0];
            entt::meta_data meta_data = meta_type.data(entry0.data_id);

            // entt::meta_any data_any = meta_data.get(meta_any);

            std::cout << "meta_any.type().info().name() " << meta_any.type().info().name() << std::endl; // DebugClass
            std::cout << "meta_type.info().name " << meta_type.info().name() << std::endl; //DebugClass
            // std::cout << "data_any.type().info().name() " << data_any.type().info().name() << std::endl; // debugvec3
            std::cout << "meta_data.type().info().name() " << meta_data.type().info().name() << std::endl; // debugvec3

            struct Property {
                entt::meta_any meta_any; entt::meta_data meta_data; MetaEntry entry;
            };
            std::stack<Property> prop_stack;
            Property last_prop{ meta_any, meta_data, entry0 };
            prop_stack.push(last_prop);

            int i = 1;
            //entt::meta_any meta_any_ = meta_any;
            //entt::meta_data meta_data_ = meta_data;
            for (;i < c.meta_path.size(); i++)
            {
                auto& e = c.meta_path[i];
                if (e.type == MetaEntry::Type::Data)
                {
                    entt::meta_any meta_any;
                    // If the previous entry was Index, meta_data will be empty
                    // Fetch meta_data from elem_any = the container?
                    if (last_prop.entry.type == MetaEntry::Type::Index)
                        meta_any = last_prop.meta_any.as_sequence_container()[last_prop.entry.index];
                    else if (last_prop.entry.type == MetaEntry::Type::Key)
                        meta_any = last_prop.meta_any.as_associative_container().find(last_prop.entry.key_any)->second;
                    else if (last_prop.entry.type == MetaEntry::Type::Data)
                        meta_any = last_prop.meta_data.get(last_prop.meta_any);
                    else { assert(0); }
                    assert(meta_any);
                    // auto meta_any = last_prop.meta_data.get(last_prop.meta_any); assert(meta_any);
                    auto meta_type = entt::resolve(meta_any.type().id());  assert(meta_type);
                    auto meta_data = meta_type.data(e.data_id); assert(meta_data);

                    last_prop = Property{ meta_any, meta_data, e };
                    prop_stack.push(last_prop);
                }
                else if (e.type == MetaEntry::Type::Index)
                {
                    assert(last_prop.entry.type == MetaEntry::Type::Data);
                    auto meta_any = last_prop.meta_data.get(last_prop.meta_any); assert(meta_any); // = container
                    ///* Should not exist */ auto meta_type = entt::resolve(meta_any.type().id());  assert(!meta_type);
                    //auto meta_data = meta_type.data(e.data_id); assert(meta_data);

                    // Forward any to element
                    // auto elem_any = meta_any.as_sequence_container()[e.index];

                    last_prop = Property{ meta_any, entt::meta_data{}, e };
                    prop_stack.push(last_prop);
                }
                else if (e.type == MetaEntry::Type::Key)
                {
                    assert(last_prop.entry.type == MetaEntry::Type::Data);
                    auto meta_any = last_prop.meta_data.get(last_prop.meta_any); assert(meta_any); // = container
                    last_prop = Property{ meta_any, entt::meta_data{}, e };
                    prop_stack.push(last_prop);
                }
                else { assert(0); }
            }
            entt::meta_any any_new = c.new_value; // float -> UVCoord -> debugvec3 -> DebugClass
            while (!prop_stack.empty())
            {
                auto& prop = prop_stack.top();
                std::cout << prop_stack.size() << " prop.meta_any " << prop.meta_any.type().info().name() << std::endl; // 
                std::cout << prop_stack.size() << " any_new " << any_new.type().info().name() << std::endl; // 

                if (prop.entry.type == MetaEntry::Type::Data)
                {
                    // Data
                    bool res = prop.meta_data.set(prop.meta_any, any_new); assert(res);
                }
                else if (prop.entry.type == MetaEntry::Type::Index)
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
                else if (prop.entry.type == MetaEntry::Type::Key)
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
#endif
#endif
        return mod;
    }

#if 0
    bool inspect_registry(
        entt::meta_type name_comp_type,
        InspectorState& inspector)
    {
        bool mod = false;
        auto& registry = inspector.context.registry;

        auto view = registry->template view<entt::entity>();
        for (auto entity : view)
        {
#if 0
            inspect_entity(entity, inspector);
#else
            auto entity_name = get_entity_name(registry, entity, name_comp_type);
            if (!inspector.begin_node(entity_name.c_str()))
                continue;

            for (auto&& [id, type] : registry->storage())
            {
                if (!type.contains(entity)) continue;

                if (entt::meta_type meta_type = entt::resolve(id); meta_type)
                {
                    auto type_name = meta_type_name(meta_type);

                    if (inspector.begin_node(type_name.c_str()))
                    {
                        auto comp_any = meta_type.from_void(type.value(entity));
                        mod |= inspect_any(comp_any, inspector);
                        inspector.end_node();
                    }
                }
                else
                {
                    //All types exposed to Lua are going to have a meta type
                    assert(false && "Meta-type required");
                }
            }
            inspector.end_node();
#endif
        }
        return mod;
    }
#endif

} // namespace Editor