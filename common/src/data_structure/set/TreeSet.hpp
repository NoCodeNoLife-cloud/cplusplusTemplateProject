/**
 * @file TreeSet.hpp
 * @brief TreeSet class declaration
 * @details This header defines the TreeSet class — an ordered set implementation
 *          backed by an AVL tree, providing O(log n) insert, erase, and lookup.
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
    /// @brief An ordered set backed by a self-balancing AVL tree
    /// @tparam T The element type (must be LessThanComparable)
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
        bool insert(const T& value)
        {
            if (Base::find(value)) return false;
            Base::insert(value);
            ++set_size_;
            return true;
        }

        /// @brief Erase a value from the set
        /// @param value The value to erase
        /// @return True if the value was erased, false if not found
        bool erase(const T& value)
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
        size_type set_size_ = 0;

        /// @brief Deep clones a subtree
        /// @param node The node to clone from
        /// @return A new unique_ptr to the cloned node
        static std::unique_ptr<tree::node::TreeNode<T>>
        clone_node(const tree::node::TreeNode<T>* node)
        {
            if (!node) return nullptr;
            auto new_node = std::make_unique<tree::node::TreeNode<T>>(node->data);
            new_node->left_ = clone_node(node->left_.get());
            new_node->right_ = clone_node(node->right_.get());
            return new_node;
        }

        /// @brief Collects elements from the tree in inorder (sorted order)
        /// @param node Current node in the recursion
        /// @param out Output vector to collect elements into
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
