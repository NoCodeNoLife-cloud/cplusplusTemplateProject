/**
 * @file BinarySearchTree.hpp
 * @brief BinarySearchTree class declaration
 * @details This header defines the BinarySearchTree class that provides functionality for Advanced data structures including trees and skip lists.
 */

#pragma once
#include <iostream>

#include "BSTBase.hpp"

namespace common::data_structure::tree
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