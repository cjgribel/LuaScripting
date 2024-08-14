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
#include "meta_literals.h"
#include "meta_aux.h"

// WHERE?
//
namespace Inspector {

    template<class T>
    bool inspect_type(T& t)
    {
        ImGui::Text("Widget not implemented");
        return false;
    }

    /// inspect float
    template<class T>
        requires std::is_same_v<T, float>
//        requires std::is_floating_point_v<T> // also matches double
    bool inspect_type(T& t)
    {
        return ImGui::InputFloat("", &t, 1.0f);
    }

    /// inspect int
    template<class T>
    requires std::is_same_v<T, int>
        // requires (std::is_integral_v<T> && !std::is_unsigned_v<T>) // also matches e.g. char
    bool inspect_type(T& t)
    {
        return ImGui::InputInt("", &t, 1);
    }
}

struct
{
    entt::entity primary_entity;
    // int id = 0;
    // void push_id()
    // {
    //     ImGui::PushID(id++);
    // }
    // void pop_id()
    // {
    //     ImGui::PopID();
    // }
    void begin_leaf(const char* label)
    {
        row();
        //ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
        ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
        next_column();
        // push_id();
        ImGui::SetNextItemWidth(-FLT_MIN);
    }
    void end_leaf()
    {
        // pop_id();
    }
    bool begin_node(const char* label)
    {
        row();
        bool open = ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_SpanFullWidth);
        if (open) {
            next_column();
            // push_id();
            ImGui::SetNextItemWidth(-FLT_MIN);
        }
        return open;
    }
    void end_node()
    {
        ImGui::TreePop();
        // pop_id();
    }
    void row()
    {
        ImGui::TableNextRow();
        next_column();
    }
    void next_column()
    {
        ImGui::TableNextColumn();
    }
} w{ entt::entity {} };

#if 0
nlohmann::json serialize_any(const entt::meta_any& any)
{
    assert(any);
    nlohmann::json json;

    if (entt::meta_type meta_type = entt::resolve(any.type().id()); meta_type)
    {
        if (entt::meta_func meta_func = meta_type.func(to_json_hs); meta_func)
        {
            // Note: invoking the meta function with json
            // (Underlying function: void(nlohmann::json&, const void*))
            // Using 'json': invoke by value; only the copy is modified
            // Using 'std::ref(json)': function is not called – perhaps
            //      'invoke' does not interpret the std::reference_wrapper
            //      returned from std::ref in a way that it matched the
            //      json& argument.
            // Using 'json_any.as_ref()': seems to forward correctly as json&

#if 1
            // Calls to_json using an alias of the json node (without copying)
            auto res = meta_func.invoke({}, entt::forward_as_meta(json), any.data());
            assert(res && "Failed to invoke to_json");
#else
            // Copies json node, calls to_json (via alias), then copies back
            entt::meta_any json_any = json;
            auto res = meta_func.invoke({}, json_any.as_ref(), any.data());
            assert(res && "Failed to invoke to_json");
            json = json_any.cast<nlohmann::json>();
#endif
            std::cout << "to_json invoked: " << json.dump() << std::endl;
        }
        else
        {
            // to_json() not available: traverse data members
            for (auto&& [id, meta_data] : meta_type.data())
            {
                std::string key_name;
                if (auto display_name_prop = meta_data.prop(display_name_hs); display_name_prop)
                    // Assume display name is in C-string format
                    key_name = std::string(display_name_prop.value().cast<char const*>());
                else
                    key_name = std::to_string(id);

                auto field_any = meta_data.get(any);
                json[key_name] = serialize_any(field_any);
            }
        }

        return json;
    }

    // any is not a meta type

    if (any.type().is_sequence_container())
    {
        auto view = any.as_sequence_container();
        assert(view && "as_sequence_container() failed");

        auto json_array = nlohmann::json::array();
        for (auto&& v : view)
        {
            json_array.push_back(serialize_any(v));
        }
        json = json_array;
    }
    else if (any.type().is_associative_container())
    {
        auto view = any.as_associative_container();
        assert(view && "as_associative_container() failed");

        // JSON structure,
        // mapped container:    [[key1, val1], [key2, val2], ...]
        // set type:            [key1, key2, ...]
        auto json_array = nlohmann::json::array();

        for (auto&& [key_any, mapped_any] : view)
        {
            if (view.mapped_type())
            {
                // Store key & value as a sub-array in the container array
                nlohmann::json json_elem{
                    serialize_any(key_any), serialize_any(mapped_any)
                };
                json_array.push_back(json_elem);
            }
            else
                // Store key in the container array
                json_array.push_back(serialize_any(key_any));
        }
        json = json_array;
    }
    else
    {
        bool res = try_apply(any, [&json](auto& value) {
            json = value;
            });
        if (!res)
            throw std::runtime_error(std::string("Unable to cast ") + any_name(any));
    }

    return json;
}

