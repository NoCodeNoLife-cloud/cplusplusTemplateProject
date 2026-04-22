#pragma once
#include <glog/logging.h>
#include <fmt/format.h>
#include <algorithm>
#include <memory>
#include <cstdint>

#include "TreeNode.hpp"

namespace common::data_structure::tree {
    /// @brief A self-balancing binary search tree implementation (AVL Tree)
    /// An AVL tree is a self-balancing binary search tree where the heights of the two child
    /// subtrees of any node differ by at most one. This implementation uses shared pointers
    /// for memory management and automatic cleanup.
    /// The tree maintains balance through rotations during insertions and deletions,
    /// ensuring O(log n) time complexity for search, insert, and delete operations.
    /// @tparam T The type of data stored in the tree. Must be comparable with <, >, and == operators.
    template<typename T>
    class AVLTree {
    public:
        AVLTree();

        /// @brief Insert a value into the AVL tree
        /// @param value The value to insert
        auto insert(T value) -> void;

        /// @brief Remove a value from the AVL tree
        /// @param value The value to remove
        auto remove(T value) -> void;

        /// @brief Find a value in the AVL tree
        /// @param value The value to find
        /// @return bool True if the value is found, false otherwise
        [[nodiscard]] auto find(T value) const -> bool;

    private:
        std::shared_ptr<TreeNode<T> > root_{};

        /// @brief Get the height of a node
        /// @param node The node to get the height of
        /// @return int32_t The height of the node
        [[nodiscard]] auto getHeight(const std::shared_ptr<TreeNode<T> > &node) const noexcept -> int32_t;

        /// @brief Get the balance factor of a node
        /// @param node The node to get the balance factor of
        /// @return int32_t The balance factor of the node
        [[nodiscard]] auto getBalance(const std::shared_ptr<TreeNode<T> > &node) const noexcept -> int32_t;

        /// @brief Rotate a subtree to the right
        /// @param y The root of the subtree to rotate
        /// @return std::shared_ptr<TreeNode<T>> The new root of the subtree
        auto rotateRight(std::shared_ptr<TreeNode<T> > y) -> std::shared_ptr<TreeNode<T> >;

        /// @brief Rotate a subtree to the left
        /// @param x The root of the subtree to rotate
        /// @return std::shared_ptr<TreeNode<T>> The new root of the subtree
        auto rotateLeft(std::shared_ptr<TreeNode<T> > x) -> std::shared_ptr<TreeNode<T> >;

        /// @brief Insert a value into a subtree
        /// @param node The root of the subtree to insert into
        /// @param value The value to insert
        /// @return std::shared_ptr<TreeNode<T>> The new root of the subtree
        auto insert(std::shared_ptr<TreeNode<T> > node, T value) -> std::shared_ptr<TreeNode<T> >;

        /// @brief Remove a value from a subtree
        /// @param node The root of the subtree to remove from
        /// @param value The value to remove
        /// @return std::shared_ptr<TreeNode<T>> The new root of the subtree
        auto remove(std::shared_ptr<TreeNode<T> > node, T value) -> std::shared_ptr<TreeNode<T> >;

        /// @brief Find the node with the minimum value in a subtree
        /// @param node The root of the subtree to search
        /// @return std::shared_ptr<TreeNode<T>> The node with the minimum value
        [[nodiscard]] auto findMin(const std::shared_ptr<TreeNode<T> > &node) const -> std::shared_ptr<TreeNode<T> >;

        /// @brief Find a value in a subtree
        /// @param node The root of the subtree to search
        /// @param value The value to find
        /// @return std::shared_ptr<TreeNode<T>> The node containing the value, or nullptr if not found
        [[nodiscard]] auto find(const std::shared_ptr<TreeNode<T> > &node, T value) const -> std::shared_ptr<TreeNode<T> >;
    };

    template<typename T>
    AVLTree<T>::AVLTree() = default;

    template<typename T>
    auto AVLTree<T>::insert(T value) -> void {
        DLOG(INFO) << fmt::format("AVLTree insert - value: {}", value);
        root_ = insert(root_, value);
    }

    template<typename T>
    auto AVLTree<T>::remove(T value) -> void {
        DLOG(INFO) << fmt::format("AVLTree remove - value: {}", value);
        root_ = remove(root_, value);
    }

    template<typename T>
    auto AVLTree<T>::find(T value) const -> bool {
        const bool found = find(root_, value) != nullptr;
        DLOG(INFO) << fmt::format("AVLTree find - value: {}, result: {}", value, found ? "found" : "not found");
        return found;
    }

    template<typename T>
    auto AVLTree<T>::getHeight(const std::shared_ptr<TreeNode<T> > &node) const noexcept -> int32_t {
        return node ? node->height : 0;
    }

    template<typename T>
    auto AVLTree<T>::getBalance(const std::shared_ptr<TreeNode<T> > &node) const noexcept -> int32_t {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }

