//
//  MetaClone.cpp
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

    bool inspect_enum_any(entt::meta_any& any, InspectorState& inspector)
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

    void inspect_any(entt::meta_any& any, InspectorState& inspector)
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
                    // JSON key name is the display name if present, or the id type
                    std::string key_name = meta_data_name(id, meta_data);

                    ImGui::SetNextItemOpen(true);
                    if (inspector.begin_node(key_name.c_str()))
                    {
                        entt::meta_any field_any = meta_data.get(any);
                        inspect_any(field_any, inspector);
                        meta_data.set(any, field_any);
                        inspector.end_node();
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
                //ImGui::PushID(count++);
                ImGui::SetNextItemWidth(-FLT_MIN);
                inspect_any(v, inspector);
                //ImGui::PopID();
                inspector.end_leaf();
            }
        }

        else if (any.type().is_associative_container())
        {
            // ImGui::Text("[is_associative_container]");

            auto view = any.as_associative_container();
            assert(view && "as_associative_container() failed");

            // JSON structure,
            // mapped container:    [[key1, val1], [key2, val2], ...]
            // set type:            [key1, key2, ...]
            // auto json_array = nlohmann::json::array();

            int count = 0;
            for (auto&& [key_any, mapped_any] : view)
            {
                //if (view.mapped_type())

                inspector.begin_leaf((std::string("#") + std::to_string(count++)).c_str());
                // ImGui::PushID(count++);
                inspect_any(key_any, inspector);
                if (view.mapped_type())
                {
                    // Store key & value as a sub-array in the container array
                    // nlohmann::json json_elem{
                    //     serialize_any(key_any), serialize_any(mapped_any)
                    // };
                    // json_array.push_back(json_elem);

                    ImGui::SetNextItemWidth(-FLT_MIN);
                    inspect_any(mapped_any, inspector);
                }
                // else
                    // Store key in the container array
                    // json_array.push_back(serialize_any(key_any));
                    // inspect_any(key_any, inspector);
                // ImGui::PopID();
                inspector.end_leaf();
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
                throw std::runtime_error(std::string("Unable to cast ") + meta_any_name(any));
        }
    }

    void inspect_registry(entt::registry& registry, InspectorState& inspector)
    {
        // nlohmann::json json;

        auto view = registry.view<entt::entity>();
        for (auto entity : view)
        {
            //std::cout << "Serializing entity "
            //<< entt::to_integral(entity) << std::endl;

            //nlohmann::json entity_json;
            //entity_json["entity"] = entt::to_integral(entity);

            auto entity_name = std::to_string(entt::to_integral(entity)).c_str();
            //std::cout << entity_name << " ";
            if (!inspector.begin_node(entity_name))
                continue;
            // ImGui::PushID(entt::to_integral(entity));

            for (auto&& [id, type] : registry.storage())
            {
                if (!type.contains(entity)) continue;

                if (entt::meta_type meta_type = entt::resolve(id); meta_type)
                {
                    // string_view ok ???

                    auto key_name = std::string{ meta_type.info().name() };// + std::string("###";
                    // std::cout << key_name.c_str() << std::endl;

                    if (inspector.begin_node(key_name.c_str()))
                    {
                        //entity_json["components"][key_name] = serialize_any(meta_type.from_void(type.value(entity)));
                        // inspect_any(meta_type.from_void(type.value(entity)));
                        auto comp_any = meta_type.from_void(type.value(entity));
                        inspect_any(comp_any, inspector);
                        inspector.end_node();
                    }
                }
                else
                {
                    // Note: all types exposed to Lua are going to have a meta type
                    assert(false && "Meta-type required");
                }
            }

            // ImGui::PopID();
            inspector.end_node();
            // }
            // json.push_back(entity_json);
        }

        // return json;
    }

} // namespace Editor