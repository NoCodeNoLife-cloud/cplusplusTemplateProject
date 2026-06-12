/**
 * @file TreeMap.hpp
 * @brief TreeMap class declaration
 * @details This header defines the TreeMap class — an ordered key-value map
 *          backed by an AVL tree, providing O(log n) insert, erase, and lookup.
 */

#pragma once
#include <cstddef>
#include <functional>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "data_structure/tree/AVLTree.hpp"
#include "data_structure/tree/node/TreeNode.hpp"

namespace common::data_structure::map
{
    namespace detail
    {
        /// @brief Internal tree node wrapper enabling key-only comparison
        template <typename K, typename V, typename Compare>
        struct TreeMapEntry
        {
            K key_{};
            V value_{};
            Compare comp_{};

            TreeMapEntry() = default;

            explicit TreeMapEntry(const K& key)
                : key_(key)
            {
            }

            TreeMapEntry(const K& key, const V& value, const Compare& comp = Compare())
                : key_(key), value_(value), comp_(comp)
            {
            }

            auto operator<=>(const TreeMapEntry& other) const
            {
                if (comp_(key_, other.key_)) return std::strong_ordering::less;
                if (comp_(other.key_, key_)) return std::strong_ordering::greater;
                return std::strong_ordering::equal;
            }

            bool operator==(const TreeMapEntry& other) const
            {
                return !comp_(key_, other.key_) && !comp_(other.key_, key_);
            }
        };
    } // namespace detail

    /// @brief An ordered map backed by a self-balancing AVL tree
    /// @tparam K Key type
    /// @tparam V Value type
    /// @tparam Compare Comparison functor, defaults to std::less<K>
    template <typename K, typename V, typename Compare = std::less<K>>
    class TreeMap : private tree::AVLTree<detail::TreeMapEntry<K, V, Compare>>
    {
    private:
        using Entry = detail::TreeMapEntry<K, V, Compare>;
        using Base = tree::AVLTree<Entry>;

    public:
        using key_type = K;
        using mapped_type = V;
        using value_type = std::pair<const K, V>;
        using size_type = std::size_t;

        /// @brief Default constructor creates an empty map
        TreeMap() = default;

        /// @brief Copy constructor creates a deep copy of another map
        /// @param other The map to copy from
        TreeMap(const TreeMap& other)
            : Base()
        {
            map_size_ = other.map_size_;
            this->root_ = clone_node(other.root_.get());
        }

        /// @brief Move constructor transfers ownership from another map
        /// @param other The map to move from
        TreeMap(TreeMap&& other) noexcept
            : Base(std::move(other))
            , map_size_(other.map_size_)
        {
            other.map_size_ = 0;
        }

        /// @brief Copy assignment operator
        /// @param other The map to copy from
        /// @return Reference to this map
        TreeMap& operator=(const TreeMap& other)
        {
            if (this != &other)
            {
                TreeMap copy(other);
                swap(copy);
            }
            return *this;
        }

        /// @brief Move assignment operator
        /// @param other The map to move from
        /// @return Reference to this map
        TreeMap& operator=(TreeMap&& other) noexcept
        {
            TreeMap(std::move(other)).swap(*this);
            return *this;
        }

        // -- Capacity --

        /// @brief Check if the map is empty
        /// @return True if the map is empty, false otherwise
        [[nodiscard]] bool empty() const noexcept
        {
            return map_size_ == 0;
        }

        /// @brief Get the number of key-value pairs in the map
        /// @return The number of elements
        [[nodiscard]] size_type size() const noexcept
        {
            return map_size_;
        }

        // -- Modifiers --

        /// @brief Insert a key-value pair into the map
        /// @param key The key to insert
        /// @param value The value to associate with the key
        /// @return True if inserted, false if the key already existed
        bool insert(const K& key, const V& value)
        {
            if (Base::find(Entry(key))) return false;
            Base::insert(Entry(key, value));
            ++map_size_;
            return true;
        }

        /// @brief Insert a key-value pair, or assign the value if the key exists
        /// @param key The key to insert or assign
        /// @param value The value to associate with the key
        /// @return True if a new element was inserted, false if an existing key was updated
        bool insert_or_assign(const K& key, const V& value)
        {
            auto* node = tree::node::TreeNode<Entry>::findNode(this->root_.get(), Entry(key));
            if (node)
            {
                node->data.value_ = value;
                return false;
            }
            Base::insert(Entry(key, value));
            ++map_size_;
            return true;
        }

        /// @brief Erase a key and its associated value from the map
        /// @param key The key to erase
        /// @return True if the key was erased, false if not found
        bool erase(const K& key)
        {
            if (!Base::find(Entry(key))) return false;
            Base::remove(Entry(key));
            --map_size_;
            return true;
        }

