//
//  VecTree.h
//
//  Created by Carl Johan Gribel 2024
//
//

#ifndef VecTree_h
#define VecTree_h

#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <cassert>

#define VecTree_NullIndex -1

template <typename T>
concept EqualityComparable = requires(T a, T b) {
    { a == b } -> std::convertible_to<bool>;
};

template<class T>
struct TreeNode
{
    unsigned m_nbr_children = 0;    // Nbr of children
    unsigned m_branch_stride = 1;   // Branch size including this node
    unsigned m_parent_ofs = 0;      // Distance to parent, relative parent.
    T m_payload;                    // Payload
};

/**
Sequential tree representation optimized for depth-first traversal.
Nodes are organized in pre-order, which means that the first child of a node is located directly after the node.
Each node has information about number children, branch stride and parent offset.
The tree can be traversed is different ways.
*/
template <class PayloadType>
    requires EqualityComparable<PayloadType>
class VecTree
{
public:
    using TreeNodeType = TreeNode<PayloadType>;
    std::vector<TreeNodeType> nodes;

    VecTree() = default;

    /// @brief Find index of a node O(N)
    /// @param payload Payload to search for
    /// @return Index Node index
    size_t find_node_index(const PayloadType& payload) const
    {
        auto it = std::find_if(nodes.begin(), nodes.end(),
            [&payload](const TreeNodeType& node)
            {
                return payload == node.m_payload;
            });
        if (it == nodes.end()) return VecTree_NullIndex;
        return std::distance(nodes.begin(), it);
    }

    inline size_t size() const
    {
        return nodes.size();
    }

    bool contains(const PayloadType& payload) const
    {
        return find_node_index(payload) != VecTree_NullIndex;
    }

    // Used for debug pritning...
    auto get_node_info(const PayloadType& payload) const
    {
        auto index = find_node_index(payload);
        assert(index != VecTree_NullIndex);
        const auto& node = nodes[index];
        return std::make_tuple(node.m_nbr_children, node.m_branch_stride, node.m_parent_ofs);
    }

    // For inspection (and transform traversal?)
    auto get_node_info_at(size_t index) const
    {
        assert(index != VecTree_NullIndex);
        const auto& node = nodes[index];
        return std::make_tuple(node.m_payload, node.m_nbr_children, node.m_branch_stride, node.m_parent_ofs);
    }

    auto get_branch_stride(const PayloadType& payload)
    {
        auto index = find_node_index(payload);
        assert(index != VecTree_NullIndex);
        return nodes[index].m_branch_stride;
    }

    auto get_parent_ofs(const PayloadType& payload)
    {
        auto index = find_node_index(payload);
        assert(index != VecTree_NullIndex);
        return nodes[index].m_parent_ofs;
    }

    bool is_root(const PayloadType& payload) const
    {
        auto [nbr_children, branch_stride, parent_ofs] = get_node_info(payload);
        return parent_ofs == 0;
    }

    bool is_leaf(const PayloadType& payload) const
    {
        auto [nbr_children, branch_stride, parent_ofs] = get_node_info(payload);
        return nbr_children == 0;
    }

    size_t get_parent_index(const PayloadType& payload) const
    {
        auto node_index = find_node_index(payload);
        assert(node_index != VecTree_NullIndex);
        // Error if root
        assert(nodes[node_index].m_parent_ofs);

        return node_index - nodes[node_index].m_parent_ofs;
    }

    PayloadType& get_parent(const PayloadType& payload) const
    {
        return nodes[get_parent_index(payload)].m_payload;
    }

    PayloadType& get_parent(const PayloadType& payload)
    {
        return nodes[get_parent_index(payload)].m_payload;
    }

    bool is_descendant_of(entt::entity entity_child, entt::entity entity_parent)
    {
        bool is_child = false;
        ascend(entity_child, [&](auto& entity, size_t index) {
            if (entity == entity_child) return;
            if (entity == entity_parent) is_child = true;
            });
        return is_child;
    }

