/**
 * @file BinarySearchTree.hpp
 * @brief Unbalanced binary search tree (no rebalancing)
 * @details A standard BST implementation without rebalancing.  Insert/erase/find
 *          degrade to O(n) in the worst case (inserted sequence is sorted).
 *          Serves as a baseline for comparing self-balancing variants (AVL,
 *          RedBlack, Splay, Treap).  Supports inorder traversal for sorted
 *          iteration.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Complexity
 * - search / insert / remove: O(h) where h = height (worst O(n), best O(log n))
 */

#pragma once
#include <iostream>

#include "BSTBase.hpp"

namespace common::data_structure::tree::core
{
    /// @brief A binary search tree implementation.
    /// @tparam T The type of data stored in the tree.
    template <typename T>
    class BinarySearchTree : public BSTBase<T, BinarySearchTree<T>>
    {
    public:
        BinarySearchTree() = default;

        /// @brief Performs an inorder traversal of the binary search tree.
        void inorderTraversal() const
        {
            inorderTraversalRecursive(this->root_.get());
            std::cout << std::endl;
        }

    private:
        /// @brief Recursively performs an inorder traversal of the binary search tree.
        /// @param node The current node in the recursion.
        void inorderTraversalRecursive(const TreeNode<T>* node) const
        {
            if (!node) return;
            inorderTraversalRecursive(node->left_.get());
            std::cout << node->data << " ";
            inorderTraversalRecursive(node->right_.get());
        }
    };
}