nlohmann::json serialize_registry(entt::registry& registry)
{
    nlohmann::json json;

    auto view = registry.view<entt::entity>();
    for (auto entity : view)
    {
        std::cout << "Serializing entity "
            << entt::to_integral(entity) << std::endl;

        nlohmann::json entity_json;
        entity_json["entity"] = entt::to_integral(entity);

        for (auto&& [id, type] : registry.storage())
        {
            if (!type.contains(entity)) continue;

            if (entt::meta_type meta_type = entt::resolve(id); meta_type)
            {
                // string_view ok ???
                auto key_name = std::string{ meta_type.info().name() };

                entity_json["components"][key_name] = serialize_any(meta_type.from_void(type.value(entity)));
            }
            else
            {
                assert(false && "Meta-type required");
            }
        }
        json.push_back(entity_json);
    }

    return json;
}
#endif

void inspect_any(entt::meta_any& any)
{
    assert(any);

    if (entt::meta_type meta_type = entt::resolve(any.type().id()); meta_type)
    {
        if (entt::meta_func meta_func = meta_type.func(inspect_hs); meta_func)
        {
            // Function signature: void(const nlohmann::json&, void*))
            // In this call, presumably, a meta_any is created for 'json',
            //      which and will hold a copy of it.

            // Call from_json using alias of json node
            auto res = meta_func.invoke({}, any.data());
            assert(res && "Failed to invoke from_json");

            std::cout << "inspect invoked: " << "..." << std::endl;
        }
        else
        {
            // inspect() not available: traverse data members
            for (auto&& [id, meta_data] : meta_type.data())
            {
                std::string key_name;
                if (auto display_name_prop = meta_data.prop(display_name_hs); display_name_prop)
                    // Assume display name is in C-string format
                    key_name = std::string(display_name_prop.value().cast<char const*>());
                else
                    key_name = std::to_string(id);

                if (w.begin_node(key_name.c_str()))
                {
                    entt::meta_any field_any = meta_data.get(any);
                    //deserialize_any(json[key_name], field_any);
                    inspect_any(field_any);
                    meta_data.set(any, field_any);
                    w.end_node();
                }
            }
        }
        return;
    }

    // any is not a meta type
#if 0
    if (any.type().is_sequence_container())
    {
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
    }

    else if (any.type().is_associative_container())
    {
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
    }

    else
    #endif
    {
        // Try casting the meta_any to a primitive type.
        //
        bool res = try_apply(any, [](auto& value) {
            //using Type = std::decay_t<decltype(value)>;
            // Assign a new value to the stored object
            // Note: any = json.get<Type>() *replaces* the stored object
            //any.assign(json.get<Type>());
            //w.begin_leaf("");
            Inspector::inspect_type(value);
            //w.end_leaf();
            });
        if (!res)
            throw std::runtime_error(std::string("Unable to cast ") + any_name(any));
    }
}

void inspect_registry_(entt::registry& registry)
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
        if (!w.begin_node(entity_name))
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

                if (w.begin_node(key_name.c_str()))
                {
                    //entity_json["components"][key_name] = serialize_any(meta_type.from_void(type.value(entity)));
                    // inspect_any(meta_type.from_void(type.value(entity)));
                    auto comp_any = meta_type.from_void(type.value(entity));
                    inspect_any(comp_any);
                    w.end_node();
                }
            }
            else
            {
                // Note: all types exposed to Lua are going to have a meta type
                assert(false && "Meta-type required");
            }
        }

        // ImGui::PopID();
        w.end_node();
        // }
        // json.push_back(entity_json);
    }

    // return json;
}

