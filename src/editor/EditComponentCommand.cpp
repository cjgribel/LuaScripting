//
//  EditComponentCommand.cpp
//
//  Created by Carl Johan Gribel on 2024-10-14.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#include <iostream>
// #include <sstream>
// #include <cassert>
// #include "imgui.h"
// #include "MetaInspect.hpp"
// #include "InspectType.hpp"
#include "meta_literals.h"
// #include "meta_aux.h"

#include "EditComponentCommand.hpp"

namespace Editor {

    void ComponentCommand::traverse_and_set_meta_type(entt::meta_any& value_any)
    {
        using EntryType = MetaPath::Entry::Type;
        assert(!registry.expired());
        auto registry_sp = registry.lock();

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

} // namespace Editor