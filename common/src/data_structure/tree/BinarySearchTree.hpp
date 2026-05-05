#pragma once
#include <glog/logging.h>
#include <fmt/format.h>
#include <iostream>
#include <memory>
#include <optional>

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

        /// @brief Finds a value in the binary search tree and returns it.
        /// @param value The value to be searched.
        /// @return std::optional<T> The value if found, std::nullopt otherwise.
        [[nodiscard]] auto findValue(T value) const -> std::optional<T>;

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
        const bool found = findRecursive(root_, value);
        return found;
    }

    template<typename T>
    auto BinarySearchTree<T>::findValue(T value) const -> std::optional<T> {
        auto node = root_;
        while (node) {
            if (value == node->data) {
                return node->data;
            }
            if (value < node->data) {
                node = node->left_;
            } else {
                node = node->right_;
            }
        }
        return std::nullopt;
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
        if (!node) {
            return std::make_shared<TreeNode<T> >(value);
        }
        if (value < node->data) node->left_ = insertRecursive(node->left_, value);
        else if (value > node->data) node->right_ = insertRecursive(node->right_, value);
        return node;
    }

    template<typename T>
    auto BinarySearchTree<T>::findRecursive(const std::shared_ptr<TreeNode<T> > &node, T value) const -> bool {
        if (!node) return false;
        if (value == node->data) return true;
        if (value < node->data) return findRecursive(node->left_, value);
        return findRecursive(node->right_, value);
    }

    template<typename T>
    auto BinarySearchTree<T>::removeRecursive(std::shared_ptr<TreeNode<T> > node, T value) -> std::shared_ptr<TreeNode<T> > {
        if (!node) {
            return nullptr;
        }
        if (value < node->data) node->left_ = removeRecursive(node->left_, value);
        else if (value > node->data) node->right_ = removeRecursive(node->right_, value);
        else {
            if (!node->left_) return node->right_;
            if (!node->right_) return node->left_;
            node->data = minValueNode(node->right_)->data;
            node->right_ = removeRecursive(node->right_, node->data);
        }
        return node;
    }

    template<typename T>
    auto BinarySearchTree<T>::minValueNode(std::shared_ptr<TreeNode<T> > node) -> std::shared_ptr<TreeNode<T> > {
        auto current = node;
        while (current && current->left_) current = current->left_;
        return current;
    }

    template<typename T>
    auto BinarySearchTree<T>::inorderTraversalRecursive(const std::shared_ptr<TreeNode<T> > &node) const -> void {
        if (!node) return;
        inorderTraversalRecursive(node->left_);
        std::cout << node->data << " ";
        inorderTraversalRecursive(node->right_);
    }
}
