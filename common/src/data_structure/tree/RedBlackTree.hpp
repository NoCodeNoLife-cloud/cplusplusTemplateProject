#pragma once
#include <glog/logging.h>
#include <fmt/format.h>
#include <memory>

#include "RedBlackTreeNode.hpp"
#include "src/data_structure/Color.hpp"

namespace common::data_structure::tree {
    /// @brief A red-black tree implementation
    /// @tparam T The type of data stored in the tree
    template<typename T>
    class RedBlackTree {
    public:
        RedBlackTree();

        /// @brief Insert a value into the red-black tree
        /// @param value The value to insert
        auto insert(const T &value) -> void;

        /// @brief Get the root node for testing purposes
        /// @return A shared pointer to the root node
        [[nodiscard]] auto getRoot() const noexcept -> std::shared_ptr<RedBlackTreeNode<T> >;

    private:
        std::shared_ptr<RedBlackTreeNode<T> > root_{};

        /// @brief Perform a left rotation on the given node
        /// @param node The node to rotate
        auto leftRotate(std::shared_ptr<RedBlackTreeNode<T> > node) -> void;

        /// @brief Perform a right rotation on the given node
        /// @param node The node to rotate
        auto rightRotate(std::shared_ptr<RedBlackTreeNode<T> > node) -> void;

        /// @brief Fix the red-black tree after insertion
        /// @param node The node that was inserted
        auto fixInsert(std::shared_ptr<RedBlackTreeNode<T> > node) -> void;
    };

    template<typename T>
    RedBlackTree<T>::RedBlackTree() = default;

    template<typename T>
    auto RedBlackTree<T>::insert(const T &value) -> void {
        DLOG(INFO) << fmt::format("RedBlackTree insert - value: {}", value);
        auto node = std::make_shared<RedBlackTreeNode<T> >(value);
        if (!root_) {
            root_ = node;
            node->setColor(Color::Black);
            DLOG(INFO) << "RedBlackTree insert - root node created and colored Black";
            return;
        }
        std::shared_ptr<RedBlackTreeNode<T> > current = root_;
        std::shared_ptr<RedBlackTreeNode<T> > parent;
        while (current) {
            parent = current;
            if (value < current->getData()) {
                current = current->getLeft();
            } else // value >= current->getData()
            {
                current = current->getRight();
            }
        }
        node->setParent(parent);
        if (value < parent->getData()) {
            parent->setLeft(node);
            DLOG(INFO) << fmt::format("RedBlackTree insert - node added as left child of {}", parent->getData());
        } else {
            parent->setRight(node);
            DLOG(INFO) << fmt::format("RedBlackTree insert - node added as right child of {}", parent->getData());
        }
        fixInsert(node);
    }

    template<typename T>
    auto RedBlackTree<T>::getRoot() const noexcept -> std::shared_ptr<RedBlackTreeNode<T> > {
        return root_;
    }

    template<typename T>
    auto RedBlackTree<T>::leftRotate(std::shared_ptr<RedBlackTreeNode<T> > node) -> void {
        DLOG(INFO) << fmt::format("RedBlackTree leftRotate - node value: {}", node->getData());
        auto rightChild = node->getRight();
        node->setRight(rightChild->getLeft());
        if (rightChild->getLeft()) {
            rightChild->getLeft()->setParent(node);
        }
        rightChild->setParent(node->getParent());
        if (!node->getParent()) {
            root_ = rightChild;
            DLOG(INFO) << "RedBlackTree leftRotate - new root is right child";
        } else if (node == node->getParent()->getLeft()) {
            node->getParent()->setLeft(rightChild);
        } else {
            node->getParent()->setRight(rightChild);
        }
        rightChild->setLeft(node);
        node->setParent(rightChild);
    }

    template<typename T>
    auto RedBlackTree<T>::rightRotate(std::shared_ptr<RedBlackTreeNode<T> > node) -> void {
        DLOG(INFO) << fmt::format("RedBlackTree rightRotate - node value: {}", node->getData());
        auto leftChild = node->getLeft();
        node->setLeft(leftChild->getRight());
        if (leftChild->getRight()) {
            leftChild->getRight()->setParent(node);
        }
        leftChild->setParent(node->getParent());
        if (!node->getParent()) {
            root_ = leftChild;
            DLOG(INFO) << "RedBlackTree rightRotate - new root is left child";
        } else if (node == node->getParent()->getLeft()) {
            node->getParent()->setLeft(leftChild);
        } else {
            node->getParent()->setRight(leftChild);
        }
        leftChild->setRight(node);
        node->setParent(leftChild);
    }

    template<typename T>
    auto RedBlackTree<T>::fixInsert(std::shared_ptr<RedBlackTreeNode<T> > node) -> void {
        DLOG(INFO) << fmt::format("RedBlackTree fixInsert - starting with node value: {}", node->getData());
        while (node != root_ && node->getParent() && node->getParent()->getColor() == Color::Red) {
            if (node->getParent() == node->getParent()->getParent()->getLeft()) {
                auto uncle = node->getParent()->getParent()->getRight();
                if (uncle && uncle->getColor() == Color::Red) {
                    DLOG(INFO) << "RedBlackTree fixInsert - Case 1: uncle is red, recoloring";
                    node->getParent()->setColor(Color::Black);
                    uncle->setColor(Color::Black);
                    node->getParent()->getParent()->setColor(Color::Red);
                    node = node->getParent()->getParent();
                } else {
                    if (node == node->getParent()->getRight()) {
                        DLOG(INFO) << "RedBlackTree fixInsert - Case 2: node is right child, left rotating";
                        node = node->getParent();
                        leftRotate(node);
                    }
                    DLOG(INFO) << "RedBlackTree fixInsert - Case 3: right rotating parent's parent";
                    node->getParent()->setColor(Color::Black);
                    if (node->getParent()->getParent()) {
                        node->getParent()->getParent()->setColor(Color::Red);
                    }
                    rightRotate(node->getParent()->getParent());
                }
            } else {
                auto uncle = node->getParent()->getParent()->getLeft();
                if (uncle && uncle->getColor() == Color::Red) {
                    DLOG(INFO) << "RedBlackTree fixInsert - Case 1 (mirror): uncle is red, recoloring";
                    node->getParent()->setColor(Color::Black);
                    uncle->setColor(Color::Black);
                    node->getParent()->getParent()->setColor(Color::Red);
                    node = node->getParent()->getParent();
                } else {
                    if (node == node->getParent()->getLeft()) {
                        DLOG(INFO) << "RedBlackTree fixInsert - Case 2 (mirror): node is left child, right rotating";
                        node = node->getParent();
                        rightRotate(node);
                    }
                    DLOG(INFO) << "RedBlackTree fixInsert - Case 3 (mirror): left rotating parent's parent";
                    node->getParent()->setColor(Color::Black);
                    if (node->getParent()->getParent()) {
                        node->getParent()->getParent()->setColor(Color::Red);
                    }
                    leftRotate(node->getParent()->getParent());
                }
            }
        }
        root_->setColor(Color::Black);
        DLOG(INFO) << "RedBlackTree fixInsert - completed, root colored Black";
    }
}
