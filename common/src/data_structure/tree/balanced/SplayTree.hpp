/**
 * @file SplayTree.hpp
 * @brief Self-adjusting splay tree â€?amortised O(log n) ops with locality
 * @details A self-adjusting BST that moves recently accessed nodes to the root
 *          via splay rotations.  Frequently accessed elements are found faster
 *          (working-set property).  All operations have amortised O(log n)
 *          complexity.  Does not require per-node balance metadata.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Complexity
 * - search / insert / remove: amortised O(log n)
 *
 * @par Reference
 * Sleator & Tarjan, "Self-Adjusting Binary Search Trees"
 * (1985), Journal of the ACM 32(3).
 */

#pragma once
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "data_structure/tree/node/TreeNode.hpp"

namespace cppforge::data_structure::tree::balanced
{
    using tree::node::TreeNode;
    /// @brief A self-adjusting binary search tree (Splay Tree)
    /// @details Splay trees are self-adjusting BSTs that move recently accessed elements
    ///          to the root via rotations (zig, zig-zig, zig-zag patterns). Frequently accessed
    ///          elements cluster near the root, giving amortized O(log n) performance.
    ///          Unlike AVL or Red-Black trees, no balance information is stored per node.
    /// @tparam T The type of data stored in the tree. Must be comparable with <, >, and == operators.
    template <typename T>
    class SplayTree
    {
    public:
        SplayTree() = default;

        SplayTree(SplayTree&& other) noexcept
            : root_(std::move(other.root_))
            , size_(std::exchange(other.size_, 0))
        {
        }

        SplayTree& operator=(SplayTree&& other) noexcept
        {
            if (this != &other)
            {
                root_ = std::move(other.root_);
                size_ = std::exchange(other.size_, 0);
            }
            return *this;
        }

        SplayTree(const SplayTree&) = delete;
        SplayTree& operator=(const SplayTree&) = delete;

        /// @brief Insert a value into the tree, then splay it to the root
        /// @param value The value to insert
        void insert(const T& value)
        {
            root_ = insertImpl(std::move(root_), value);
        }

        /// @brief Remove a value from the tree
        /// @details Splay the value to the root, then remove it. The predecessor
        ///          (maximum of left subtree) becomes the new root.
        /// @param value The value to remove
        void remove(const T& value)
        {
            if (!root_) return;
            root_ = splay(std::move(root_), value);
            if (root_ && root_->data == value)
            {
                if (!root_->left_) // NOLINT(bugprone-branch-clone)
                {
                    root_ = std::move(root_->right_);
                }
                else if (!root_->right_)
                {
                    root_ = std::move(root_->left_);
                }
                else
                {
                    auto new_root = splay(std::move(root_->left_), value);
                    new_root->right_ = std::move(root_->right_);
                    root_ = std::move(new_root);
                }
                --size_;
            }
        }

        /// @brief Find a value and splay the accessed node to the root
        /// @param value The value to find
        /// @return true if the value exists, false otherwise
        [[nodiscard]] bool find(const T& value)
        {
            if (!root_) return false;
            root_ = splay(std::move(root_), value);
            return root_->data == value;
        }

        /// @brief Find a value, splay the accessed node, and return the stored value
        /// @param value The value to find
        /// @return The stored value if found, std::nullopt otherwise
        [[nodiscard]] std::optional<T> findValue(const T& value)
        {
            if (!root_) return std::nullopt;
            root_ = splay(std::move(root_), value);
            if (root_->data == value)
            {
                return root_->data;
            }
            return std::nullopt;
        }

        /// @brief Check if the tree is empty
        [[nodiscard]] bool empty() const
        {
            return size_ == 0;
        }

        /// @brief Get the number of nodes in the tree
        [[nodiscard]] size_t size() const
        {
            return size_;
        }

        /// @brief Remove all nodes from the tree
        void clear()
        {
            root_.reset();
            size_ = 0;
        }

        /// @brief Return the inorder traversal as a vector
        /// @return Vector of elements in sorted order
        [[nodiscard]] std::vector<T> inorder() const
        {
            std::vector<T> result;
            inorderRecursive(root_.get(), result);
            return result;
        }

    private:
        std::unique_ptr<TreeNode<T>> root_{};
        size_t size_{0};

        void inorderRecursive(const TreeNode<T>* node, std::vector<T>& result) const
        {
            if (!node) return;
            inorderRecursive(node->left_.get(), result);
            result.push_back(node->data);
            inorderRecursive(node->right_.get(), result);
        }

        std::unique_ptr<TreeNode<T>> insertImpl(std::unique_ptr<TreeNode<T>> node, const T& value)
        {
            if (!node)
            {
                ++size_;
                return std::make_unique<TreeNode<T>>(value);
            }
            if (value < node->data) // NOLINT(bugprone-branch-clone)
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
            return splay(std::move(node), value);
        }

        std::unique_ptr<TreeNode<T>> splay(std::unique_ptr<TreeNode<T>> node, const T& value)
        {
            if (!node || node->data == value) return node;

            if (value < node->data)
            {
                if (!node->left_) return node;

                if (value < node->left_->data)
                {
                    node->left_->left_ = splay(std::move(node->left_->left_), value);
                    node = rotateRight(std::move(node));
                }
                else if (value > node->left_->data)
                {
                    node->left_->right_ = splay(std::move(node->left_->right_), value);
                    if (node->left_->right_)
                    {
                        node->left_ = rotateLeft(std::move(node->left_));
                    }
                }

                if (node->left_)
                {
                    return rotateRight(std::move(node));
                }
                return node;
            }

            if (!node->right_) return node;

            if (value > node->right_->data)
            {
                node->right_->right_ = splay(std::move(node->right_->right_), value);
                node = rotateLeft(std::move(node));
            }
            else if (value < node->right_->data)
            {
                node->right_->left_ = splay(std::move(node->right_->left_), value);
                if (node->right_->left_)
                {
                    node->right_ = rotateRight(std::move(node->right_));
                }
            }

            if (node->right_)
            {
                return rotateLeft(std::move(node));
            }
            return node;
        }

        std::unique_ptr<TreeNode<T>> rotateRight(std::unique_ptr<TreeNode<T>> node)
        {
            auto left = std::move(node->left_);
            node->left_ = std::move(left->right_);
            left->right_ = std::move(node);
            return left;
        }

        std::unique_ptr<TreeNode<T>> rotateLeft(std::unique_ptr<TreeNode<T>> node)
        {
            auto right = std::move(node->right_);
            node->right_ = std::move(right->left_);
            right->left_ = std::move(node);
            return right;
        }
    };
}
