/**
 * @file RedBlackTree.hpp
 * @brief Red-Black tree â€?self-balancing BST with O(log n) guaranteed ops
 * @details A self-balancing binary search tree that maintains five red-black
 *          invariants (root is black, no adjacent red nodes, equal black-height
 *          on every path).  Guarantees O(log n) worst-case insert, erase, and
 *          find without requiring the rigid rebalancing of AVL trees.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Complexity
 * - insert / remove / find: O(log n) worst-case
 *
 * @par Reference
 * Guibas & Sedgewick, "A Dichromatic Framework for Balanced Trees"
 * (1978), FOCS.
 */

#pragma once
#include <memory>

#include "data_structure/base_type/Color.hpp"
#include "data_structure/tree/node/RedBlackTreeNode.hpp"

namespace cppforge::data_structure::tree::balanced
{
    using tree::node::RedBlackTreeNode;
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

        /// @brief Remove a value from the red-black tree
        /// @param value The value to remove
        void remove(const T& value);

        /// @brief Get the root node for testing purposes
        /// @return A shared pointer to the root node
        [[nodiscard]] std::shared_ptr<RedBlackTreeNode<T>> getRoot() const ;

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

        /// @brief Fix the red-black tree after removal
        /// @param node The replacement node (or its parent) that may violate RB properties
        /// @param parent The parent of the replacement node
        /// @param is_left Whether the replacement node is a left child of parent
        void fixDelete(std::shared_ptr<RedBlackTreeNode<T>> node,
                       std::shared_ptr<RedBlackTreeNode<T>> parent,
                       bool is_left);

        /// @brief Find the minimum node in a subtree
        /// @param node The root of the subtree
        /// @return The minimum node
        [[nodiscard]] std::shared_ptr<RedBlackTreeNode<T>> findMinNode(
            std::shared_ptr<RedBlackTreeNode<T>> node) const;
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
    std::shared_ptr<RedBlackTreeNode<T>> RedBlackTree<T>::getRoot() const
    {
        return root_;
    }

