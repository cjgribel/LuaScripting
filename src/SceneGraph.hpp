//
//  SceneGraph.hpp
//  assimp1
//
//  Created by Carl Johan Gribel on 2021-05-18.
//  Copyright © 2021 Carl Johan Gribel. All rights reserved.
//

#ifndef SceneGraph_hpp
#define SceneGraph_hpp

#include <stdio.h>
#include <sstream>
#include <entt/entt.hpp>
// #include "vec.h"
// #include "mat.h"
#include "VecTree.h"
#include "MetaInspect.hpp"
//#include "manager.h"

// using namespace linalg;

// struct Transform
// {
//     m4f local_tfm;      // Never reset by scene graph
//     m4f global_tfm;     // Computed during traversal
//     m4f staging_tfm;    // Temporary transform - parent to local
//                         // Is reset after traversal
// };

class SceneGraph
{
public:

private:
    struct SceneGraphNode
    {
        entt::entity entity;
        std::string name;

        bool operator==(const SceneGraphNode& node) const
        {
            return entity == node.entity;
        }

        SceneGraphNode(entt::entity entity, const std::string& name)
            : entity(entity), name(name) {}
    };
    // struct SceneGraphNode : public TreeNode
    // {
    //     Handle<Transform> transform_hnd;
    //     std::string name;

    //     SceneGraphNode(const Handle<Transform> transform_hnd,
    //                    const std::string& name)
    //     : transform_hnd(transform_hnd), name(name) { }
    // };

public: // TODO: don't expose directly
    VecTree<entt::entity> tree;
    // VecTree<SceneGraphNode> tree;
    // seqtree_t<SceneGraphNode> tree;
    // TypedResourceManager<Transform> transforms;
    // const std::string root = "Scene root";

public:
    // std::string tree_dump;

    SceneGraph()
    {
        // Handle<Transform> root_hnd = transforms.create(Transform {m4f_1, m4f_1});
        // tree.insert(SceneGraphNode{root_hnd, root}, "");
    }

    bool create_node(
        entt::entity entity,
        //const m4f& local_tfm,
        //const std::string& name,
        entt::entity parent_entity = entt::null
        //const std::string& parent_name = ""
    )
    {
        if (parent_entity == entt::null)
        {
            tree.insert_as_root(entity);
            return true;
        }
        else
            return tree.insert(entity, parent_entity);

        //        if (parent.size())
        //            if (!tree.find_node(parent))
        //                return Handle<Transform>();

                // Handle<Transform> hnd = transforms.create(Transform{ local_tfm, m4f_1, m4f_1 });
                // tree.insert(SceneGraphNode{ hnd, name }, parent.size() ? parent : root);

                // dump_tree_to_string();
                // return hnd;
    }

    bool erase_node(entt::entity entity)
    {
        return tree.erase_branch(entity);
    }

    size_t size()
    {
        return tree.nodes.size();
    }

    void reset()
    {
        // for (auto& node : tree.nodes) node.transform_hnd->global_tfm = m4f_1;
    }

    void traverse(entt::registry& registry)
    {
        // SHIFT SO FUNC IS CALLED FOR PARENT???
        //
        std::cout << "traverse:" << std::endl;
        tree.traverse_progressive([&](auto& node, auto& parent, size_t node_index, size_t parent_index) {
            // + Transform = parent tfm + tfm (+ maybe their aggregate)
            // Apply parent -> node
            std::cout << "traverse node " << Editor::get_entity_name(registry, node, entt::meta_type {});
            std::cout << ", parent " << Editor::get_entity_name(registry, parent, entt::meta_type {});
            std::cout << std::endl;
            });
    }
    // CAN WE LAMBDA THIS?
    //
//     void traverse()
//     {
//         // Reset global transforms
//         //for (auto& node : tree.nodes) node.transform_hnd->global_tfm = m4f_1;

//         reset();

//         for (int node_index = 0;
//              node_index < tree.nodes.size();
//              node_index++)
//         {
//             const m4f node_tfm =
//             tree.nodes[node_index].transform_hnd->staging_tfm *
//             tree.nodes[node_index].transform_hnd->local_tfm;
// //            const m4f node_tfm = tree.nodes[node_index].transform_hnd->local_tfm;

//             const m4f global_tfm = tree.nodes[node_index].transform_hnd->global_tfm * node_tfm;
//             tree.nodes[node_index].transform_hnd->global_tfm = global_tfm;

//             int child_index = node_index+1;
//             for (int i=0; i < tree.nodes[node_index].m_nbr_children; i++)
//             {
//                 // As of now: LHS is ALWAYS identity
// //                tree.nodes[child_index].transform_hnd->global_tfm *= global_tfm;
//                 // Equivalent
//                 tree.nodes[child_index].transform_hnd->global_tfm = global_tfm;

//                 // if we do
// //                tree.nodes[child_index].transform_hnd->global_tfm = global_tfm * tree.nodes[child_index].transform_hnd->global_tfm;
//                 // then there can be "stuff" in tree.nodes[child_index].transform_hnd->global_tfm
//                 // Stuff = tfm's that parents local,
//                 // but is still child to the parent node tfm (e.g. staging_tfm ???)

//                 child_index += tree.nodes[child_index].m_branch_stride;
//             }
//         }

//         // Reset temp transforms
//         for (auto& node : tree.nodes) node.transform_hnd->staging_tfm = m4f_1;
//     }

// private:

    void dump_to_cout(
        entt::registry& registry,
        entt::meta_type meta_type_with_name)
    {
        std::cout << "Scene graph nodes:" << std::endl;
        tree.traverse_depthfirst([&](const auto& entity, size_t index, size_t level)
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
};

#endif /* SceneGraph_hpp */
