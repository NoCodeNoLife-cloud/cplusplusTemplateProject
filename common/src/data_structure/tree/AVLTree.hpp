/**
 * @file AVLTree.hpp
 * @brief AVLTree class declaration
 * @details This header defines the AVLTree class that provides functionality for Advanced data structures including trees and skip lists.
 */

#pragma once
#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>

#include "BSTBase.hpp"

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
    class AVLTree : public BSTBase<T, AVLTree<T>>
    {
        friend class BSTBase<T, AVLTree<T>>;

    public:
        AVLTree() = default;

    private:
        // ---- CRTP hooks (override BSTBase defaults) ----

        /// @brief Rebalance after insertion at the current level
        std::unique_ptr<TreeNode<T>> postInsert(std::unique_ptr<TreeNode<T>> node, const T& value)
        {
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

        /// @brief Rebalance after removal at the current level
        std::unique_ptr<TreeNode<T>> postRemove(std::unique_ptr<TreeNode<T>> node)
        {
            if (!node) return nullptr;
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

        // ---- AVL-specific helpers ----

        [[nodiscard]] int32_t getHeight(const TreeNode<T>* node) const
        {
            return node ? node->height_ : 0;
        }

        [[nodiscard]] int32_t getBalance(const TreeNode<T>* node) const
        {
            return node ? getHeight(node->left_.get()) - getHeight(node->right_.get()) : 0;
        }

        std::unique_ptr<TreeNode<T>> rotateRight(std::unique_ptr<TreeNode<T>> y)
        {
            auto x = std::move(y->left_);
            auto T2 = std::move(x->right_);
            x->right_ = std::move(y);
            x->right_->left_ = std::move(T2);
            x->right_->height_ = 1 + std::max(getHeight(x->right_->left_.get()), getHeight(x->right_->right_.get()));
            x->height_ = 1 + std::max(getHeight(x->left_.get()), getHeight(x->right_.get()));
            return x;
        }

        std::unique_ptr<TreeNode<T>> rotateLeft(std::unique_ptr<TreeNode<T>> x)
        {
            auto y = std::move(x->right_);
            auto T2 = std::move(y->left_);
            y->left_ = std::move(x);
            y->left_->right_ = std::move(T2);
            y->left_->height_ = 1 + std::max(getHeight(y->left_->left_.get()), getHeight(y->left_->right_.get()));
            y->height_ = 1 + std::max(getHeight(y->left_.get()), getHeight(y->right_.get()));
            return y;
        }
    };
}