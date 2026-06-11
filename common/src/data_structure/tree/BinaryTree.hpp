/**
 * @file BinaryTree.hpp
 * @brief BinaryTree class declaration
 * @details This header defines the BinaryTree class that provides functionality for Advanced data structures including trees and skip lists.
 */

#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>

#include "data_structure/tree/TreeToolkit.hpp"

namespace common::data_structure::tree
{
    /// @brief A general binary tree implementation.
    /// Unlike BinarySearchTree, this tree does not enforce any ordering invariant.
    /// It supports arbitrary hierarchical structures useful for expression trees,
    /// Huffman trees, parse trees, and other non-search applications.
    /// Each BinaryTree object is a node that owns its left and right subtrees.
    /// @tparam T The type of data stored in each node
    template <typename T>
    class BinaryTree
    {
        // Grant access for generic tree algorithm helpers
        friend class TreeToolkit;

    public:
        BinaryTree() = default;

        /// @brief Construct a binary tree with a single node
        /// @param value The value for the node
        explicit BinaryTree(T value);

        /// @brief Get the value stored at this node
        /// @return Reference to the value
        [[nodiscard]] T& data();

        /// @brief Get the value stored at this node (const)
        /// @return Const reference to the value
        [[nodiscard]] const T& data() const;

        /// @brief Get a non-owning pointer to the left subtree
        /// @return Pointer to the left subtree, or nullptr
        [[nodiscard]] BinaryTree<T>* left() const;

        /// @brief Get a non-owning pointer to the right subtree
        /// @return Pointer to the right subtree, or nullptr
        [[nodiscard]] BinaryTree<T>* right() const;

        /// @brief Set the left subtree (takes ownership)
        /// @param subtree Unique pointer to the new left subtree
        void setLeft(std::unique_ptr<BinaryTree<T>> subtree);

        /// @brief Set the right subtree (takes ownership)
        /// @param subtree Unique pointer to the new right subtree
        void setRight(std::unique_ptr<BinaryTree<T>> subtree);

        /// @brief Preorder traversal: root -> left -> right
        /// @return Vector of values in preorder
        [[nodiscard]] std::vector<T> preorder() const;

        /// @brief Inorder traversal: left -> root -> right
        /// @return Vector of values in inorder
        [[nodiscard]] std::vector<T> inorder() const;

        /// @brief Postorder traversal: left -> right -> root
        /// @return Vector of values in postorder
        [[nodiscard]] std::vector<T> postorder() const;

        /// @brief Level-order traversal (breadth-first)
        /// @return Vector of values in level order
        [[nodiscard]] std::vector<T> levelOrder() const;

        /// @brief Check if this node has no children
        /// @return True if the node is a leaf
        [[nodiscard]] bool isLeaf() const noexcept;

        /// @brief Get the height of the tree rooted at this node
        /// @return The height (number of edges on the longest root-to-leaf path)
        [[nodiscard]] int32_t height() const;

        /// @brief Get the number of nodes in the tree rooted at this node
        /// @return The total node count
        [[nodiscard]] size_t size() const;

        /// @brief Remove all children from this node
        void clear() noexcept;

    private:
        T data_{};
        std::unique_ptr<BinaryTree<T>> left_{};
        std::unique_ptr<BinaryTree<T>> right_{};

        /// @brief Recursively collect preorder values
        static void preorderRecursive(const BinaryTree<T>* node, std::vector<T>& result);

        /// @brief Recursively collect inorder values
        static void inorderRecursive(const BinaryTree<T>* node, std::vector<T>& result);

        /// @brief Recursively collect postorder values
        static void postorderRecursive(const BinaryTree<T>* node, std::vector<T>& result);


    };

    template <typename T>
    BinaryTree<T>::BinaryTree(T value)
        : data_(std::move(value))
    {
    }

    template <typename T>
    T& BinaryTree<T>::data()
    {
        return data_;
    }

    template <typename T>
    const T& BinaryTree<T>::data() const
    {
        return data_;
    }

    template <typename T>
    BinaryTree<T>* BinaryTree<T>::left() const
    {
        return left_.get();
    }

    template <typename T>
    BinaryTree<T>* BinaryTree<T>::right() const
    {
        return right_.get();
    }

    template <typename T>
    void BinaryTree<T>::setLeft(std::unique_ptr<BinaryTree<T>> subtree)
    {
        left_ = std::move(subtree);
    }

    template <typename T>
    void BinaryTree<T>::setRight(std::unique_ptr<BinaryTree<T>> subtree)
    {
        right_ = std::move(subtree);
    }

    template <typename T>
    std::vector<T> BinaryTree<T>::preorder() const
    {
        std::vector<T> result;
        preorderRecursive(this, result);
        return result;
    }

    template <typename T>
    void BinaryTree<T>::preorderRecursive(const BinaryTree<T>* node, std::vector<T>& result)
    {
        if (!node) return;
        result.push_back(node->data_);
        preorderRecursive(node->left_.get(), result);
        preorderRecursive(node->right_.get(), result);
    }

    template <typename T>
    std::vector<T> BinaryTree<T>::inorder() const
    {
        std::vector<T> result;
        inorderRecursive(this, result);
        return result;
    }

    template <typename T>
    void BinaryTree<T>::inorderRecursive(const BinaryTree<T>* node, std::vector<T>& result)
    {
        if (!node) return;
        inorderRecursive(node->left_.get(), result);
        result.push_back(node->data_);
        inorderRecursive(node->right_.get(), result);
    }

    template <typename T>
    std::vector<T> BinaryTree<T>::postorder() const
    {
        std::vector<T> result;
        postorderRecursive(this, result);
        return result;
    }

    template <typename T>
    void BinaryTree<T>::postorderRecursive(const BinaryTree<T>* node, std::vector<T>& result)
    {
        if (!node) return;
        postorderRecursive(node->left_.get(), result);
        postorderRecursive(node->right_.get(), result);
        result.push_back(node->data_);
    }

    template <typename T>
    std::vector<T> BinaryTree<T>::levelOrder() const
    {
        std::vector<T> result;

        std::queue<const BinaryTree<T>*> q;
        q.push(this);

        while (!q.empty())
        {
            const auto* node = q.front();
            q.pop();
            result.push_back(node->data_);

            if (node->left_) q.push(static_cast<const BinaryTree<T>*>(node->left_.get()));
            if (node->right_) q.push(static_cast<const BinaryTree<T>*>(node->right_.get()));
        }

        return result;
    }

    template <typename T>
    bool BinaryTree<T>::isLeaf() const noexcept
    {
        return left_ == nullptr && right_ == nullptr;
    }

    template <typename T>
    int32_t BinaryTree<T>::height() const
    {
        return TreeToolkit::height(this);
    }

    template <typename T>
    size_t BinaryTree<T>::size() const
    {
        return TreeToolkit::size(this);
    }

    template <typename T>
    void BinaryTree<T>::clear() noexcept
    {
        left_.reset();
        right_.reset();
    }
}
