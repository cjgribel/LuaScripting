//
//  MetaClone.hpp
//  engine_core_2024
//
//  Created by Carl Johan Gribel on 2024-08-08.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef MetaInspect_hpp
#define MetaInspect_hpp

#include <entt/entt.hpp>

namespace Editor {

    struct InspectorState;

    bool inspect_enum_any(entt::meta_any& any, InspectorState& inspector);

    void inspect_any(entt::meta_any& any, InspectorState& inspector);

    void inspect_registry(entt::registry& registry, entt::meta_type comp_with_name, InspectorState& inspector);

}

#endif /* MetaSerialize_hpp */
