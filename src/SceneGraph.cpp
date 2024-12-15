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

void SceneGraph::traverse(std::shared_ptr<entt::registry>& registry)
{
    // std::cout << "traverse:" << std::endl;
    tree.traverse_progressive([&](entt::entity* entity_ptr, entt::entity* entity_parent_ptr) {
        // + Transform = parent tfm + tfm (+ maybe their aggregate)

        // std::cout << "node " << Editor::get_entity_name(registry, entity, entt::meta_type{});
        // std::cout << ", parent " << Editor::get_entity_name(registry, entity_parent, entt::meta_type{});
        // std::cout << std::endl;

        if (!registry->template all_of<Transform>(*entity_ptr)) return;
        assert(registry->valid(*entity_ptr));
        auto& tfm_node = registry->template get<Transform>(*entity_ptr);

        if (entity_parent_ptr && registry->template all_of<Transform>(*entity_parent_ptr))
        {
            assert(registry->valid(*entity_parent_ptr)); // fix
            auto& tfm_parent = registry->template get<Transform>(*entity_parent_ptr);
            tfm_node.x_parent = tfm_parent.x_global;
            tfm_node.y_parent = tfm_parent.y_global;
            tfm_node.angle_parent = tfm_parent.angle_global;
        }

        // // opt sin/cos
        // tfm_node.x_global = tfm_node.x;
        // tfm_node.y_global = tfm_node.y;
        // tfm_node.angle_global = tfm_node.angle;
        tfm_node.x_global = tfm_node.x * cos(tfm_node.angle_parent) - tfm_node.y * sin(tfm_node.angle_parent) + tfm_node.x_parent;
        tfm_node.y_global = tfm_node.x * sin(tfm_node.angle_parent) + tfm_node.y * cos(tfm_node.angle_parent) + tfm_node.y_parent;
        tfm_node.angle_global = tfm_node.angle + tfm_node.angle_parent;
        });
}

void SceneGraph::dump_to_cout(
    const std::shared_ptr<const entt::registry>& registry,
    const entt::meta_type meta_type_with_name) const
{
    //std::cout << "Scene graph nodes:" << std::endl;
    tree.traverse_depthfirst([&](const entt::entity& entity, size_t index, size_t level)
        {
            //auto entity = node.m_payload;
            auto entity_name = Editor::get_entity_name(registry, entity, meta_type_with_name);

            auto [nbr_children, branch_stride, parent_ofs] = tree.get_node_info(entity);

            for (int i = 0; i < level; i++) std::cout << "\t";
            std::cout << " [node " << index << "]";
            std::cout << " " << entity_name //node.m_name
                << " (children " << nbr_children
                << ", stride " << branch_stride
                << ", parent ofs " << parent_ofs << ")\n";
        });
}