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
#include "MetaInspect.hpp"
#include "InspectType.hpp"
#include "meta_literals.h"
#include "meta_aux.h"

namespace Editor {

    // // meta command -->
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
        entt::meta_any new_value;
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

    static void execute(entt::registry& registry, const MetaCommandDescriptor& cmd)
    {
#if 0
        auto type = registry.storage(c.comp_id);
        assert(type->contains(c.entity));
        entt::meta_type meta_type = entt::resolve(c.comp_id);
        entt::meta_any meta_any = meta_type.from_void(type->value(entity));

        assert(c.meta_path.size()); // Must be at least one?
        assert(c.meta_path[0].type == MetaEntry::Type::Data); // First must be Data?            
        auto& e = c.meta_path[0];
        entt::meta_data meta_data = meta_type.data(e.data_id);

        int i = 1;
        for (;i < c.meta_path.size(); i++)
        {
            auto& e = c.meta_path[i];

            // --> meta_any, meta_type
            if (e.type == MetaEntry::Type::Data)
            {
                // auto meta_any_cpy = meta_any;
                // meta_any = entt::meta_any {};
                meta_any = meta_data.get(meta_any); assert(meta_any);
                meta_type = entt::resolve(meta_any.type().id());  assert(meta_type);
                meta_data = meta_type.data(e.data_id); assert(meta_data);

                data_any = meta_data.get(meta_any);

                // assert stuff

                std::cout << "meta_any.type().info().name() " << meta_any.type().info().name() << std::endl; // debugvec3
                std::cout << "meta_type.info().name " << meta_type.info().name() << std::endl; //debugvec3
                std::cout << "data_any.type().info().name() " << data_any.type().info().name() << std::endl; // float
                std::cout << "meta_data.type().info().name() " << meta_data.type().info().name() << std::endl; // float

                //auto tmp_any = meta_data.get(meta_any);
                //meta_any.assign(tmp_any);
            }
            else if (e.type == MetaEntry::Type::Index) { assert(0); }
            else if (e.type == MetaEntry::Type::Key) { assert(0); }
            else { assert(0); }
        }
#endif
    }

    // <--

    std::string get_entity_name(
        entt::registry& registry,
        entt::entity entity,
        entt::meta_type meta_type_with_name)
    {
        //assert(registry.valid(entity));
        auto entity_str = std::to_string(entt::to_integral(entity));

        // DEBUG
        if (!registry.valid(entity)) entity_str = entity_str + " [invalid]";

        // No meta type to use
        if (!meta_type_with_name) return entity_str;

        // Check if name data field exists
        entt::meta_data meta_data = meta_type_with_name.data("name"_hs);
        if (!meta_data) return entity_str;

        // Find storage for component type
        auto storage = registry.storage(meta_type_with_name.id());
        if (!storage) return entity_str;
        if (!storage->contains(entity)) return entity_str;

        // Instantiate component
        auto v = storage->value(entity);
        auto comp_any = meta_type_with_name.from_void(v);
        if (!comp_any) return entity_str;

        // Get data value
        auto data = meta_data.get(comp_any);

        // Fetch name from component
        auto name_ptr = data.try_cast<std::string>();
        // Cast failed
        if (!name_ptr) return entity_str;

        // Does NOT return the correct string
//        return *name_ptr + std::string("###") + entity_str;

        return data.cast<std::string>() + "###" + entity_str;
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

    void inspect_any(
        entt::meta_any& any,
        InspectorState& inspector)
    {
        assert(any);
        bool mod = false;

        if (entt::meta_type meta_type = entt::resolve(any.type().id()); meta_type)
        {
            if (entt::meta_func meta_func = meta_type.func(inspect_hs); meta_func)
            {
                // Function signatures 
                // void* ptr, Editor::InspectorState& inspector
                // const void* ptr, Editor::InspectorState& inspector (?)

                // Call from_json using alias of json node
                auto res = meta_func.invoke({}, any.data(), entt::forward_as_meta(inspector));
                assert(res && "Failed to invoke inspect");

                // std::cout << "inspect invoked: " << "..." << std::endl;
            }
            else if (meta_type.is_enum())
            {
                mod |= inspect_enum_any(any, inspector);
            }
            else
            {
                // inspect() not available: traverse data members
                for (auto&& [id, meta_data] : meta_type.data())
                {
                    std::string key_name = meta_data_name(id, meta_data);

                    // if (inspector.is_disabled())
                    ImGui::SetNextItemOpen(true);

                    if (inspector.begin_node(key_name.c_str()))
                    {
                        // Push meta command
                        MetaEntry meta_entry{};
                        meta_entry.type = MetaEntry::Type::Data; meta_entry.data_id = id; meta_entry.name = key_name;
                        meta_command.meta_path.push_back(meta_entry);

                        entt::meta_any data_any = meta_data.get(any);

                        // Check & set readonly
                        bool readonly = get_meta_data_prop<bool, ReadonlyDefault>(meta_data, readonly_hs);
                        if (readonly) inspector.begin_disabled();

                        inspect_any(data_any, inspector);
                        meta_data.set(any, data_any);
                        inspector.end_node();

                        // Unset readonly
                        if (readonly) inspector.end_disabled();

                        // Pop meta command if no change registered
                        meta_command.meta_path.pop_back();
                    }
                }
            }
            return;
        }

        // any is not a meta type

        if (any.type().is_sequence_container())
        {
            //ImGui::Text("[is_sequence_container]");

            auto view = any.as_sequence_container();
            assert(view && "as_sequence_container() failed");

            //auto json_array = nlohmann::json::array();
            int count = 0;
            for (auto&& v : view)
            {
                inspector.begin_leaf((std::string("#") + std::to_string(count++)).c_str());
                // ImGui::SetNextItemWidth(-FLT_MIN);
                inspect_any(v, inspector);
                inspector.end_leaf();
            }
        }

        else if (any.type().is_associative_container())
        {
            auto view = any.as_associative_container();
            assert(view && "as_associative_container() failed");

            int count = 0;
            for (auto&& [key_any, mapped_any] : view)
            {
                // inspector.begin_leaf((std::string("#") + std::to_string(count++)).c_str());
                // inspect_any(key_any, inspector);
                // if (view.mapped_type())
                // {
                //     ImGui::SetNextItemWidth(-FLT_MIN);
                //     inspect_any(mapped_any, inspector);
                // }
                // inspector.end_leaf();

                // Displaying key, value and the element itself as expandable nodes
                ImGui::SetNextItemOpen(true);
                if (inspector.begin_node((std::string("#") + std::to_string(count++)).c_str()))
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
                        inspect_any(key_any, inspector);
                        inspector.end_disabled();
                        inspector.end_node();
                    }
                    if (view.mapped_type())
                    {
                        ImGui::SetNextItemOpen(true);
                        if (inspector.begin_node("[value]"))
                        {
                            inspect_any(mapped_any, inspector);
                            inspector.end_node();
                        }
                    }
                    inspector.end_node();
                }
            }
        }

