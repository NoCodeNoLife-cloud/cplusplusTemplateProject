/**
 * @file BSTBase.hpp
 * @brief CRTP base class for binary search tree variants
 * @details Provides shared insert/remove/find/findValue logic for BST and AVL.
 *          Derived classes override postInsert() / postRemove() hooks to add
 *          rebalancing (AVL) or leave them as no-ops (plain BST).
 */

#pragma once
#include <memory>
#include <optional>
#include <utility>

#include "TreeNode.hpp"

namespace common::data_structure::tree
{
    /// @brief CRTP base for binary search tree implementations
    /// @tparam T The type of data stored in the tree
    /// @tparam Derived The derived tree class (CRTP)
    template <typename T, typename Derived>
    class BSTBase
    {
    public:
        BSTBase() = default;

        /// @brief Insert a value into the tree
        /// @param value The value to insert
        void insert(const T& value)
        {
            root_ = insertImpl(std::move(root_), value);
        }

        /// @brief Remove a value from the tree
        /// @param value The value to remove
        void remove(const T& value)
        {
            root_ = removeImpl(std::move(root_), value);
        }

        /// @brief Check if a value exists in the tree
        /// @param value The value to find
        /// @return True if the value is found, false otherwise
        [[nodiscard]] bool find(const T& value) const
        {
            return TreeNode<T>::findNode(root_.get(), value) != nullptr;
        }

        /// @brief Find a value and return it
        /// @param value The value to find
        /// @return std::optional<T> The value if found, std::nullopt otherwise
        [[nodiscard]] std::optional<T> findValue(const T& value) const
        {
            const auto* node = TreeNode<T>::findNode(root_.get(), value);
            if (node)
            {
                return node->data;
            }
            return std::nullopt;
        }

    protected:
        std::unique_ptr<TreeNode<T>> root_{};

        /// @brief Recursive BST insertion
        /// @param node Current subtree root
        /// @param value Value to insert
        /// @return Updated subtree root
        std::unique_ptr<TreeNode<T>> insertImpl(std::unique_ptr<TreeNode<T>> node, const T& value)
        {
            if (!node)
            {
                return std::make_unique<TreeNode<T>>(value);
            }
            if (value < node->data)
            {
                node->left_ = insertImpl(std::move(node->left_), value);
            }
            else if (value > node->data)
            {
                node->right_ = insertImpl(std::move(node->right_), value);
            }
            else
            {
                return node;
            }
            return derived().postInsert(std::move(node), value);
        }

        /// @brief Recursive BST removal
        /// @param node Current subtree root
        /// @param value Value to remove
        /// @return Updated subtree root
        std::unique_ptr<TreeNode<T>> removeImpl(std::unique_ptr<TreeNode<T>> node, const T& value)
        {
            if (!node) return nullptr;

            if (value < node->data)
            {
                node->left_ = removeImpl(std::move(node->left_), value);
                return derived().postRemove(std::move(node));
            }
            if (value > node->data)
            {
                node->right_ = removeImpl(std::move(node->right_), value);
                return derived().postRemove(std::move(node));
            }

            // Found the node to delete
            if (!node->left_ && !node->right_) return nullptr;
            if (!node->left_) return std::move(node->right_);
            if (!node->right_) return std::move(node->left_);

            auto* successor = TreeNode<T>::findMin(node->right_.get());
            node->data = successor->data;
            node->right_ = removeImpl(std::move(node->right_), node->data);
            return derived().postRemove(std::move(node));
        }

        /// @brief CRTP hook invoked after each insertion step.
        /// Override in derived (AVL) to rebalance; default is no-op.
        std::unique_ptr<TreeNode<T>> postInsert(std::unique_ptr<TreeNode<T>> node, const T&)
        {
            return node;
        }

        /// @brief CRTP hook invoked after each removal step.
        /// Override in derived (AVL) to rebalance; default is no-op.
        std::unique_ptr<TreeNode<T>> postRemove(std::unique_ptr<TreeNode<T>> node)
        {
            return node;
        }

    private:
        Derived& derived() { return static_cast<Derived&>(*this); }
    };

}
