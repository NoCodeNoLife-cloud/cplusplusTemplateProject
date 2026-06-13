/**
 * @file TreeSet.hpp
 * @brief AVL-tree-backed ordered set with O(log n) insert/erase/contains
 * @details An ordered set backed by a self-balancing AVL tree.  Elements are
 *          stored in sorted order as determined by T's operator<.  The AVL
 *          balancing invariant guarantees O(log n) worst-case complexity for
 *          insert, erase, and find operations.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Complexity
 * - insert / erase / contains: O(log n) worst-case
 * - min / max:                O(log n) worst-case
 * - toVector:                 O(n)
 *
 * @par Usage Example
 * @code
 * TreeSet<int> numbers;
 * numbers.insert(42);
 * numbers.insert(7);
 * assert(numbers.contains(42));
 * assert(numbers.min() == 7);
 * @endcode
 */

#pragma once
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

#include "data_structure/tree/AVLTree.hpp"
#include "data_structure/tree/node/TreeNode.hpp"

namespace common::data_structure
{
    /// @brief An ordered set backed by a self-balancing AVL tree.
    ///
    /// @tparam T Element type (must be LessThanComparable, i.e. operator<
    ///           must be defined).
    ///
    /// @par Thread Safety
    /// This class is **not** thread-safe.  External synchronisation is required
    /// for concurrent reads and writes.
    ///
    /// @par Memory
    /// Each element is stored in a dynamically allocated tree node.
    /// Insert/erase do not invalidate iterators to other elements.
    template <typename T>
    class TreeSet : private tree::AVLTree<T>
    {
    private:
        using Base = tree::AVLTree<T>;

    public:
        using size_type = std::size_t;

        /// @brief Default constructor creates an empty set
        TreeSet() = default;

        /// @brief Copy constructor creates a deep copy of another set
        /// @param other The set to copy from
        TreeSet(const TreeSet& other)
            : Base()
        {
            set_size_ = other.set_size_;
            this->root_ = clone_node(other.root_.get());
        }

        /// @brief Move constructor transfers ownership from another set
        /// @param other The set to move from
        TreeSet(TreeSet&& other) noexcept
            : Base(std::move(other))
            , set_size_(other.set_size_)
        {
            other.set_size_ = 0;
        }

        /// @brief Copy assignment operator
        /// @param other The set to copy from
        /// @return Reference to this set
        TreeSet& operator=(const TreeSet& other)
        {
            if (this != &other)
            {
                TreeSet copy(other);
                swap(copy);
            }
            return *this;
        }

        /// @brief Move assignment operator
        /// @param other The set to move from
        /// @return Reference to this set
        TreeSet& operator=(TreeSet&& other) noexcept
        {
            TreeSet(std::move(other)).swap(*this);
            return *this;
        }

        /// @brief Insert a value into the set
        /// @param value The value to insert
        /// @return True if the value was inserted, false if it already existed
        [[nodiscard]] bool insert(const T& value)
        {
            if (Base::find(value)) return false;
            Base::insert(value);
            ++set_size_;
            return true;
        }

        /// @brief Erase a value from the set
        /// @param value The value to erase
        /// @return True if the value was erased, false if not found
        [[nodiscard]] bool erase(const T& value)
        {
            if (!Base::find(value)) return false;
            Base::remove(value);
            --set_size_;
            return true;
        }

        /// @brief Check if a value exists in the set
        /// @param value The value to check
        /// @return True if the value exists, false otherwise
        [[nodiscard]] bool contains(const T& value) const
        {
            return Base::find(value);
        }

        // -- Capacity --

        /// @brief Get the number of elements in the set
        /// @return The number of elements
        [[nodiscard]] size_type size() const noexcept
        {
            return set_size_;
        }

        /// @brief Check if the set is empty
        /// @return True if the set is empty, false otherwise
        [[nodiscard]] bool empty() const noexcept
        {
            return set_size_ == 0;
        }

        // -- Modifiers --

        /// @brief Remove all elements from the set
        void clear() noexcept
        {
            this->root_.reset();
            set_size_ = 0;
        }

        /// @brief Swap the contents of this set with another
        /// @param other The set to swap with
        void swap(TreeSet& other) noexcept
        {
            this->root_.swap(other.root_);
            using std::swap;
            swap(set_size_, other.set_size_);
        }

        /// @brief Find the value in the set
        /// @param value The value to find
        /// @return The value if found, std::nullopt otherwise
        [[nodiscard]] std::optional<T> findValue(const T& value) const
        {
            const auto* node = tree::node::TreeNode<T>::findNode(this->root_.get(), value);
            if (node) return node->data;
            return std::nullopt;
        }

        /// @brief Get all elements in sorted order
        /// @return A vector containing all elements in ascending order
        [[nodiscard]] std::vector<T> toVector() const
        {
            std::vector<T> result;
            result.reserve(set_size_);
            inorder_collect(this->root_.get(), result);
            return result;
        }

        /// @brief Get the minimum element in the set
        /// @return The minimum element, or std::nullopt if the set is empty
        [[nodiscard]] std::optional<T> min() const
        {
            if (empty()) return std::nullopt;
            const auto* node = tree::node::TreeNode<T>::findMin(this->root_.get());
            return node->data;
        }

        /// @brief Get the maximum element in the set
        /// @return The maximum element, or std::nullopt if the set is empty
        [[nodiscard]] std::optional<T> max() const
        {
            if (empty()) return std::nullopt;
            const auto* node = this->root_.get();
            while (node->right_) node = node->right_.get();
            return node->data;
        }

    private:
        size_type set_size_ = 0; ///< Number of elements in the set.

        // ── Internal helpers ───────────────────────────────────────────

        /// @brief Deep clones a subtree rooted at @p node.
        /// @param node  Node to clone (may be null).
        /// @return New unique_ptr owning a deep copy of the subtree.
        static std::unique_ptr<tree::node::TreeNode<T>>
        clone_node(const tree::node::TreeNode<T>* node)
        {
            if (!node) return nullptr;
            // Preorder: copy the current node, then recursively copy children.
            auto new_node = std::make_unique<tree::node::TreeNode<T>>(node->data);
            new_node->left_ = clone_node(node->left_.get());
            new_node->right_ = clone_node(node->right_.get());
            return new_node;
        }

        /// @brief Collects elements via inorder traversal (ascending order).
        /// @param node  Current node (may be null).
        /// @param out   Output vector receiving elements in sorted order.
        static void inorder_collect(const tree::node::TreeNode<T>* node,
                                    std::vector<T>& out)
        {
            if (!node) return;
            inorder_collect(node->left_.get(), out);
            out.push_back(node->data);
            inorder_collect(node->right_.get(), out);
        }
    };
}