    template<typename T>
    auto AVLTree<T>::rotateRight(std::shared_ptr<TreeNode<T> > y) -> std::shared_ptr<TreeNode<T> > {
        DLOG(INFO) << fmt::format("AVLTree rotateRight - node value: {}", y->data);
        auto x = y->left;
        auto T2 = x->right;
        x->right = y;
        y->left = T2;
        y->height = 1 + std::max(getHeight(y->left), getHeight(y->right));
        x->height = 1 + std::max(getHeight(x->left), getHeight(x->right));
        return x;
    }

    template<typename T>
    auto AVLTree<T>::rotateLeft(std::shared_ptr<TreeNode<T> > x) -> std::shared_ptr<TreeNode<T> > {
        DLOG(INFO) << fmt::format("AVLTree rotateLeft - node value: {}", x->data);
        auto y = x->right;
        auto T2 = y->left;
        y->left = x;
        x->right = T2;
        x->height = 1 + std::max(getHeight(x->left), getHeight(x->right));
        y->height = 1 + std::max(getHeight(y->left), getHeight(y->right));
        return y;
    }

    template<typename T>
    auto AVLTree<T>::insert(std::shared_ptr<TreeNode<T> > node, T value) -> std::shared_ptr<TreeNode<T> > {
        if (!node) {
            DLOG(INFO) << fmt::format("AVLTree insert - creating new node with value: {}", value);
            return std::make_shared<TreeNode<T> >(value);
        }
        if (value < node->data) node->left = insert(node->left, value);
        else if (value > node->data) node->right = insert(node->right, value);
        else {
            DLOG(INFO) << fmt::format("AVLTree insert - duplicate value ignored: {}", value);
            return node;
        }
        node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
        const int32_t balance = getBalance(node);
        DLOG(INFO) << fmt::format("AVLTree insert - node: {}, balance factor: {}", node->data, balance);
        if (balance > 1 && value < node->left->data) {
            DLOG(INFO) << "AVLTree insert - performing right rotation (LL case)";
            return rotateRight(node);
        }
        if (balance < -1 && value > node->right->data) {
            DLOG(INFO) << "AVLTree insert - performing left rotation (RR case)";
            return rotateLeft(node);
        }
        if (balance > 1 && value > node->left->data) {
            DLOG(INFO) << "AVLTree insert - performing left-right rotation (LR case)";
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        if (balance < -1 && value < node->right->data) {
            DLOG(INFO) << "AVLTree insert - performing right-left rotation (RL case)";
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
        return node;
    }

    template<typename T>
    auto AVLTree<T>::remove(std::shared_ptr<TreeNode<T> > node, T value) -> std::shared_ptr<TreeNode<T> > {
        if (!node) {
            DLOG(WARNING) << fmt::format("AVLTree remove - value not found: {}", value);
            return node;
        }
        if (value < node->data) node->left = remove(node->left, value);
        else if (value > node->data) node->right = remove(node->right, value);
        else {
            DLOG(INFO) << fmt::format("AVLTree remove - removing node with value: {}", value);
            if (!node->left && !node->right) node = nullptr;
            else if (!node->left) node = node->right;
            else if (!node->right) node = node->left;
            else {
                auto successor = findMin(node->right);
                DLOG(INFO) << fmt::format("AVLTree remove - replacing with successor: {}", successor->data);
                node->data = successor->data;
                node->right = remove(node->right, successor->data);
            }
        }
        if (!node) return node;
        node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
        const int32_t balance = getBalance(node);
        DLOG(INFO) << fmt::format("AVLTree remove - node: {}, balance factor: {}", node->data, balance);
        if (balance > 1) {
            if (getBalance(node->left) >= 0) {
                DLOG(INFO) << "AVLTree remove - performing right rotation (LL case)";
                return rotateRight(node);
            }
            DLOG(INFO) << "AVLTree remove - performing left-right rotation (LR case)";
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        if (balance < -1) {
            if (getBalance(node->right) <= 0) {
                DLOG(INFO) << "AVLTree remove - performing left rotation (RR case)";
                return rotateLeft(node);
            }
            DLOG(INFO) << "AVLTree remove - performing right-left rotation (RL case)";
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
        return node;
    }

    template<typename T>
    auto AVLTree<T>::findMin(const std::shared_ptr<TreeNode<T> > &node) const -> std::shared_ptr<TreeNode<T> > {
        auto current = node;
        while (current->left) current = current->left;
        return current;
    }

    template<typename T>
    auto AVLTree<T>::find(const std::shared_ptr<TreeNode<T> > &node, T value) const -> std::shared_ptr<TreeNode<T> > {
        if (!node) return nullptr;
        if (value == node->data) return node;
        return value < node->data ? find(node->left, value) : find(node->right, value);
    }
}