    void reparent(const PayloadType& payload, const PayloadType& parent_payload)
    {
        assert(!is_descendant_of(parent_payload, payload));
        auto node_index = find_node_index(payload);
        auto node_branch_stride = get_branch_stride(payload);

        // Move branch to a temporary buffer
        std::deque<TreeNodeType> branch;
        branch.resize(node_branch_stride);
        std::move(
            nodes.begin() + node_index,
            nodes.begin() + node_index + node_branch_stride,
            branch.begin()
        );

        // Remove branch from its current parent
        erase_branch(payload);

        // Reinsert branch under new parent
        insert(branch.front().m_payload, parent_payload);
        for (size_t i = 1; i < branch.size(); i++)
        {
            auto& node = branch[i];
            auto& parent_node = branch[i - node.m_parent_ofs];
            insert(node.m_payload, parent_node.m_payload);
        }
    }

    void unparent(const PayloadType& payload)
    {
        auto node_index = find_node_index(payload);
        auto node_branch_stride = get_branch_stride(payload);

        // Move branch to a temporary buffer
        std::deque<TreeNodeType> branch;
        branch.resize(node_branch_stride);
        std::move(
            nodes.begin() + node_index,
            nodes.begin() + node_index + node_branch_stride,
            branch.begin()
        );

        // Remove branch from its current parent
        erase_branch(payload);

        // Reinsert branch under new parent
        insert_as_root(branch.front().m_payload);
        for (size_t i = 1; i < branch.size(); i++)
        {
            auto& node = branch[i];
            auto& parent_node = branch[i - node.m_parent_ofs];
            insert(node.m_payload, parent_node.m_payload);
        }
    }

    void insert_as_root(const PayloadType& payload)
    {
        nodes.insert(nodes.end(), TreeNodeType{ .m_payload = payload });
    }

    /// @brief Insert a node
    /// @param payload Payload to insert
    /// @param parent_payload Payload of parent node.
    /// @return True if insertion was successfull, false otherwise
    bool insert(
        const PayloadType& payload,
        const PayloadType& parent_payload
    )
    {
        auto node = TreeNodeType{ .m_payload = payload };

        // No parent given - insert as root
        // if (!parent_name.size())
        // {
        //     nodes.insert(nodes.begin(), node);
        //     return true;
        // }

        // Locate parent
        auto parent_index = find_node_index(parent_payload);
        if (parent_index == VecTree_NullIndex)
            return false;
        auto pit = nodes.begin() + parent_index;

        // Update branch strides within the same branch
        // Iterate backwards from parent to the root and increment strides
        // ranging to the insertion
        //
        auto prit = pit;
        while (prit >= nodes.begin())
        {
            if (prit->m_branch_stride > (unsigned)std::distance(prit, pit))
                prit->m_branch_stride++;
            // discontinue past root (preceeding trees not affected)
            if (!prit->m_parent_ofs)
                break;
            prit--;
        }

        // Update parent offsets
        // Iterate forward up until the next root and increment parent ofs'
        // ranging to the insertion
        //
        auto pfit = pit + 1;
        while (pfit < nodes.end())
        {
            if (!pfit->m_parent_ofs) // discontinue at succeeding root
                break;
            if (pfit->m_parent_ofs >= (unsigned)std::distance(pit, pfit))
                pfit->m_parent_ofs++;
            pfit++;
        }

        // Increment parent's nbr of children
        pit->m_nbr_children++;
        // Insert new node after parent
        node.m_parent_ofs = 1;
        nodes.insert(pit + 1, node);

        return true;
    }

    /// @brief Erase a node and its branch
    /// @param payload Payload of the node to erase
    /// @return True if erasure was successfull, false otherwise
    bool erase_branch(const PayloadType& payload)
    {
        auto node_index = find_node_index(payload);
        if (node_index == VecTree_NullIndex) return false;

        auto& node = nodes[node_index];
        //auto nbr_children = node.m_nbr_children;
        auto branch_stride = node.m_branch_stride;

        // Parent node
        auto pit = nodes.begin() + (node_index - node.m_parent_ofs);

        // From parent and up the tree, 
        // update branch strides that range over the node
        //
        auto prit = pit;
        while (prit >= nodes.begin())
        {
            // Check if branch stride is within range
            if (prit->m_branch_stride > (unsigned)std::distance(prit, pit))
                prit->m_branch_stride -= branch_stride;
            // Break at root
            if (!prit->m_parent_ofs)
                break;
            prit--;
        }

        // From after the node's branch, 
        // update parent offsets that range backward past the enode
        //
        auto pfit = nodes.begin() + node_index + branch_stride;
        //auto pfit = pit + branch_stride; // pit + 1;
        while (pfit < nodes.end())
        {
            if (!pfit->m_parent_ofs) // discontinue at succeeding root
                break;
            if (pfit->m_parent_ofs >= (unsigned)std::distance(pit, pfit))
                pfit->m_parent_ofs -= branch_stride;
            pfit++;
        }

        pit->m_nbr_children--;
        nodes.erase(nodes.begin() + node_index,
            nodes.begin() + node_index + branch_stride);
        return true;
    }

