//
//  MetaSerialize.hpp
//
//  Created by Carl Johan Gribel on 2024-08-08.
//  Copyright © 2024 Carl Johan Gribel. All rights reserved.
//

#ifndef MetaSerialize_hpp
#define MetaSerialize_hpp

#include <entt/entt.hpp>
//#include <nlohmann/json_fwd.hpp> // Use nlohmann::json& as references instead
#include <nlohmann/json.hpp>

#include "Context.hpp"

namespace Meta {

    nlohmann::json serialize_any(
        const entt::meta_any& meta_any);

    // nlohmann::json serialize_entity(
    //     entt::entity,
    //     std::shared_ptr<entt::registry>& registry);

    nlohmann::json serialize_entities(
        entt::entity* entity_first,
        int count,
        std::shared_ptr<entt::registry>& registry);

    nlohmann::json serialize_registry(
        std::shared_ptr<entt::registry>& registry);

    // void deserialize_any(
    //     const nlohmann::json& json,
    //     entt::meta_any& meta_any,
    //     entt::entity entity,
    //     Editor::Context& context);

    // entt::entity deserialize_entity(
    //     const nlohmann::json& json,
    //     Editor::Context& context
    // );

    void deserialize_entities(
        const nlohmann::json& json,
        Editor::Context& context);

} // namespace Meta

#endif /* MetaSerialize_hpp */
