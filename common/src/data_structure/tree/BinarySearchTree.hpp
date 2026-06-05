/**
 * @file BinarySearchTree.hpp
 * @brief BinarySearchTree class declaration
 * @details This header defines the BinarySearchTree class that provides functionality for Advanced data structures including trees and skip lists.
 */

#pragma once
#include <iostream>
#include <memory>
#include <optional>
#include <utility>

#include "TreeNode.hpp"

namespace common::data_structure::tree
{
    /// @brief A binary search tree implementation.
    /// @tparam T The type of data stored in the tree.
    template <typename T>
    class BinarySearchTree
    {
    public:
        BinarySearchTree() ;

        /// @brief Inserts a value into the binary search tree.
        /// @param value The value to be inserted.
        void insert(const T& value);

        /// @brief Finds a value in the binary search tree.
        /// @param value The value to be searched.
        /// @return True if the value is found, false otherwise.
        [[nodiscard]] bool find(const T& value) const;

        /// @brief Finds a value in the binary search tree and returns it.
        /// @param value The value to be searched.
        /// @return std::optional<T> The value if found, std::nullopt otherwise.
        [[nodiscard]] std::optional<T> findValue(const T& value) const;

        /// @brief Removes a value from the binary search tree.
        /// @param value The value to be removed.
        void remove(const T& value);

        /// @brief Performs an inorder traversal of the binary search tree.
        void inorderTraversal() const;

    private:
        std::unique_ptr<TreeNode<T>> root_{};

        /// @brief Recursively inserts a value into the binary search tree.
        /// @param node The current node in the recursion.
        /// @param value The value to be inserted.
        /// @return The updated node after insertion.
        std::unique_ptr<TreeNode<T>> insertRecursive(std::unique_ptr<TreeNode<T>> node, const T& value);

        /// @brief Recursively finds a value in the binary search tree.
        /// @param node The current node in the recursion.
        /// @param value The value to be searched.
        /// @return True if the value is found, false otherwise.
        [[nodiscard]] bool findRecursive(const TreeNode<T>* node, const T& value) const;

        /// @brief Recursively removes a value from the binary search tree.
        /// @param node The current node in the recursion.
        /// @param value The value to be removed.
        /// @return The updated node after removal.
        std::unique_ptr<TreeNode<T>> removeRecursive(std::unique_ptr<TreeNode<T>> node, const T& value);

        /// @brief Finds the node with the minimum value in a subtree.
        /// @param node The root of the subtree.
        /// @return The node with the minimum value.
        [[nodiscard]] TreeNode<T>* minValueNode(TreeNode<T>* node);

        /// @brief Recursively performs an inorder traversal of the binary search tree.
        /// @param node The current node in the recursion.
        void inorderTraversalRecursive(const TreeNode<T>* node) const;
    };

    template <typename T>
    BinarySearchTree<T>::BinarySearchTree()  = default;

    template <typename T>
    void BinarySearchTree<T>::insert(const T& value)
    {
        root_ = insertRecursive(std::move(root_), value);
    }

    template <typename T>
    bool BinarySearchTree<T>::find(const T& value) const
    {
        return findRecursive(root_.get(), value);
    }

    template <typename T>
    std::optional<T> BinarySearchTree<T>::findValue(const T& value) const
    {
        auto* node = root_.get();
        while (node)
        {
            if (value == node->data)
            {
                return node->data;
            }
            if (value < node->data)
            {
                node = node->left_.get();
            }
            else
            {
                node = node->right_.get();
            }
        }
        return std::nullopt;
    }

    template <typename T>
    void BinarySearchTree<T>::remove(const T& value)
    {
        root_ = removeRecursive(std::move(root_), value);
    }

    template <typename T>
    void BinarySearchTree<T>::inorderTraversal() const
    {
        inorderTraversalRecursive(root_.get());
        std::cout << std::endl;
    }

    template <typename T>
    std::unique_ptr<TreeNode<T>> BinarySearchTree<T>::insertRecursive(std::unique_ptr<TreeNode<T>> node, const T& value)
    {
        if (!node)
        {
            return std::make_unique<TreeNode<T>>(value);
        }
        if (value < node->data) node->left_ = insertRecursive(std::move(node->left_), value);
        else if (value > node->data) node->right_ = insertRecursive(std::move(node->right_), value);
        return node;
    }

    template <typename T>
    bool BinarySearchTree<T>::findRecursive(const TreeNode<T>* node, const T& value) const
    {
        if (!node) return false;
        if (value == node->data) return true;
        if (value < node->data) return findRecursive(node->left_.get(), value);
        return findRecursive(node->right_.get(), value);
    }

    template <typename T>
    std::unique_ptr<TreeNode<T>> BinarySearchTree<T>::removeRecursive(std::unique_ptr<TreeNode<T>> node, const T& value)
    {
        if (!node)
        {
            return nullptr;
        }
        if (value < node->data) node->left_ = removeRecursive(std::move(node->left_), value);
        else if (value > node->data) node->right_ = removeRecursive(std::move(node->right_), value);
        else
        {
            if (!node->left_) return std::move(node->right_);
            if (!node->right_) return std::move(node->left_);
            auto* successor = minValueNode(node->right_.get());
            node->data = successor->data;
            node->right_ = removeRecursive(std::move(node->right_), node->data);
        }
        return node;
    }

    template <typename T>
    TreeNode<T>* BinarySearchTree<T>::minValueNode(TreeNode<T>* node)
    {
        auto* current = node;
        while (current && current->left_) current = current->left_.get();
        return current;
    }

    template <typename T>
    void BinarySearchTree<T>::inorderTraversalRecursive(const TreeNode<T>* node) const
    {
        if (!node) return;
        inorderTraversalRecursive(node->left_.get());
        std::cout << node->data << " ";
        inorderTraversalRecursive(node->right_.get());
    }
}