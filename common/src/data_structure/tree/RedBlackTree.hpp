/**
 * @file RedBlackTree.hpp
 * @brief RedBlackTree class declaration
 * @details This header defines the RedBlackTree class that provides functionality for Advanced data structures including trees and skip lists.
 */

#pragma once
#include <memory>

#include "RedBlackTreeNode.hpp"
#include "src/data_structure/Color.hpp"

namespace common::data_structure::tree
{
    /// @brief A red-black tree implementation
    /// @tparam T The type of data stored in the tree
    template <typename T>
    class RedBlackTree
    {
    public:
        RedBlackTree();

        /// @brief Insert a value into the red-black tree
        /// @param value The value to insert
        void insert(const T& value);

        /// @brief Get the root node for testing purposes
        /// @return A shared pointer to the root node
        [[nodiscard]] std::shared_ptr<RedBlackTreeNode<T>> getRoot() const noexcept;

    private:
        std::shared_ptr<RedBlackTreeNode<T>> root_{};

        /// @brief Perform a left rotation on the given node
        /// @param node The node to rotate
        void leftRotate(std::shared_ptr<RedBlackTreeNode<T>> node);

        /// @brief Perform a right rotation on the given node
        /// @param node The node to rotate
        void rightRotate(std::shared_ptr<RedBlackTreeNode<T>> node);

        /// @brief Fix the red-black tree after insertion
        /// @param node The node that was inserted
        void fixInsert(std::shared_ptr<RedBlackTreeNode<T>> node);
    };

    template <typename T>
    RedBlackTree<T>::RedBlackTree() = default;

    template <typename T>
    void RedBlackTree<T>::insert(const T& value)
    {
        auto node = std::make_shared<RedBlackTreeNode<T>>(value);
        if (!root_)
        {
            root_ = node;
            node->setColor(Color::Black);
            return;
        }
        std::shared_ptr<RedBlackTreeNode<T>> current = root_;
        std::shared_ptr<RedBlackTreeNode<T>> parent;
        while (current)
        {
            parent = current;
            if (value < current->getData())
            {
                current = current->getLeft();
            }
            else // value >= current->getData()
            {
                current = current->getRight();
            }
        }
        node->setParent(parent);
        if (value < parent->getData())
        {
            parent->setLeft(node);
        }
        else
        {
            parent->setRight(node);
        }
        fixInsert(node);
    }

    template <typename T>
    std::shared_ptr<RedBlackTreeNode<T>> RedBlackTree<T>::getRoot() const noexcept
    {
        return root_;
    }

    template <typename T>
    void RedBlackTree<T>::leftRotate(std::shared_ptr<RedBlackTreeNode<T>> node)
    {
        auto rightChild = node->getRight();
        node->setRight(rightChild->getLeft());
        if (rightChild->getLeft())
        {
            rightChild->getLeft()->setParent(node);
        }
        rightChild->setParent(node->getParent());
        if (!node->getParent())
        {
            root_ = rightChild;
        }
        else if (node == node->getParent()->getLeft())
        {
            node->getParent()->setLeft(rightChild);
        }
        else
        {
            node->getParent()->setRight(rightChild);
        }
        rightChild->setLeft(node);
        node->setParent(rightChild);
    }

    template <typename T>
    void RedBlackTree<T>::rightRotate(std::shared_ptr<RedBlackTreeNode<T>> node)
    {
        auto leftChild = node->getLeft();
        node->setLeft(leftChild->getRight());
        if (leftChild->getRight())
        {
            leftChild->getRight()->setParent(node);
        }
        leftChild->setParent(node->getParent());
        if (!node->getParent())
        {
            root_ = leftChild;
        }
        else if (node == node->getParent()->getLeft())
        {
            node->getParent()->setLeft(leftChild);
        }
        else
        {
            node->getParent()->setRight(leftChild);
        }
        leftChild->setRight(node);
        node->setParent(leftChild);
    }

    template <typename T>
    void RedBlackTree<T>::fixInsert(std::shared_ptr<RedBlackTreeNode<T>> node)
    {
        while (node != root_ && node->getParent() && node->getParent()->getColor() == Color::Red)
        {
            if (node->getParent() == node->getParent()->getParent()->getLeft())
            {
                auto uncle = node->getParent()->getParent()->getRight();
                if (uncle && uncle->getColor() == Color::Red)
                {
                    node->getParent()->setColor(Color::Black);
                    uncle->setColor(Color::Black);
                    node->getParent()->getParent()->setColor(Color::Red);
                    node = node->getParent()->getParent();
                }
                else
                {
                    if (node == node->getParent()->getRight())
                    {
                        node = node->getParent();
                        leftRotate(node);
                    }
                    node->getParent()->setColor(Color::Black);
                    if (node->getParent()->getParent())
                    {
                        node->getParent()->getParent()->setColor(Color::Red);
                    }
                    rightRotate(node->getParent()->getParent());
                }
            }
            else
            {
                auto uncle = node->getParent()->getParent()->getLeft();
                if (uncle && uncle->getColor() == Color::Red)
                {
                    node->getParent()->setColor(Color::Black);
                    uncle->setColor(Color::Black);
                    node->getParent()->getParent()->setColor(Color::Red);
                    node = node->getParent()->getParent();
                }
                else
                {
                    if (node == node->getParent()->getLeft())
                    {
                        node = node->getParent();
                        rightRotate(node);
                    }
                    node->getParent()->setColor(Color::Black);
                    if (node->getParent()->getParent())
                    {
                        node->getParent()->getParent()->setColor(Color::Red);
                    }
                    leftRotate(node->getParent()->getParent());
                }
            }
        }
        root_->setColor(Color::Black);
    }
}