        else
        {
            // Try casting the meta_any to a primitive type.
            //
            bool res = try_apply(any, [&inspector, &any](auto& value) {
                // Editor::inspect_type(value, inspector);

                // meta command
                auto copy = value; //entt::meta_any copy_any = any;
                if (Editor::inspect_type(copy, inspector))
                {
                    // Maybe check if already used = inspection done in multiple places at once (!)
                    meta_command.new_value = copy; //any;
                    meta_command.is_used = true;
                    issued_commands.push_back(meta_command);
                }
                // else
                // No - POP the path
                    // meta_command.meta_path.pop_back(); // Leaf reached - reset path

                });
            if (!res)
                throw std::runtime_error(std::string("Unable to cast type ") + meta_type_name(any.type()));
        }
    }

    void inspect_entity(
        entt::entity entity,
        InspectorState& inspector)
    {
        // meta command: clear issued
        issued_commands.clear();

        auto& registry = *inspector.registry;
        assert(entity != entt::null);
        assert(registry.valid(entity));

        for (auto&& [id, type] : registry.storage())
        {
            if (!type.contains(entity)) continue;

            if (entt::meta_type meta_type = entt::resolve(id); meta_type)
            {
                auto type_name = meta_type_name(meta_type);

                if (inspector.begin_node(type_name.c_str()))
                {
                    // Reset meta command for each component type
                    meta_command = MetaCommandDescriptor{};
                    meta_command.entity = entity; meta_command.comp_id = id;

                    auto comp_any = meta_type.from_void(type.value(entity));
                    inspect_any(comp_any, inspector);
                    inspector.end_node();
                }
            }
            else
            {
                //All types exposed to Lua are going to have a meta type
                assert(false && "Meta-type required");
            }
        }

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

            execute(registry, c);

            // auto type = registry.storage(e.data_id);

            //entt::meta_type meta_type = c.component_meta_type;
            //entt::meta_any meta_any;

            // Component
            auto type = registry.storage(c.comp_id);
            assert(type->contains(c.entity));
            entt::meta_type meta_type = entt::resolve(c.comp_id);
            entt::meta_any meta_any = meta_type.from_void(type->value(entity));

            assert(c.meta_path.size()); // Must be at least one?
            assert(c.meta_path[0].type == MetaEntry::Type::Data); // First must be Data?            
            auto& e = c.meta_path[0];
            entt::meta_data meta_data = meta_type.data(e.data_id);
            // 1st entry (Data)
            // meta_any - component
            // meta_type - component meta type
            // meta_data - 1st data member

            // 2nd entry
            // meta_any - any for meta_data^    meta_data.get(meta_any)
            // meta_type - meta type of meta_any
            // meta_data - 2nd data member

            entt::meta_any data_any = meta_data.get(meta_any);

            std::cout << "meta_any.type().info().name() " << meta_any.type().info().name() << std::endl; // DebugClass
            std::cout << "meta_type.info().name " << meta_type.info().name() << std::endl; //DebugClass
            std::cout << "data_any.type().info().name() " << data_any.type().info().name() << std::endl; // debugvec3
            std::cout << "meta_data.type().info().name() " << meta_data.type().info().name() << std::endl; // debugvec3

            struct Property { entt::meta_any meta_any; entt::meta_data meta_data; /*entt::meta_any new_data_any;*/ };
            std::stack<Property> prop_stack;
            prop_stack.push(Property{ meta_any, meta_data });
            int i = 1;
            entt::meta_any meta_any_ = meta_any;
            entt::meta_data meta_data_ = meta_data;
            for (;i < c.meta_path.size(); i++)
            {
                auto& e = c.meta_path[i];
                if (e.type == MetaEntry::Type::Data)
                {
                    meta_any_ = meta_data_.get(meta_any_); assert(meta_any_);

                    auto meta_type = entt::resolve(meta_any_.type().id());  assert(meta_type);
                    meta_data_ = meta_type.data(e.data_id); assert(meta_data);

                    prop_stack.push(Property{ meta_any_, meta_data_ });
                } // else ...
            }
            entt::meta_any meta_any_rec = c.new_value;
            while (!prop_stack.empty())
            {
                auto& prop = prop_stack.top();
                std::cout << prop_stack.size() << " prop.meta_any " << prop.meta_any.type().info().name() << std::endl; // 
                std::cout << prop_stack.size() << " meta_any_rec " << meta_any_rec.type().info().name() << std::endl; // 
                assert(prop.meta_data.set(prop.meta_any, meta_any_rec));
                /*if (prop_stack.size() > 1)*/ meta_any_rec = prop.meta_any;
                prop_stack.pop();
            }
            std::cout << "DONE" << std::endl;
            std::cout << "prop.meta_any " << meta_any.type().info().name() << std::endl; // 
            std::cout << "meta_any_rec " << meta_any_rec.type().info().name() << std::endl; // 
            { auto fltptr = meta_any_rec.try_cast<float>(); if (fltptr) std::cout << " meta_any_rec " << *fltptr; }
            // meta_data.set(meta_any, meta_any_rec);
            meta_any.assign(meta_any_rec);
#if 0
            // entt::meta_any meta_any2;

            int i = 1;
            for (;i < c.meta_path.size(); i++)
            {
                auto& e = c.meta_path[i];

                // --> meta_any, meta_type
                if (e.type == MetaEntry::Type::Data)
                {
                    // auto meta_any_cpy = meta_any;
                    // meta_any = entt::meta_any {};
                    meta_any = meta_data.get(meta_any); assert(meta_any);
                    meta_type = entt::resolve(meta_any.type().id());  assert(meta_type);
                    meta_data = meta_type.data(e.data_id); assert(meta_data);

                    data_any = meta_data.get(meta_any);

                    // assert stuff

                    std::cout << "meta_any.type().info().name() " << meta_any.type().info().name() << std::endl; // debugvec3
                    std::cout << "meta_type.info().name " << meta_type.info().name() << std::endl; //debugvec3
                    std::cout << "data_any.type().info().name() " << data_any.type().info().name() << std::endl; // float
                    std::cout << "meta_data.type().info().name() " << meta_data.type().info().name() << std::endl; // float

                    //auto tmp_any = meta_data.get(meta_any);
                    //meta_any.assign(tmp_any);
                }
                else if (e.type == MetaEntry::Type::Index) { assert(0); }
                else if (e.type == MetaEntry::Type::Key) { assert(0); }
                else { assert(0); }
            }
            // Path followed
            // now use meta_type and meta_any to set data
            // meta_any = c.new_value;
            std::cout << "Before ";
            { auto fltptr = c.new_value.try_cast<float>(); if (fltptr) std::cout << " new " << *fltptr; }
            { auto fltptr = data_any.try_cast<float>(); if (fltptr) std::cout << " data_any " << *fltptr; }
            std::cout << std::endl;
            //meta_any.assign(c.new_value);
            assert(meta_data.set(meta_any, c.new_value));
            // meta_data.set(data_any, c.new_value);
            data_any = meta_data.get(meta_any);
            std::cout << "After  ";
            { auto fltptr = c.new_value.try_cast<float>(); if (fltptr) std::cout << " new " << *fltptr; }
            { auto fltptr = data_any.try_cast<float>(); if (fltptr) std::cout << " data_any " << *fltptr; }
            std::cout << std::endl;
#endif
        }
    }

    void inspect_registry(
        entt::meta_type name_comp_type,
        InspectorState& inspector)
    {
        auto& registry = *inspector.registry;

        auto view = registry.view<entt::entity>();
        for (auto entity : view)
        {
#if 0
            inspect_entity(entity, inspector);
#else
            auto entity_name = get_entity_name(registry, entity, name_comp_type);
            if (!inspector.begin_node(entity_name.c_str()))
                continue;

            for (auto&& [id, type] : registry.storage())
            {
                if (!type.contains(entity)) continue;

                if (entt::meta_type meta_type = entt::resolve(id); meta_type)
                {
                    auto type_name = meta_type_name(meta_type);

                    if (inspector.begin_node(type_name.c_str()))
                    {
                        auto comp_any = meta_type.from_void(type.value(entity));
                        inspect_any(comp_any, inspector);
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
    }

} // namespace Editor