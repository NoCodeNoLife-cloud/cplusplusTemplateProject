#pragma once
#include <iostream>
#include <memory>

#include "TreeNode.hpp"

namespace common::data_structure::tree {
    /// @brief A binary search tree implementation.
    /// @tparam T The type of data stored in the tree.
    template<typename T>
    class BinarySearchTree {
    public:
        BinarySearchTree() noexcept;

        /// @brief Inserts a value into the binary search tree.
        /// @param value The value to be inserted.
        auto insert(T value) -> void;

        /// @brief Finds a value in the binary search tree.
        /// @param value The value to be searched.
        /// @return True if the value is found, false otherwise.
        [[nodiscard]] auto find(T value) const -> bool;

        /// @brief Removes a value from the binary search tree.
        /// @param value The value to be removed.
        auto remove(T value) -> void;

        /// @brief Performs an inorder traversal of the binary search tree.
        auto inorderTraversal() const -> void;

    private:
        std::shared_ptr<TreeNode<T> > root_{};

        /// @brief Recursively inserts a value into the binary search tree.
        /// @param node The current node in the recursion.
        /// @param value The value to be inserted.
        /// @return The updated node after insertion.
        auto insertRecursive(std::shared_ptr<TreeNode<T> > node, T value) -> std::shared_ptr<TreeNode<T> >;

        /// @brief Recursively finds a value in the binary search tree.
        /// @param node The current node in the recursion.
        /// @param value The value to be searched.
        /// @return True if the value is found, false otherwise.
        [[nodiscard]] auto findRecursive(const std::shared_ptr<TreeNode<T> > &node, T value) const -> bool;

        /// @brief Recursively removes a value from the binary search tree.
        /// @param node The current node in the recursion.
        /// @param value The value to be removed.
        /// @return The updated node after removal.
        auto removeRecursive(std::shared_ptr<TreeNode<T> > node, T value) -> std::shared_ptr<TreeNode<T> >;

        /// @brief Finds the node with the minimum value in a subtree.
        /// @param node The root of the subtree.
        /// @return The node with the minimum value.
        [[nodiscard]] auto minValueNode(std::shared_ptr<TreeNode<T> > node) -> std::shared_ptr<TreeNode<T> >;

        /// @brief Recursively performs an inorder traversal of the binary search tree.
        /// @param node The current node in the recursion.
        auto inorderTraversalRecursive(const std::shared_ptr<TreeNode<T> > &node) const -> void;
    };

    template<typename T>
    BinarySearchTree<T>::BinarySearchTree() noexcept = default;

    template<typename T>
    auto BinarySearchTree<T>::insert(T value) -> void {
        root_ = insertRecursive(root_, value);
    }

    template<typename T>
    auto BinarySearchTree<T>::find(T value) const -> bool {
        return findRecursive(root_, value);
    }

    template<typename T>
    auto BinarySearchTree<T>::remove(T value) -> void {
        root_ = removeRecursive(root_, value);
    }

    template<typename T>
    auto BinarySearchTree<T>::inorderTraversal() const -> void {
        inorderTraversalRecursive(root_);
        std::cout << std::endl;
    }

    template<typename T>
    auto BinarySearchTree<T>::insertRecursive(std::shared_ptr<TreeNode<T> > node, T value) -> std::shared_ptr<TreeNode<T> > {
        if (!node) return std::make_shared<TreeNode<T> >(value);
        if (value < node->data) node->left = insertRecursive(node->left, value);
        else if (value > node->data) node->right = insertRecursive(node->right, value);
        return node;
    }

    template<typename T>
    auto BinarySearchTree<T>::findRecursive(const std::shared_ptr<TreeNode<T> > &node, T value) const -> bool {
        if (!node) return false;
        if (value == node->data) return true;
        if (value < node->data) return findRecursive(node->left, value);
        return findRecursive(node->right, value);
    }

    template<typename T>
    auto BinarySearchTree<T>::removeRecursive(std::shared_ptr<TreeNode<T> > node, T value) -> std::shared_ptr<TreeNode<T> > {
        if (!node) return nullptr;
        if (value < node->data) node->left = removeRecursive(node->left, value);
        else if (value > node->data) node->right = removeRecursive(node->right, value);
        else {
            if (!node->left) return node->right;
            if (!node->right) return node->left;
            node->data = minValueNode(node->right)->data;
            node->right = removeRecursive(node->right, node->data);
        }
        return node;
    }

    template<typename T>
    auto BinarySearchTree<T>::minValueNode(std::shared_ptr<TreeNode<T> > node) -> std::shared_ptr<TreeNode<T> > {
        auto current = node;
        while (current && current->left) current = current->left;
        return current;
    }

    template<typename T>
    auto BinarySearchTree<T>::inorderTraversalRecursive(const std::shared_ptr<TreeNode<T> > &node) const -> void {
        if (!node) return;
        inorderTraversalRecursive(node->left);
        std::cout << node->data << " ";
        inorderTraversalRecursive(node->right);
    }
}