        /// @brief Remove all elements from the map
        void clear() noexcept
        {
            this->root_.reset();
            map_size_ = 0;
        }

        /// @brief Swap the contents of this map with another
        /// @param other The map to swap with
        void swap(TreeMap& other) noexcept
        {
            this->root_.swap(other.root_);
            using std::swap;
            swap(map_size_, other.map_size_);
        }

        // -- Element access --

        /// @brief Access the value for a key, throwing if not found
        /// @param key The key to look up
        /// @return Reference to the mapped value
        /// @throws std::out_of_range If the key does not exist
        V& at(const K& key)
        {
            auto* node = tree::node::TreeNode<Entry>::findNode(this->root_.get(), Entry(key));
            if (!node) throw std::out_of_range("TreeMap::at: key not found");
            return node->data.value_;
        }

        /// @brief Access the value for a key, throwing if not found (const)
        /// @param key The key to look up
        /// @return Const reference to the mapped value
        /// @throws std::out_of_range If the key does not exist
        const V& at(const K& key) const
        {
            const auto* node = tree::node::TreeNode<Entry>::findNode(this->root_.get(), Entry(key));
            if (!node) throw std::out_of_range("TreeMap::at: key not found");
            return node->data.value_;
        }

        /// @brief Access or insert the value for a key
        /// @param key The key to look up or insert
        /// @return Reference to the mapped value (default-inserted if new)
        V& operator[](const K& key)
        {
            auto* node = tree::node::TreeNode<Entry>::findNode(this->root_.get(), Entry(key));
            if (node) return node->data.value_;

            Base::insert(Entry(key));
            ++map_size_;
            node = tree::node::TreeNode<Entry>::findNode(this->root_.get(), Entry(key));
            return node->data.value_;
        }

        // -- Lookup --

        /// @brief Check if a key exists in the map
        /// @param key The key to check
        /// @return True if the key exists, false otherwise
        [[nodiscard]] bool contains(const K& key) const
        {
            return Base::find(Entry(key));
        }

        /// @brief Find the value for a key
        /// @param key The key to find
        /// @return The mapped value if found, std::nullopt otherwise
        [[nodiscard]] std::optional<V> findValue(const K& key) const
        {
            const auto* node = tree::node::TreeNode<Entry>::findNode(this->root_.get(), Entry(key));
            if (node) return node->data.value_;
            return std::nullopt;
        }

        /// @brief Get all key-value pairs in sorted order by key
        /// @return A vector containing all pairs in ascending key order
        [[nodiscard]] std::vector<std::pair<K, V>> toVector() const
        {
            std::vector<std::pair<K, V>> result;
            result.reserve(map_size_);
            inorder_collect(this->root_.get(), result);
            return result;
        }

        /// @brief Get the smallest key and its value
        /// @return The minimum key-value pair, or std::nullopt if empty
        [[nodiscard]] std::optional<std::pair<K, V>> min() const
        {
            if (empty()) return std::nullopt;
            const auto* node = tree::node::TreeNode<Entry>::findMin(this->root_.get());
            return std::pair<K, V>(node->data.key_, node->data.value_);
        }

        /// @brief Get the largest key and its value
        /// @return The maximum key-value pair, or std::nullopt if empty
        [[nodiscard]] std::optional<std::pair<K, V>> max() const
        {
            if (empty()) return std::nullopt;
            const auto* node = this->root_.get();
            while (node->right_) node = node->right_.get();
            return std::pair<K, V>(node->data.key_, node->data.value_);
        }

    private:
        size_type map_size_ = 0;

        /// @brief Deep clones a subtree
        /// @param node The node to clone from
        /// @return A new unique_ptr to the cloned node
        static std::unique_ptr<tree::node::TreeNode<Entry>>
        clone_node(const tree::node::TreeNode<Entry>* node)
        {
            if (!node) return nullptr;
            auto new_node = std::make_unique<tree::node::TreeNode<Entry>>(node->data);
            new_node->left_ = clone_node(node->left_.get());
            new_node->right_ = clone_node(node->right_.get());
            return new_node;
        }

        /// @brief Collects pairs from the tree in inorder (sorted by key)
        /// @param node Current node in the recursion
        /// @param out Output vector to collect pairs into
        static void inorder_collect(const tree::node::TreeNode<Entry>* node,
                                    std::vector<std::pair<K, V>>& out)
        {
            if (!node) return;
            inorder_collect(node->left_.get(), out);
            out.emplace_back(node->data.key_, node->data.value_);
            inorder_collect(node->right_.get(), out);
        }
    };

} // namespace common::data_structure::map
