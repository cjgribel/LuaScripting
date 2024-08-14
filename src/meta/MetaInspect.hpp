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
//#include <nlohmann/json.hpp> // Prefer <nlohmann/json_fwd.hpp>

//nlohmann::json serialize_any(const entt::meta_any& meta_any);

//nlohmann::json serialize_registry(entt::registry& registry);

void inspect_any(entt::meta_any& any);

void inspect_registry_(entt::registry& registry);

#endif /* MetaSerialize_hpp */
