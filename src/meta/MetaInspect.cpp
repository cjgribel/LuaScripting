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
#include "InspectorState.hpp"
#include "meta_literals.h"
#include "meta_aux.h"

namespace Editor {

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
            assert(any.assign(selected_entry->second));
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
                        entt::meta_any data_any = meta_data.get(any);

                        // Check & set readonly
                        bool readonly = get_meta_data_prop<bool, ReadonlyDefault>(meta_data, readonly_hs);
                        if (readonly) inspector.begin_disabled();

                        inspect_any(data_any, inspector);
                        meta_data.set(any, data_any);
                        inspector.end_node();

                        // Unset readonly
                        if (readonly) inspector.end_disabled();
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
            bool res = try_apply(any, [&inspector](auto& value) {
                Editor::inspect_type(value, inspector);
                });
            if (!res)
                throw std::runtime_error(std::string("Unable to cast type ") + meta_type_name(any.type()));
        }
    }


    void inspect_entity(
        entt::entity entity,
        InspectorState& inspector)
    {
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