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
#include "VecTree.h"
#include "MetaInspect.hpp"


class SceneGraph
{
// public:

// private:
    // struct SceneGraphNode
    // {
    //     entt::entity entity;
    //     std::string name;

    //     bool operator==(const SceneGraphNode& node) const
    //     {
    //         return entity == node.entity;
    //     }

    //     SceneGraphNode(entt::entity entity, const std::string& name)
    //         : entity(entity), name(name) {}
    // };

public: // TODO: don't expose directly
    VecTree<entt::entity> tree;
    using BranchQueue = std::deque<entt::entity>;

    SceneGraph() = default;

    bool insert_node(
        entt::entity entity,
        entt::entity parent_entity = entt::null
    );

    bool erase_node(entt::entity entity);

    bool is_root(entt::entity entity);

    bool is_leaf(entt::entity entity);

    unsigned get_nbr_children(entt::entity entity);

    entt::entity get_parent(entt::entity entity);

    bool is_descendant_of(entt::entity entity, entt::entity parent_entity);

    void reparent(entt::entity entity, entt::entity parent_entity);

    void unparent(entt::entity entity);

    size_t size();

    // void reset();

    void traverse(std::shared_ptr<entt::registry>& registry);

    BranchQueue get_branch_topdown(entt::entity entity);

    BranchQueue get_branch_bottomup(entt::entity entity);

    // template<class F> requires std::invocable<F, PayloadType&, size_t, size_t>
    // void traverse_depthfirst(
    //     F&& func)
    //void visit_depthfirst();

    void dump_to_cout(
        const std::shared_ptr<const entt::registry>& registry,
        const entt::meta_type meta_type_with_name) const;
};

#endif /* SceneGraph_hpp */
