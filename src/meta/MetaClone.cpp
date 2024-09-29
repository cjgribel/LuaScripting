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

void clone_entity(entt::registry &registry, entt::entity src_entity, entt::entity dst_entity)
{
    for (auto&&[id, type] : registry.storage())
    {
        if(!type.contains(src_entity)) continue;
        void* src_ptr = type.value(src_entity);
        
        // Use clone() if it exists
        if (entt::meta_type meta_type = entt::resolve(id); meta_type)
        {
            if (entt::meta_func meta_func = meta_type.func(clone_hs); meta_func)
            {
                std::cout << "Invoking clone() for " << type.type().name() << std::endl;
                
                auto copy_any = meta_func.invoke({}, src_ptr, dst_entity);
                assert(copy_any && "Failed to invoke clone() for type ");
                
                type.push(dst_entity, copy_any.data());
                continue;
            }
        }
        
        // Fallback: Direct copy if no meta type or clone function
        type.push(dst_entity, src_ptr);
    }
}

} // namespace Editor