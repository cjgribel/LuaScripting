//
//  MetaSerialize.cpp
//
//  Created by Carl Johan Gribel on 2024-08-08.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <cassert>
// #include <nlohmann/json_fwd.hpp> //
#include <nlohmann/json.hpp>
#include "config.h"
#include "MetaSerialize.hpp"
#include "meta_literals.h"
#include "meta_aux.h"

namespace Meta {

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
#ifdef SERIALIZATION_DEBUG_PRINTS
                std::cout << "to_json invoked: " << json.dump() << std::endl;
#endif
            }
            else if (meta_type.is_enum())
            {
                // std::cout << " [is_enum]";

                // Cast to underlying meta type
                auto any_conv = cast_to_underlying_type(meta_type, any);

                auto enum_entries = gather_meta_enum_entries(any);
                // Look for entry with current value
                auto entry = std::find_if(enum_entries.begin(), enum_entries.end(), [&any_conv](auto& e) {
                    return e.second == any_conv;
                    });
                assert(entry != enum_entries.end());
                // Push entry name to json
                json = entry->first;
            }
            else
            {
                // to_json() not available: traverse data members
                for (auto&& [id, meta_data] : meta_type.data())
                {
                    // JSON key name is the display name if present, or the id type
                    std::string key_name = meta_data_name(id, meta_data);

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
                throw std::runtime_error(std::string("Unable to cast ") + meta_type_name(any.type()));
        }

        return json;
    }

    nlohmann::json serialize_entity(
        entt::entity entity,
        std::shared_ptr<entt::registry>& registry)
    {
        std::cout << "Serializing entity "
            << entt::to_integral(entity) << std::endl;

        nlohmann::json entity_json;
        entity_json["entity"] = entt::to_integral(entity);

        // For all component types
        for (auto&& [id, type] : registry->storage())
        {
            if (!type.contains(entity)) continue;

            if (entt::meta_type meta_type = entt::resolve(id); meta_type)
            {
                auto key_name = std::string{ meta_type.info().name() }; // Better for serialization?
                // auto type_name = meta_type_name(meta_type); // Inspector-friendly version

                entity_json["components"][key_name] = serialize_any(meta_type.from_void(type.value(entity)));
            }
            else
            {
                assert(false && "No meta type for component");
            }
        }
        return entity_json;
    }

    nlohmann::json serialize_registry(std::shared_ptr<entt::registry>& registry)
    {
        nlohmann::json json;

        auto view = registry->template view<entt::entity>();
        for (auto entity : view)
        {
#if 1
            json.push_back(serialize_entity(entity, registry));
#else
            std::cout << "Serializing entity "
                << entt::to_integral(entity) << std::endl;

            nlohmann::json entity_json;
            entity_json["entity"] = entt::to_integral(entity);

            // For all component types
            for (auto&& [id, type] : registry->storage())
            {
                if (!type.contains(entity)) continue;

                if (entt::meta_type meta_type = entt::resolve(id); meta_type)
                {
                    auto key_name = std::string{ meta_type.info().name() }; // Better for serialization?
                    // auto type_name = meta_type_name(meta_type); // Inspector-friendly version

                    entity_json["components"][key_name] = serialize_any(meta_type.from_void(type.value(entity)));
                }
                else
                {
                    assert(false && "Meta-type required");
                }
            }
            json.push_back(entity_json);
#endif
        }

        return json;
    }

    void deserialize_any(
        const nlohmann::json& json,
        entt::meta_any& any,
        entt::entity entity,
        Editor::Context& context)
    {
        assert(any);

        if (entt::meta_type meta_type = entt::resolve(any.type().id()); meta_type)
        {
            if (entt::meta_func meta_func = meta_type.func(from_json_hs); meta_func)
            {
                // Function signature: void(const nlohmann::json&, void*))
                // In this call, presumably, a meta_any is created for 'json',
                //      which and will hold a copy of it.
#if 1
            // Call from_json using alias of json node
                // auto res = meta_func.invoke({}, entt::forward_as_meta(json), any.data());
                auto res = meta_func.invoke(
                    {},
                    entt::forward_as_meta(json),
                    any.data(),
                    entity,
                    entt::forward_as_meta(context));
#else
            // json node is possibly copied to an entt::meta_any here
                auto res = meta_func.invoke({}, json, any.data());
#endif
                assert(res && "Failed to invoke from_json");

                std::cout << "from_json invoked: " << json.dump() << std::endl;
            }
            else if (meta_type.is_enum())
            {
                std::cout << " [is_enum]";

                assert(json.is_string());
                auto entry_name = json.get<std::string>();

                auto enum_entries = gather_meta_enum_entries(any);
                //            // Look for entry with a matching name
                const auto entry = std::find_if(enum_entries.begin(), enum_entries.end(), [&entry_name](auto& e) {
                    return e.first == entry_name;
                    });
                assert(entry != enum_entries.end());

                // Update any value
#if 1
                auto any_conv = entry->second.allow_cast(meta_type);
                bool r = any.assign(any_conv);
#else
                bool r = any.assign(entry->second);
#endif
                assert(r);
            }
            else
            {
                // from_json() not available: traverse data members
                for (auto&& [id, meta_data] : meta_type.data())
                {
                    // JSON key name is the display name if present, or the id type
                    std::string key_name = meta_data_name(id, meta_data);

                    entt::meta_any field_any = meta_data.get(any);
                    deserialize_any(json[key_name], field_any, entity, context);
                    meta_data.set(any, field_any);
                }
            }
            return;
        }

        // any is not a meta type

        if (any.type().is_sequence_container())
        {
            auto view = any.as_sequence_container();
            assert(view && "as_sequence_container() failed");

            // Resize and then deserialize in-place
            // Note: This approach is necessary to make fixed-size containers without insertion to work (std::array)
#define SEQDESER_ALT

#ifndef SEQDESER_ALT
            // Clear before adding deserialized elements one by one
            view.clear();
#else
            // Resize meta container to fit json array
            // As expected, seems to do nothing for fixed-size containers such as std::array
            view.resize(json.size());
#endif
            assert(json.is_array());

            for (int i = 0; i < json.size(); i++)
            {
#ifndef SEQDESER_ALT
                entt::meta_any elem_any = view.value_type().construct();
                deserialize_any(json[i], elem_any, entity, context);
                view.insert(view.end(), elem_any);
#else
                entt::meta_any elem_any = view[i]; // view[i].as_ref() works too
                deserialize_any(json[i], elem_any, entity, context); // TODO: view[i] here if &&
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
                    deserialize_any(json_elem[0], key_any, entity, context);
                    deserialize_any(json_elem[1], mapped_any, entity, context);
                    view.insert(key_any, mapped_any);
                }
                else // Just key
                {
                    entt::meta_any key_any = view.key_type().construct();
                    deserialize_any(json_elem, key_any, entity, context);
                    view.insert(key_any);
                }
            }
        }

        else
        {
            // Try casting the meta_any to a primitive type.
            //
            bool res = try_apply(any, [&json, &any](auto& value) {
                using Type = std::decay_t<decltype(value)>;
                // Assign a new value to the stored object
                // Note: any = json.get<Type>() *replaces* the stored object
                any.assign(json.get<Type>());
                });
            if (!res)
                throw std::runtime_error(std::string("Unable to cast ") + meta_type_name(any.type()));
        }
    }

    void deserialize_entity(
        const nlohmann::json& json,
        Editor::Context& context
    )
    {
        assert(json.contains("entity"));
        entt::entity entity_hint = json["entity"].get<entt::entity>();
        assert(!context.registry->valid(entity_hint));
        auto entity = context.registry->create(entity_hint);
        assert(entity_hint == entity);

        std::cout << "Deserializing entity " << entt::to_integral(entity) << std::endl;

        assert(json.contains("components"));
        for (const auto& component_json : json["components"].items())
        {
            auto key_str = component_json.key().c_str();
            auto id = entt::hashed_string::value(key_str);

            if (entt::meta_type meta_type = entt::resolve(id); meta_type)
            {
                // Default-construct component component
                entt::meta_any any = meta_type.construct();
                // Deserialize component
                deserialize_any(component_json.value(), any, entity, context);
                // Add component to entity storage
                context.registry->storage(id)->push(entity, any.data());
            }
            else
            {
                assert(false && "Deserialized component is not a meta-type");
            }
        }
    }

    void deserialize_registry(
        const nlohmann::json& json,
        Editor::Context& context)
    {
        assert(json.is_array());
        for (const auto& entity_json : json)
        {
#if 1
            deserialize_entity(entity_json, context);
#else
            assert(entity_json.contains("entity"));
            entt::entity entity_hint = entity_json["entity"].get<entt::entity>();
            assert(!context.registry->valid(entity_hint));
            auto entity = context.registry->create(entity_hint);
            assert(entity_hint == entity);
            //
            // if (entity_hint != entity) {
            //     assert(context.registry->valid(entity));
            //     assert(!context.registry->valid(entity_hint));
            //     auto view = context.registry->template view<entt::entity>();
            //     bool f = false;
            //     for (auto entity : view) f |= (entity == entity_hint);
            //     std::cout << "entity_hint != entity, entity_hint exists: " << f << std::endl;
            //     assert(0);
            // }

            std::cout << "Deserializing entity " << entt::to_integral(entity) << std::endl;

            assert(entity_json.contains("components"));
            for (const auto& component_json : entity_json["components"].items())
            {
                auto key_str = component_json.key().c_str();
                auto id = entt::hashed_string::value(key_str);

                if (entt::meta_type meta_type = entt::resolve(id); meta_type)
                {
                    // Default-construct component component
                    entt::meta_any any = meta_type.construct();
                    // Deserialize component
                    deserialize_any(component_json.value(), any, entity, context);
                    // Add component to entity storage
                    context.registry->storage(id)->push(entity, any.data());
                }
                else
                {
                    assert(false && "Deserialized component is not a meta-type");
                }
            }
#endif
        }
    }

} // namespace Meta