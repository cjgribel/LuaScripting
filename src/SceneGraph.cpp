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

void SceneGraph::traverse(entt::registry& registry)
{
    // SHIFT SO FUNC IS CALLED FOR PARENT???
    //
    std::cout << "traverse:" << std::endl;
    tree.traverse_progressive([&](auto& entity, auto& entity_parent, size_t node_index, size_t parent_index) {
        // + Transform = parent tfm + tfm (+ maybe their aggregate)
        // Apply parent -> node
        std::cout << "traverse node " << Editor::get_entity_name(registry, entity, entt::meta_type{});
        std::cout << ", parent " << Editor::get_entity_name(registry, entity_parent, entt::meta_type{});
        std::cout << std::endl;

        // if (!registry.all_of<Transform>(entity)) return;
        // if (!registry.all_of<Transform>(entity_parent)) return;
        // auto& tfm_node = registry.get<Transform>(entity);
        // auto& tfm_parent = registry.get<Transform>(entity_parent);

        });
}