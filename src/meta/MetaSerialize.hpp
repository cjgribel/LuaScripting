//
//  MetaSerialize.hpp
//
//  Created by Carl Johan Gribel on 2024-08-08.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef MetaSerialize_hpp
#define MetaSerialize_hpp

#include <entt/entt.hpp>
// #include <nlohmann/json_fwd.hpp> // Use nlohmann::json& as references instead
#include <nlohmann/json.hpp>

namespace Meta {

    nlohmann::json serialize_any(const entt::meta_any& meta_any);

    nlohmann::json serialize_registry(entt::registry& registry);

    void deserialize_any(const nlohmann::json& object_json, entt::meta_any& meta_any);

    void deserialize_registry(const nlohmann::json& json, entt::registry& registry);

} // namespace Meta

#endif /* MetaSerialize_hpp */
