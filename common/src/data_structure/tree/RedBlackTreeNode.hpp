/**
 * @file RedBlackTreeNode.hpp
 * @brief RedBlackTreeNode class declaration
 * @details This header defines the RedBlackTreeNode class that provides functionality for Advanced data structures including trees and skip lists.
 */

#pragma once
#include <memory>

namespace common::data_structure::tree
{
    /// @brief A node in a Red-Black Tree
    /// @tparam T The type of data stored in the node
    template <typename T>
    class RedBlackTreeNode
    {
    public:
        explicit RedBlackTreeNode(const T& value) noexcept;

        /// @brief Get the data stored in the node
        /// @return The data stored in the node
        [[nodiscard]] T getData() const noexcept;

        /// @brief Set the data stored in the node
        /// @param value The new value to store in the node
        void setData(const T& value) noexcept;

        /// @brief Get the left child node
        /// @return A shared pointer to the left child node
        [[nodiscard]] std::shared_ptr<RedBlackTreeNode> getLeft() const noexcept;

        /// @brief Set the left child node
        /// @param node A shared pointer to the new left child node
        void setLeft(std::shared_ptr<RedBlackTreeNode> node) noexcept;

        /// @brief Get the right child node
        /// @return A shared pointer to the right child node
        [[nodiscard]] std::shared_ptr<RedBlackTreeNode> getRight() const noexcept;

        /// @brief Set the right child node
        /// @param node A shared pointer to the new right child node
        void setRight(std::shared_ptr<RedBlackTreeNode> node) noexcept;

        /// @brief Get the parent node
        /// @return A shared pointer to the parent node
        [[nodiscard]] std::shared_ptr<RedBlackTreeNode> getParent() const noexcept;

        /// @brief Set the parent node
        /// @param node A shared pointer to the new parent node
        void setParent(std::shared_ptr<RedBlackTreeNode> node) noexcept;

        /// @brief Get the color of the node
        /// @return The color of the node
        [[nodiscard]] Color getColor() const noexcept;

        /// @brief Set the color of the node
        /// @param c The new color for the node
        void setColor(Color c) noexcept;

    private:
        T data_{};
        std::shared_ptr<RedBlackTreeNode> left_{};
        std::shared_ptr<RedBlackTreeNode> right_{};
        std::shared_ptr<RedBlackTreeNode> parent_{};
        Color color_{Color::Red};
    };

    template <typename T>
    RedBlackTreeNode<T>::RedBlackTreeNode(const T& value) noexcept : data_(value)
    {
    }

    template <typename T>
    T RedBlackTreeNode<T>::getData() const noexcept
    {
        return data_;
    }

    template <typename T>
    void RedBlackTreeNode<T>::setData(const T& value) noexcept
    {
        data_ = value;
    }

    template <typename T>
    std::shared_ptr<RedBlackTreeNode> RedBlackTreeNode<T>::getLeft() const noexcept
    {
        return left_;
    }

    template <typename T>
    void RedBlackTreeNode<T>::setLeft(std::shared_ptr<RedBlackTreeNode> node) noexcept
    {
        left_ = node;
    }

    template <typename T>
    std::shared_ptr<RedBlackTreeNode> RedBlackTreeNode<T>::getRight() const noexcept
    {
        return right_;
    }

    template <typename T>
    void RedBlackTreeNode<T>::setRight(std::shared_ptr<RedBlackTreeNode> node) noexcept
    {
        right_ = node;
    }

    template <typename T>
    std::shared_ptr<RedBlackTreeNode> RedBlackTreeNode<T>::getParent() const noexcept
    {
        return parent_;
    }

    template <typename T>
    void RedBlackTreeNode<T>::setParent(std::shared_ptr<RedBlackTreeNode> node) noexcept
    {
        parent_ = node;
    }

    template <typename T>
    Color RedBlackTreeNode<T>::getColor() const noexcept
    {
        return color_;
    }

    template <typename T>
    void RedBlackTreeNode<T>::setColor(const Color c) noexcept
    {
        color_ = c;
    }
}