    /// @brief Traverse depth-first in a per-level manner
    /// @param node_name Name of node to descend from
    /// Useful for hierarchical transformations. The tree is optimized for this type of traversal.
    /// F is a function of type void(PayloadType& node, PayloadType& parent, size_t node_index, size_t parent_index)
    template<class F>
        requires std::invocable<F, PayloadType*, PayloadType*>
    void traverse_progressive(
        size_t start_index,
        const F& func)
    {
        //auto start_node_index = find_node_index(payload);
        // assert(start_node_index != VecTree_NullIndex);
        assert(start_index >= 0 && start_index < size());

        for (int i = 0; i < nodes[start_index].m_branch_stride; i++)
        {
            auto node_index = start_index + i;
            auto& node = nodes[node_index];

            if (!node.m_parent_ofs)
                func(&node.m_payload, nullptr);

            size_t child_index = node_index + 1;
            for (int j = 0; j < node.m_nbr_children; j++)
            {
                func(&nodes[child_index].m_payload, &node.m_payload);
                child_index += nodes[child_index].m_branch_stride;
            }
        }
    }

    template<class F>
        requires std::invocable<F, PayloadType*, PayloadType*>
    void traverse_progressive(
        const PayloadType& payload,
        const F& func)
    {
        auto index = find_node_index(payload);
        assert(index != VecTree_NullIndex);
        traverse_progressive(index, func);
    }

    template<class F>
        requires std::invocable<F, PayloadType*, PayloadType*>
    void traverse_progressive(
        const F& func)
    {
        // if (size())
        //     traverse_progressive(0, func);

        size_t i = 0;
        while (i < size())
        {
            traverse_progressive(i, func);
            i += nodes[i].m_branch_stride;
        }
    }

    /// @brief Traverse tree depth-first without level information
    /// @param node_name Name of node to descend from
    /// The tree is optimized for this type of traversal.
    /// F is a function of type void(PayloadType&, size_t), where the second argument is node index
    template<class F>
        requires std::invocable<F, PayloadType&, size_t>
    void traverse_depthfirst(
        size_t node_index,
        const F& func)
    {
        // auto node_index = find_node_index(node_name);
        assert(node_index != VecTree_NullIndex);
        assert(node_index >= 0);
        assert(node_index < nodes.size());

        auto& node = nodes[node_index];
        for (int i = 0; i < node.m_branch_stride; i++)
        {
            func(nodes[node_index + i].m_payload, node_index + i);
        }
    }

    template<class F>
        requires std::invocable<F, PayloadType&, size_t>
    void traverse_depthfirst(const PayloadType& start_payload, const F& func)
    {
        traverse_depthfirst(find_node_index(start_payload), func);
    }

    template<class F>
        requires std::invocable<F, PayloadType&, size_t>
    void traverse_depthfirst(const F& func)
    {
        size_t i = 0;
        while (i < size())
        {
            traverse_depthfirst(i, func);
            i += nodes[i].m_branch_stride;
        }
    }

    // --- Depth-first with level information ---------------------------------

private:
    template<class T, class F> requires std::invocable<F, PayloadType&, size_t, size_t>
    static void traverse_depthfirst_impl(
        T& self,
        size_t start_index,
        const F& func)
    {
        auto& nodes = self.nodes;
        if (!nodes.size()) return;
        // auto node_index = find_node_index(payload);
        assert(start_index != VecTree_NullIndex);
        assert(start_index >= 0);
        assert(start_index < nodes.size());

        std::vector<std::pair<size_t, size_t>> stack;
        stack.reserve(nodes[start_index].m_branch_stride);
        stack.push_back({ start_index, 0 });

        while (!stack.empty())
        {
            auto [index, level] = stack.back();
            stack.pop_back();

            auto& node = nodes[index];
            func(node.m_payload, index, level);

            std::vector<std::pair<size_t, size_t>> child_stack;
            stack.reserve(node.m_nbr_children);
            size_t child_index = index + 1;
            for (int i = 0; i < node.m_nbr_children; i++)
            {
                child_stack.push_back({ child_index, level + 1 });
                child_index += nodes[child_index].m_branch_stride;
            }
            stack.insert(stack.end(), child_stack.rbegin(), child_stack.rend());
        }
    }

