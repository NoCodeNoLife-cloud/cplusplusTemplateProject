/**
 * @file TreeNode.hpp
 * @brief Generic tree node with left/right children for BST / AVL / RB trees
 * @details A reusable tree node structure used across multiple tree
 *          implementations (BinarySearchTree, AVLTree, RedBlackTree, etc.).
 *          Stores data payload, optional height (for AVL) or colour (for RB),
 *          and left/right child unique_ptr pointers.  Provides static helper
 *          methods for findNode, findMin, and findMax.
 *
 * @par Ownership
 * Nodes own their left and right subtrees via std::unique_ptr, ensuring
 * automatic memory reclamation on node destruction.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 */

#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

#include "data_structure/tree/core/TreeToolkit.hpp"

namespace common::data_structure::tree::node
{
    /// @brief A tree node class that contains a value and pointers to left and right children.
    /// @tparam T The type of the value stored in the node.
    template <typename T>
    class TreeNode
    {
    public:
        /// @brief Construct a TreeNode with the given value
        /// @param value The value to store in the node
        explicit TreeNode(T value);

        T data{};
        std::unique_ptr<TreeNode> left_{};
        std::unique_ptr<TreeNode> right_{};
        int32_t height_{1};

        /// @brief Find the node with the minimum value in a subtree
        [[nodiscard]] static TreeNode<T>* findMin(TreeNode<T>* node);
        /// @brief Find the node with the minimum value in a subtree (const)
        [[nodiscard]] static const TreeNode<T>* findMin(const TreeNode<T>* node);
        /// @brief Find a node by value in a binary search tree (mutable)
        [[nodiscard]] static TreeNode<T>* findNode(TreeNode<T>* node, const T& value);
        /// @brief Find a node by value in a binary search tree (const)
        [[nodiscard]] static const TreeNode<T>* findNode(const TreeNode<T>* node, const T& value);
        /// @brief Compute the height of a subtree
        [[nodiscard]] static int32_t height(const TreeNode<T>* node);
        /// @brief Count the number of nodes in a subtree
        [[nodiscard]] static size_t size(const TreeNode<T>* node);
    };

    template <typename T>
    TreeNode<T>::TreeNode(T value) : data(std::move(value))
    {
    }

    template <typename T>
    TreeNode<T>* TreeNode<T>::findMin(TreeNode<T>* node)
    {
        auto* current = node;
        while (current && current->left_) current = current->left_.get();
        return current;
    }

    template <typename T>
    const TreeNode<T>* TreeNode<T>::findMin(const TreeNode<T>* node)
    {
        const auto* current = node;
        while (current && current->left_) current = current->left_.get();
        return current;
    }

    template <typename T>
    TreeNode<T>* TreeNode<T>::findNode(TreeNode<T>* node, const T& value)
    {
        if (!node) return nullptr;
        if (value == node->data) return node;
        return value < node->data ? findNode(node->left_.get(), value) : findNode(node->right_.get(), value);
    }

    template <typename T>
    const TreeNode<T>* TreeNode<T>::findNode(const TreeNode<T>* node, const T& value)
    {
        if (!node) return nullptr;
        if (value == node->data) return node;
        return value < node->data ? findNode(node->left_.get(), value) : findNode(node->right_.get(), value);
    }

    template <typename T>
    int32_t TreeNode<T>::height(const TreeNode<T>* node)
    {
        return core::TreeToolkit::height(node);
    }

    template <typename T>
    size_t TreeNode<T>::size(const TreeNode<T>* node)
    {
        return core::TreeToolkit::size(node);
    }
}
