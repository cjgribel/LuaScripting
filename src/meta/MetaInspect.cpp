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
        bool mod = false;

        ImGui::Text("[is_enum]");

        // Cast to underlying meta type
        auto any_conv = cast_to_underlying_type(meta_type, any);

        auto enum_entries = gather_meta_enum_entries(any);
        // // Look for entry with current value
        auto entry = std::find_if(enum_entries.begin(), enum_entries.end(), [&any_conv](auto& e) {
            return e.second == any_conv;
            });
        assert(entry != enum_entries.end());
        // // Push entry name to json
        // json = entry->first;

        auto current = entry; //int currentItem = 2;
        if (ImGui::BeginCombo("ENUM##enum", entry->first.c_str()))
        {
            for (auto it = enum_entries.begin(); it != enum_entries.end(); it++)
            {
                const bool isSelected = (it->second == entry->second); // (currentItem == i);
                if (ImGui::Selectable(it->first.c_str(), isSelected))
                    current = it;

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        assert(any.assign(current->second));


        //             assert(json.is_string());
        //             auto entry_name = json.get<std::string>();

        //             auto enum_entries = gather_meta_enum_entries(any);
        // //            // Look for entry with a matching name
        //             const auto entry = std::find_if(enum_entries.begin(), enum_entries.end(), [&entry_name](auto& e){
        //                 return e.first == entry_name;
        //             });
        //             assert(entry != enum_entries.end());

        //             // Update any value
        // #if 1
        //             auto any_conv = entry->second.allow_cast(meta_type);
        //             bool r = any.assign(any_conv);
        // #else
        //             bool r = any.assign(entry->second);
        // #endif
        //             assert(r);

        return mod;
    }

    void inspect_any(entt::meta_any& any, InspectorState& inspector)
    {
        assert(any);
        bool mod = false;

        if (entt::meta_type meta_type = entt::resolve(any.type().id()); meta_type)
        {
            if (entt::meta_func meta_func = meta_type.func(inspect_hs); meta_func)
            {
                // Function signature: void(const nlohmann::json&, void*))
                // In this call, presumably, a meta_any is created for 'json',
                //      which and will hold a copy of it.

                // Call from_json using alias of json node
                auto res = meta_func.invoke({}, any.data(), entt::forward_as_meta(inspector));
                assert(res && "Failed to invoke inspect");

                std::cout << "inspect invoked: " << "..." << std::endl;
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

                    // std::string key_name;
                    // if (auto display_name_prop = meta_data.prop(display_name_hs); display_name_prop)
                    //     // Assume display name is in C-string format
                    //     key_name = std::string(display_name_prop.value().cast<char const*>());
                    // else
                    //     key_name = std::to_string(id);

                    ImGui::SetNextItemOpen(true);
                    if (inspector.begin_node(key_name.c_str()))
                    {
                        entt::meta_any field_any = meta_data.get(any);
                        //deserialize_any(json[key_name], field_any);
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
            ImGui::Text("[is_sequence_container]");
#if 0
            auto view = any.as_sequence_container();
            assert(view && "as_sequence_container() failed");

            //#define SEQDESER_ALT /* Resize and then deserialize in-place, seems to work also */

#ifndef SEQDESER_ALT
        // Clear before deserializing
            view.clear();
#else
        // Resize meta container to fit json array
            view.resize(json.size());
#endif
            assert(json.is_array());

            for (int i = 0; i < json.size(); i++)
            {
#ifndef SEQDESER_ALT
                entt::meta_any elem_any = view.value_type().construct();
                deserialize_any(json[i], elem_any);
                view.insert(view.end(), elem_any);
#else
                entt::meta_any elem_any = view[i]; // view[i].as_ref() works too
                deserialize_any(json[i], elem_any); // TODO: view[i] here if &&
#endif
            }
#endif
        }

        else if (any.type().is_associative_container())
        {
            ImGui::Text("[is_associative_container]");
#if 0
            auto view = any.as_associative_container();
            assert(view && "as_associative_container() failed");

            // Clear meta container before populating it with json array
            view.clear();

            // JSON structure,
            // mapped container:    [[key1, val1], [key2, val2], ...]
            // set type:            [key1, key2, ...]
            assert(json.is_array());

            for (int i = 0; i < json.size(); i++)
            {
                auto json_elem = json[i];
                if (json_elem.is_array()) // Key & mapped value
                {
                    entt::meta_any key_any = view.key_type().construct();
                    entt::meta_any mapped_any = view.mapped_type().construct();
                    deserialize_any(json_elem[0], key_any);
                    deserialize_any(json_elem[1], mapped_any);
                    view.insert(key_any, mapped_any);
                }
                else // Just key
                {
                    entt::meta_any key_any = view.key_type().construct();
                    deserialize_any(json_elem, key_any);
                    view.insert(key_any);
                }
            }
#endif
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