    template<class T, class F> requires std::invocable<F, PayloadType&, size_t, size_t>
    static void traverse_depthfirst_impl(
        T& self,
        const PayloadType& start_payload,
        const F& func)
    {
        traverse_depthfirst_impl(self, self.find_node_index(start_payload), func);
    }

    template<class T, class F> requires std::invocable<F, PayloadType&, size_t, size_t>
    static void traverse_depthfirst_impl(
        T& self,
        const F& func)
    {
        size_t i = 0;
        while (i < self.size())
        {
            traverse_depthfirst_impl(self, i, func);
            i += self.nodes[i].m_branch_stride;
        }
    }

public:
    /// @brief Traverse tree depth-first with level information
    /// @param node_name Name of node to descend from
    /// The tree is not optimized for this type of traversal.
    /// F is a function of type void(PayloadType&, size_t, size_t),
    /// where the second argument is node index, and the third argument is node level.
    template<class F> requires std::invocable<F, PayloadType&, size_t, size_t>
    void traverse_depthfirst(
        size_t start_index,
        const F& func) const
    {
        traverse_depthfirst_impl(*this, start_index, func);
    }

    template<class F> requires std::invocable<F, PayloadType&, size_t, size_t>
    void traverse_depthfirst(
        size_t start_index,
        const F& func)
    {
        traverse_depthfirst_impl(*this, start_index, func);
    }

    template<class F> requires std::invocable<F, PayloadType&, size_t, size_t>
    void traverse_depthfirst(
        const PayloadType& start_payload,
        const F& func) const
    {
        traverse_depthfirst_impl(*this, start_payload, func);
    }

    template<class F> requires std::invocable<F, PayloadType&, size_t, size_t>
    void traverse_depthfirst(
        const PayloadType& start_payload,
        const F& func)
    {
        traverse_depthfirst_impl(*this, start_payload, func);
    }

    template<class F> requires std::invocable<F, PayloadType&, size_t, size_t>
    void traverse_depthfirst(
        const F& func) const
    {
        traverse_depthfirst_impl(*this, func);
    }

    template<class F> requires std::invocable<F, PayloadType&, size_t, size_t>
    void traverse_depthfirst(
        const F& func)
    {
        traverse_depthfirst_impl(*this, func);
    }

    // --- Breadth-first ------------------------------------------------------

private:
    // TODO: impl

public:
    /// @brief Traverse tree breadth-first (level-order).
    /// @param node_name Name of node to descend from
    /// The tree is not optimized for this type of traversal.
    /// F is a function of type void(PayloadType&, size_t), where the second argument is node index
    template<class F>
        requires std::invocable<F, PayloadType&, size_t>
    void traverse_breadthfirst(
        size_t start_index,
        const F& func)
    {
        if (!nodes.size()) return;
        assert(start_index != VecTree_NullIndex);
        assert(start_index >= 0);
        assert(start_index < nodes.size());

        std::queue<size_t> queue;
        queue.push(start_index);

        while (!queue.empty())
        {
            auto index = queue.front();
            queue.pop();

            auto& node = nodes[index];
            func(node.m_payload, index);

            size_t child_index = index + 1;
            for (int i = 0; i < node.m_nbr_children; i++)
            {
                queue.push(child_index);
                child_index += nodes[child_index].m_branch_stride;
            }
        }
    }

    template<class F>
        requires std::invocable<F, PayloadType&, size_t>
    void traverse_breadthfirst(
        const PayloadType& start_payload,
        const F& func)
    {
        traverse_breadthfirst(find_node_index(start_payload), func);
    }

    /// @brief Ascend to root.
    /// @param node_name Name of node to ascend from
    /// F is a function of type void(NodeType&, size_t), where the second argument is node index
    template<class F>
        requires std::invocable<F, PayloadType&, size_t>
    void ascend(
        size_t start_index,
        const F& func)
    {
        if (!nodes.size()) return;
        // auto node_index = find_node_index(payload);
        assert(start_index != VecTree_NullIndex);
        assert(start_index >= 0);
        assert(start_index < nodes.size());
        auto node_index = start_index; // find_node_index(node_name);

        while (nodes[node_index].m_parent_ofs)
        {
            func(nodes[node_index].m_payload, node_index);
            node_index -= nodes[node_index].m_parent_ofs;
        }
        func(nodes[node_index].m_payload, node_index);
    }

