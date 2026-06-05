/**
 * @file AVLTree.hpp
 * @brief AVLTree class declaration
 * @details This header defines the AVLTree class that provides functionality for Advanced data structures including trees and skip lists.
 */

#pragma once
#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

#include "TreeNode.hpp"

namespace common::data_structure::tree
{
    /// @brief A self-balancing binary search tree implementation (AVL Tree)
    /// An AVL tree is a self-balancing binary search tree where the heights of the two child
    /// subtrees of any node differ by at most one. This implementation uses unique pointers
    /// for memory management and automatic cleanup.
    /// The tree maintains balance through rotations during insertions and deletions,
    /// ensuring O(log n) time complexity for search, insert, and delete operations.
    /// @tparam T The type of data stored in the tree. Must be comparable with <, >, and == operators.
    template <typename T>
    class AVLTree
    {
    public:
        AVLTree();

        /// @brief Insert a value into the AVL tree
        /// @param value The value to insert
        void insert(const T& value);

        /// @brief Remove a value from the AVL tree
        /// @param value The value to remove
        void remove(const T& value);

        /// @brief Find a value in the AVL tree
        /// @param value The value to find
        /// @return bool True if the value is found, false otherwise
        [[nodiscard]] bool find(const T& value) const;

        /// @brief Find a value in the AVL tree and return it
        /// @param value The value to find
        /// @return std::optional<T> The value if found, std::nullopt otherwise
        [[nodiscard]] std::optional<T> findValue(const T& value) const;

    private:
        std::unique_ptr<TreeNode<T>> root_{};

        /// @brief Get the height of a node
        /// @param node The node to get the height of
        /// @return int32_t The height of the node
        [[nodiscard]] int32_t getHeight(const TreeNode<T>* node) const;

        /// @brief Get the balance factor of a node
        /// @param node The node to get the balance factor of
        /// @return int32_t The balance factor of the node
        [[nodiscard]] int32_t getBalance(const TreeNode<T>* node) const;

        /// @brief Rotate a subtree to the right
        /// @param y The root of the subtree to rotate
        /// @return std::unique_ptr<TreeNode<T>> The new root of the subtree
        std::unique_ptr<TreeNode<T>> rotateRight(std::unique_ptr<TreeNode<T>> y);

        /// @brief Rotate a subtree to the left
        /// @param x The root of the subtree to rotate
        /// @return std::unique_ptr<TreeNode<T>> The new root of the subtree
        std::unique_ptr<TreeNode<T>> rotateLeft(std::unique_ptr<TreeNode<T>> x);

        /// @brief Insert a value into a subtree
        /// @param node The root of the subtree to insert into
        /// @param value The value to insert
        /// @return std::unique_ptr<TreeNode<T>> The new root of the subtree
        std::unique_ptr<TreeNode<T>> insert(std::unique_ptr<TreeNode<T>> node, const T& value);

        /// @brief Remove a value from a subtree
        /// @param node The root of the subtree to remove from
        /// @param value The value to remove
        /// @return std::unique_ptr<TreeNode<T>> The new root of the subtree
        std::unique_ptr<TreeNode<T>> remove(std::unique_ptr<TreeNode<T>> node, const T& value);

        /// @brief Find the node with the minimum value in a subtree
        /// @param node The root of the subtree to search
        /// @return TreeNode<T>* The node with the minimum value
        [[nodiscard]] TreeNode<T>* findMin(TreeNode<T>* node) const;

        /// @brief Find a value in a subtree
        /// @param node The root of the subtree to search
        /// @param value The value to find
        /// @return TreeNode<T>* The node containing the value, or nullptr if not found
        [[nodiscard]] TreeNode<T>* find(TreeNode<T>* node, const T& value) const;
    };

    template <typename T>
    AVLTree<T>::AVLTree() = default;

    template <typename T>
    void AVLTree<T>::insert(const T& value)
    {
        root_ = insert(std::move(root_), value);
    }

    template <typename T>
    void AVLTree<T>::remove(const T& value)
    {
        root_ = remove(std::move(root_), value);
    }

    template <typename T>
    bool AVLTree<T>::find(const T& value) const
    {
        return find(root_.get(), value) != nullptr;
    }

    template <typename T>
    std::optional<T> AVLTree<T>::findValue(const T& value) const
    {
        const auto* node = find(root_.get(), value);
        if (node)
        {
            return node->data;
        }
        return std::nullopt;
    }

    template <typename T>
    int32_t AVLTree<T>::getHeight(const TreeNode<T>* node) const
    {
        return node ? node->height_ : 0;
    }

