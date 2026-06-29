/**
 * @file HashSet.hpp
 * @brief Robin-hood-hashed set with O(1) average insert/erase/find
 * @details A hash set wrapping RobinHoodHashMap with a key-only interface.
 *          Provides O(1) average-case insert, erase, and containment checks
 *          with good cache locality due to open addressing.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Usage Example
 * @code
 * HashSet<std::string> tags;
 * tags.insert("c++");
 * tags.insert("template");
 * assert(tags.contains("c++"));
 * @endcode
 */

#pragma once
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>

#include <cppforge/data_structure/hash/RobinHoodHashMap.hpp>

namespace cppforge::data_structure
{
    /// @brief A hash set built on Robin Hood open-addressing hashing.
    ///
    /// @tparam T        Element type (must be copyable).
    /// @tparam Hasher   Hash functor, defaults to std::hash<T>.
    /// @tparam KeyEqual Equality comparator, defaults to std::equal_to<T>.
    ///
    /// @par Thread Safety
    /// This class is **not** thread-safe.  External synchronisation is required
    /// for concurrent reads and writes.
    ///
    /// @par Iterator Invalidation
    /// Insert may cause rehashing, which invalidates all iterators.
    /// Erase does not invalidate iterators except to the erased element.
    template <typename T, typename Hasher = std::hash<T>, typename KeyEqual = std::equal_to<T>>
    class HashSet
    {
    public:
        using size_type = std::size_t;

    private:
        using MapType = hash::RobinHoodHashMap<T, bool, Hasher, KeyEqual>;

    public:
        /// @brief Forward iterator over set elements
        class iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = const T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T*;
            using reference = const T&;

            iterator() = default;

            reference operator*() const { return (*map_it_).first; }
            pointer operator->() const { return &(*map_it_).first; }

            iterator& operator++() { ++map_it_; return *this; }
            iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }

            bool operator==(const iterator& other) const { return map_it_ == other.map_it_; }
            bool operator!=(const iterator& other) const { return map_it_ != other.map_it_; }

        private:
            friend class HashSet;
            using MapIter = typename MapType::iterator;
            explicit iterator(MapIter it) : map_it_(it) {}
            MapIter map_it_;
        };

        using const_iterator = iterator;

        /// @brief Default constructor creates an empty set
        HashSet() = default;

        /// @brief Constructs a set with a given initial capacity
        /// @param capacity Minimum initial capacity
        explicit HashSet(size_type capacity) : map_(capacity) {}

        /// @brief Insert a value into the set
        /// @param value The value to insert
        /// @return A pair of iterator to the element and a bool (true if inserted)
        [[nodiscard]] std::pair<iterator, bool> insert(const T& value)
        {
            auto map_result = map_.insert(value, false);
            return {iterator(map_result.first), map_result.second};
        }

        /// @brief Insert a value into the set using move semantics
        /// @param value The value to move into the set
        /// @return A pair of iterator to the element and a bool (true if inserted)
        [[nodiscard]] std::pair<iterator, bool> insert(T&& value)
        {
            auto map_result = map_.insert(std::move(value), false);
            return {iterator(map_result.first), map_result.second};
        }

        /// @brief Erase the element with the given value from the set
        /// @param value The value to erase
        /// @return True if the element was erased, false if not found
        [[nodiscard]] bool erase(const T& value)
        {
            return map_.erase(value);
        }

        /// @brief Erase the element at the given iterator position
        /// @param it Iterator to the element to erase
        /// @return Iterator to the next occupied element
        [[nodiscard]] iterator erase(const_iterator it)
        {
            return iterator(map_.erase(it.map_it_));
        }

        /// @brief Check if a value exists in the set
        /// @param value The value to check
        /// @return True if the value exists, false otherwise
        [[nodiscard]] bool contains(const T& value) const
        {
            return map_.contains(value);
        }

        /// @brief Find an element by value
        /// @param value The value to find
        /// @return Iterator to the element, or end() if not found
        [[nodiscard]] iterator find(const T& value)
        {
            auto map_it = map_.find(value);
            if (map_it == map_.end())
            {
                return end();
            }
            return iterator(map_it);
        }

        /// @brief Find an element by value (const)
        /// @param value The value to find
        /// @return Const iterator to the element, or end() if not found
        [[nodiscard]] const_iterator find(const T& value) const
        {
            // Need to use non-const find since iterator stores MapType::iterator
            auto& mutable_map = const_cast<HashSet*>(this)->map_;
            auto map_it = mutable_map.find(value);
            if (map_it == mutable_map.end())
            {
                return end();
            }
            return const_iterator(map_it);
        }

        // -- Iterators --

        /// @brief Returns an iterator to the first element
        [[nodiscard]] iterator begin() { return iterator(map_.begin()); }

        /// @brief Returns a const iterator to the first element
        [[nodiscard]] const_iterator begin() const
        {
            auto& mutable_map = const_cast<HashSet*>(this)->map_;
            return const_iterator(mutable_map.begin());
        }

        /// @brief Returns a const iterator to the first element
        [[nodiscard]] const_iterator cbegin() const { return begin(); }

        /// @brief Returns an iterator to the element following the last element
        [[nodiscard]] iterator end() { return iterator(map_.end()); }

        /// @brief Returns a const iterator to the element following the last element
        [[nodiscard]] const_iterator end() const
        {
            auto& mutable_map = const_cast<HashSet*>(this)->map_;
            return const_iterator(mutable_map.end());
        }

        /// @brief Returns a const iterator to the element following the last element
        [[nodiscard]] const_iterator cend() const { return end(); }

        // -- Capacity --

        /// @brief Get the number of elements in the set
        /// @return The number of elements
        [[nodiscard]] size_type size() const noexcept { return map_.size(); }

        /// @brief Check if the set is empty
        /// @return True if the set is empty, false otherwise
        [[nodiscard]] bool empty() const noexcept { return map_.empty(); }

        // -- Modifiers --

        /// @brief Remove all elements from the set
        void clear() noexcept { map_.clear(); }

        /// @brief Swap the contents of this set with another
        /// @param other The set to swap with
        void swap(HashSet& other) noexcept { map_.swap(other.map_); }

        /// @brief Reserve space for at least the given number of elements
        /// @param count The number of elements to reserve space for
        void reserve(size_type count) { map_.reserve(count); }

    private:
        MapType map_{};
    };
}