    template<class F>
        requires std::invocable<F, PayloadType&, size_t>
    void ascend(
        const PayloadType& start_payload,
        const F& func)
    {
        ascend(find_node_index(start_payload), func);
    }

    void reduce()
    {
        // if node[i+1] has 1 child, and node[i+1] is not tied to BONE or MESH
        //      MERGE node[i+1] (child) into into node[i]
        //      REMOVE node[i]
        //
        // MERGE = merge node transformations and keyframes(*) of all animations
        //
        // (*) if nbr keys is "1 or equal" between node[i] and node[i+1]

        for (int i = 0; i < nodes.size();)
        {
            // Abort if node has BONE or MESH
            if (nodes[i].bone_index > -1 || nodes[i].nbr_meshes > 0)
            {
                i++;
                continue;
            }
            // else if has 1 children and child is not

            if (nodes[i].m_nbr_children != 1)
            {
                i++;
                continue;
            }

            // Abort if child has BONE or MESH (first child is at i+1)
            //                if (nodes[i+1].bone_index > -1 || nodes[i+1].nbr_meshes > 0)
            //                {
            //                    i++;
            //                    continue;
            //                }

            // Now delete this node:

            auto node = nodes.begin() + i;

            // IF HAS PARENT
            // Update strides of the same branch.
            // Iterate backwards from parent to the root and increment strides
            // ranging to the insertion
            //
            auto prit = node; //-1;
            while (prit >= nodes.begin())
            {
                if (prit->m_branch_stride > /*>*/ (unsigned)std::distance(prit, node /*+1*/)) // !!!
                    prit->m_branch_stride--;                                                  //++;
                if (!prit->m_parent_ofs)                                                      // discontinue past root (preceeding trees not affected)
                    break;
                prit--;
            }

            // Update parent indices
            // Iterate forward up until the next root and increment parent ofs'
            // ranging to the insertion
            //
            auto pfit = node + 1;
            while (pfit < nodes.end())
            {
                if (!pfit->m_parent_ofs) // discontinue at succeeding root
                    break;
                if (pfit->m_parent_ofs >= /*=*/(unsigned)std::distance(node, pfit))
                    pfit->m_parent_ofs--; //++;
                pfit++;
            }

            //            nodes[i + 1].local_tfm = nodes[i].local_tfm * nodes[i + 1].local_tfm;
                        // TODO
                        // Mirror this merger in the keyframes for all animations:
                        //  animation_t::std::vector<node_animation_t>

                        // nodes[i].m_branch_stride = nodes[i+1].m_branch_stride;
                        // nodes[i].m_nbr_children = nodes[i+1].m_nbr_children;
            nodes[i + 1].m_parent_ofs = nodes[i].m_parent_ofs;
            std::cout << "REMOVED " << nodes[i].name << std::endl;
            nodes.erase(nodes.begin() + i);
        }
    }

    /// Dump all roots to a stream using recursive traversal (to be removed)
    template<class T>
    void print_to_stream(T& outstream)
    {
        int i = 0;
        while (i < nodes.size())
        {
            print_to_stream(i, "", outstream);
            i += nodes[i].m_branch_stride;
        }
    }

private:

    template<class T>
    void print_to_stream(unsigned i, const std::string& indent, T& outstream)
    {
        auto& node = nodes[i];
        outstream << indent;
        outstream << " [node " << i << "]";
        if (node.bone_index > -1)
            outstream << "[bone " << node.bone_index << "]";
        if (node.nbr_meshes)
            outstream << "[" << node.nbr_meshes << " meshes]";
        outstream << " " << node.name
            << " (children " << node.m_nbr_children
            << ", stride " << node.m_branch_stride
            << ", parent ofs " << node.m_parent_ofs << ")";
        outstream << std::endl;
        int ci = i + 1;
        for (int j = 0; j < node.m_nbr_children; j++)
        {
            print_to_stream(ci, indent + "\t", outstream);
            ci += nodes[ci].m_branch_stride;
        }
    }
};

#endif /* seqtree_h */