    template <typename T>
    void RedBlackTree<T>::remove(const T& value)
    {
        if (!root_) return;

        std::shared_ptr<RedBlackTreeNode<T>> node = root_;
        std::shared_ptr<RedBlackTreeNode<T>> parent;
        bool is_left = false;

        // Find the node to delete
        while (node)
        {
            if (value == node->getData()) break;
            parent = node;
            if (value < node->getData())
            {
                is_left = true;
                node = node->getLeft();
            }
            else
            {
                is_left = false;
                node = node->getRight();
            }
        }

        if (!node) return; // value not found

        Color deleted_color = node->getColor();
        std::shared_ptr<RedBlackTreeNode<T>> replacement;
        std::shared_ptr<RedBlackTreeNode<T>> repl_parent;
        bool repl_is_left = false;

        if (!node->getLeft() && !node->getRight())
        {
            // Leaf node - just remove
            replacement = nullptr;
            repl_parent = parent;
            repl_is_left = is_left;
            if (parent)
            {
                if (is_left) parent->setLeft(nullptr);
                else parent->setRight(nullptr);
            }
            else
            {
                root_ = nullptr;
            }
        }
        else if (!node->getLeft())
        {
            // Only right child
            replacement = node->getRight();
            repl_parent = parent;
            repl_is_left = is_left;
            if (parent)
            {
                if (is_left) parent->setLeft(replacement);
                else parent->setRight(replacement);
            }
            else
            {
                root_ = replacement;
            }
            replacement->setParent(parent);
        }
        else if (!node->getRight())
        {
            // Only left child
            replacement = node->getLeft();
            repl_parent = parent;
            repl_is_left = is_left;
            if (parent)
            {
                if (is_left) parent->setLeft(replacement);
                else parent->setRight(replacement);
            }
            else
            {
                root_ = replacement;
            }
            replacement->setParent(parent);
        }
        else
        {
            // Two children: find inorder successor (minimum of right subtree)
            auto successor = findMinNode(node->getRight());
            deleted_color = successor->getColor();
            replacement = successor->getRight();
            repl_parent = successor;
            repl_is_left = false;

            if (successor->getParent() == node)
            {
                // Successor is direct right child
                if (replacement)
                {
                    replacement->setParent(successor);
                }
                repl_parent = successor;
                repl_is_left = false;
            }
            else
            {
                // Successor is deeper in the right subtree
                if (successor->getParent()->getLeft() == successor)
                {
                    successor->getParent()->setLeft(replacement);
                    repl_is_left = true;
                }
                else
                {
                    successor->getParent()->setRight(replacement);
                    repl_is_left = false;
                }
                if (replacement)
                {
                    replacement->setParent(successor->getParent());
                }
                repl_parent = successor->getParent();

                // Link successor's right to node's right
                successor->setRight(node->getRight());
                if (node->getRight())
                {
                    node->getRight()->setParent(successor);
                }
            }

            // Replace node with successor
            successor->setLeft(node->getLeft());
            if (node->getLeft())
            {
                node->getLeft()->setParent(successor);
            }
            successor->setParent(node->getParent());
            successor->setColor(node->getColor());

            if (!node->getParent())
            {
                root_ = successor;
            }
            else if (node == node->getParent()->getLeft())
            {
                node->getParent()->setLeft(successor);
            }
            else
            {
                node->getParent()->setRight(successor);
            }

            // The node we're removing's parent reference was for the successor swap
            // replacement is now tracked via repl_parent/repl_is_left
        }

        if (deleted_color == Color::Black)
        {
            fixDelete(replacement, repl_parent, repl_is_left);
        }
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

    template <typename T>
    void RedBlackTree<T>::fixDelete(std::shared_ptr<RedBlackTreeNode<T>> node,
                                     std::shared_ptr<RedBlackTreeNode<T>> parent,
                                     bool /*is_left*/)
    {
        while (node != root_ && (!node || node->getColor() == Color::Black))
        {
            const bool is_left_child = parent && node == parent->getLeft();

            if (is_left_child)
            {
                auto sibling = parent ? parent->getRight() : nullptr;
                if (sibling && sibling->getColor() == Color::Red)
                {
                    sibling->setColor(Color::Black);
                    parent->setColor(Color::Red);
                    leftRotate(parent);
                    sibling = parent->getRight();
                }
                if (!sibling ||
                    (!sibling->getLeft() || sibling->getLeft()->getColor() == Color::Black) &&
                    (!sibling->getRight() || sibling->getRight()->getColor() == Color::Black))
                {
                    if (sibling) sibling->setColor(Color::Red);
                    node = parent;
                    parent = node ? node->getParent() : nullptr;
                }
                else
                {
                    if (!sibling->getRight() || sibling->getRight()->getColor() == Color::Black)
                    {
                        if (sibling->getLeft()) sibling->getLeft()->setColor(Color::Black);
                        sibling->setColor(Color::Red);
                        rightRotate(sibling);
                        sibling = parent->getRight();
                    }
                    if (sibling)
                    {
                        sibling->setColor(parent ? parent->getColor() : Color::Black);
                        if (sibling->getRight()) sibling->getRight()->setColor(Color::Black);
                    }
                    if (parent) parent->setColor(Color::Black);
                    if (parent) leftRotate(parent);
                    node = root_;
                }
            }
            else
            {
                auto sibling = parent ? parent->getLeft() : nullptr;
                if (sibling && sibling->getColor() == Color::Red)
                {
                    sibling->setColor(Color::Black);
                    parent->setColor(Color::Red);
                    rightRotate(parent);
                    sibling = parent->getLeft();
                }
                if (!sibling ||
                    (!sibling->getLeft() || sibling->getLeft()->getColor() == Color::Black) &&
                    (!sibling->getRight() || sibling->getRight()->getColor() == Color::Black))
                {
                    if (sibling) sibling->setColor(Color::Red);
                    node = parent;
                    parent = node ? node->getParent() : nullptr;
                }
                else
                {
                    if (!sibling->getLeft() || sibling->getLeft()->getColor() == Color::Black)
                    {
                        if (sibling->getRight()) sibling->getRight()->setColor(Color::Black);
                        sibling->setColor(Color::Red);
                        leftRotate(sibling);
                        sibling = parent->getLeft();
                    }
                    if (sibling)
                    {
                        sibling->setColor(parent ? parent->getColor() : Color::Black);
                        if (sibling->getLeft()) sibling->getLeft()->setColor(Color::Black);
                    }
                    if (parent) parent->setColor(Color::Black);
                    if (parent) rightRotate(parent);
                    node = root_;
                }
            }
        }
        if (node) node->setColor(Color::Black);
    }

    template <typename T>
    std::shared_ptr<RedBlackTreeNode<T>> RedBlackTree<T>::findMinNode(
        std::shared_ptr<RedBlackTreeNode<T>> node) const
    {
        if (!node) return nullptr;
        while (node->getLeft())
        {
            node = node->getLeft();
        }
        return node;
    }
}
