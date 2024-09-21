//
//  SceneGraph.cpp
//  assimp1
//
//  Created by Carl Johan Gribel on 2021-05-18.
//  Copyright © 2021 Carl Johan Gribel. All rights reserved.
//

#include <stdio.h>
#include "SceneGraph.hpp"
//#include "transform.hpp"
#include "CoreComponents.hpp"

void SceneGraph::traverse(entt::registry& registry)
{
    // SHIFT SO FUNC IS CALLED FOR PARENT???
    //
    // std::cout << "traverse:" << std::endl;
    tree.traverse_progressive([&](const auto& entity, const auto& entity_parent) {
        // + Transform = parent tfm + tfm (+ maybe their aggregate)
    
        // std::cout << "node " << Editor::get_entity_name(registry, entity, entt::meta_type{});
        // std::cout << ", parent " << Editor::get_entity_name(registry, entity_parent, entt::meta_type{});
        // std::cout << std::endl;

        if (!registry.all_of<Transform>(entity)) return;
        assert(registry.valid(entity));
        auto& tfm_node = registry.get<Transform>(entity);

        if (entity_parent != entt::null && registry.all_of<Transform>(entity_parent))
        {
            assert(registry.valid(entity_parent)); // fix
            auto& tfm_parent = registry.get<Transform>(entity_parent);
            tfm_node.x_parent = tfm_parent.x_global;
            tfm_node.y_parent = tfm_parent.y_global;
            tfm_node.rot_parent = tfm_parent.rot_global;
        }

        // // opt sin/cos
        // tfm_node.x_global = tfm_node.x;
        // tfm_node.y_global = tfm_node.y;
        // tfm_node.rot_global = tfm_node.rot;
        tfm_node.x_global = tfm_node.x * cos(tfm_node.rot_parent) - tfm_node.y * sin(tfm_node.rot_parent) + tfm_node.x_parent;
        tfm_node.y_global = tfm_node.x * sin(tfm_node.rot_parent) + tfm_node.y * cos(tfm_node.rot_parent) + tfm_node.y_parent;
        tfm_node.rot_global = tfm_node.rot + tfm_node.rot_parent;
        });
}