    template <typename T>
    int32_t AVLTree<T>::getBalance(const TreeNode<T>* node) const
    {
        return node ? getHeight(node->left_.get()) - getHeight(node->right_.get()) : 0;
    }

    template <typename T>
    std::unique_ptr<TreeNode<T>> AVLTree<T>::rotateRight(std::unique_ptr<TreeNode<T>> y)
    {
        auto x = std::move(y->left_);
        auto T2 = std::move(x->right_);
        x->right_ = std::move(y);
        x->right_->left_ = std::move(T2);
        x->right_->height_ = 1 + std::max(getHeight(x->right_->left_.get()), getHeight(x->right_->right_.get()));
        x->height_ = 1 + std::max(getHeight(x->left_.get()), getHeight(x->right_.get()));
        return x;
    }

    template <typename T>
    std::unique_ptr<TreeNode<T>> AVLTree<T>::rotateLeft(std::unique_ptr<TreeNode<T>> x)
    {
        auto y = std::move(x->right_);
        auto T2 = std::move(y->left_);
        y->left_ = std::move(x);
        y->left_->right_ = std::move(T2);
        y->left_->height_ = 1 + std::max(getHeight(y->left_->left_.get()), getHeight(y->left_->right_.get()));
        y->height_ = 1 + std::max(getHeight(y->left_.get()), getHeight(y->right_.get()));
        return y;
    }

    template <typename T>
    std::unique_ptr<TreeNode<T>> AVLTree<T>::insert(std::unique_ptr<TreeNode<T>> node, const T& value)
    {
        if (!node)
        {
            return std::make_unique<TreeNode<T>>(value);
        }
        if (value < node->data) node->left_ = insert(std::move(node->left_), value);
        else if (value > node->data) node->right_ = insert(std::move(node->right_), value);
        else
        {
            return node;
        }
        node->height_ = 1 + std::max(getHeight(node->left_.get()), getHeight(node->right_.get()));
        const int32_t balance = getBalance(node.get());
        if (balance > 1 && value < node->left_->data)
        {
            return rotateRight(std::move(node));
        }
        if (balance < -1 && value > node->right_->data)
        {
            return rotateLeft(std::move(node));
        }
        if (balance > 1 && value > node->left_->data)
        {
            node->left_ = rotateLeft(std::move(node->left_));
            return rotateRight(std::move(node));
        }
        if (balance < -1 && value < node->right_->data)
        {
            node->right_ = rotateRight(std::move(node->right_));
            return rotateLeft(std::move(node));
        }
        return node;
    }

    template <typename T>
    std::unique_ptr<TreeNode<T>> AVLTree<T>::remove(std::unique_ptr<TreeNode<T>> node, const T& value)
    {
        if (!node)
        {
            return node;
        }
        if (value < node->data) node->left_ = remove(std::move(node->left_), value);
        else if (value > node->data) node->right_ = remove(std::move(node->right_), value);
        else
        {
            if (!node->left_ && !node->right_) node = nullptr;
            else if (!node->left_) node = std::move(node->right_);
            else if (!node->right_) node = std::move(node->left_);
            else
            {
                auto* successor = findMin(node->right_.get());
                node->data = successor->data;
                node->right_ = remove(std::move(node->right_), node->data);
            }
        }
        if (!node) return node;
        node->height_ = 1 + std::max(getHeight(node->left_.get()), getHeight(node->right_.get()));
        const int32_t balance = getBalance(node.get());
        if (balance > 1)
        {
            if (getBalance(node->left_.get()) >= 0)
            {
                return rotateRight(std::move(node));
            }
            node->left_ = rotateLeft(std::move(node->left_));
            return rotateRight(std::move(node));
        }
        if (balance < -1)
        {
            if (getBalance(node->right_.get()) <= 0)
            {
                return rotateLeft(std::move(node));
            }
            node->right_ = rotateRight(std::move(node->right_));
            return rotateLeft(std::move(node));
        }
        return node;
    }

    template <typename T>
    TreeNode<T>* AVLTree<T>::findMin(TreeNode<T>* node) const
    {
        auto* current = node;
        while (current && current->left_) current = current->left_.get();
        return current;
    }

    template <typename T>
    TreeNode<T>* AVLTree<T>::find(TreeNode<T>* node, const T& value) const
    {
        if (!node) return nullptr;
        if (value == node->data) return node;
        return value < node->data ? find(node->left_.get(), value) : find(node->right_.get(), value);
    }
}