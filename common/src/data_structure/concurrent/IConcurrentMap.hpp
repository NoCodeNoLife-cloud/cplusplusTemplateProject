/**
 * @file IConcurrentMap.hpp
 * @brief Abstract interface for a concurrent (thread-safe) hash map
 * @details This header defines the IConcurrentMap interface that provides a
 *          thread-safe associative container contract.  All read and write
 *          operations are safe to call concurrently.  The interface uses the
 *          Non-Virtual Interface (NVI) pattern for for_each to allow template
 *          callables while keeping the virtual dispatch internal.
 *
 * Reference: Inspired by java.util.concurrent.ConcurrentMap (JSR 166).
 */

#pragma once

#include <cstddef>
#include <functional>
#include <optional>

namespace common::data_structure::concurrent
{
    /// @brief Thread-safe associative map interface (key → value).
    ///
    /// Maps keys to values with thread-safe insert, lookup, erase, and
    /// traversal operations.  All public methods are safe to call from
    /// multiple threads concurrently.
    ///
    /// @tparam K Key type.
    /// @tparam V Value type.
    ///
    /// @par Thread Safety
    /// All public methods are thread-safe.  Specific concurrency guarantees
    /// (e.g., shared vs exclusive locking, lock granularity) depend on the
    /// concrete implementation.  See subclass documentation for details.
    ///
    /// @par Usage Example
    /// @code
    /// class MyMap : public IConcurrentMap<int, std::string> { ... };
    /// MyMap map;
    /// map.insert(1, "one");
    /// if (auto val = map.get(1)) {
    ///     // val->empty() ...
    /// }
    /// @endcode
    template <typename K, typename V>
    class IConcurrentMap
    {
    public:
        virtual ~IConcurrentMap() = default;

        // ── Modifiers ──────────────────────────────────────────────────

        /// @brief Inserts a key-value pair if the key does not already exist.
        /// @param key   Key to insert.
        /// @param value Value to associate with the key.
        /// @return true if the pair was inserted, false if the key already
        ///         exists (no change).
        [[nodiscard]] virtual auto insert(K key, V value) -> bool = 0;

        /// @brief Inserts a key-value pair, or assigns the value if the key
        ///        already exists.
        /// @param key   Key to insert or assign.
        /// @param value Value to associate with the key.
        /// @return true if a new entry was inserted, false if an existing
        ///         entry was updated.
        [[nodiscard]] virtual auto insert_or_assign(K key, V value) -> bool = 0;

        /// @brief Erases the entry for the given key.
        /// @param key Key to erase.
        /// @return true if the key existed and was removed, false otherwise.
        [[nodiscard]] virtual auto erase(const K& key) -> bool = 0;

        /// @brief Removes all entries from the map.
        virtual void clear() = 0;

        // ── Lookup ─────────────────────────────────────────────────────

        /// @brief Retrieves the value associated with the given key.
        /// @param key Key to look up.
        /// @return The value if the key exists, or std::nullopt if not found.
        [[nodiscard]] virtual auto get(const K& key) const -> std::optional<V> = 0;

        /// @brief Checks whether a key exists in the map.
        /// @param key Key to check.
        /// @return true if the key is present.
        [[nodiscard]] virtual auto contains(const K& key) const -> bool = 0;

        // ── Capacity ───────────────────────────────────────────────────

        /// @brief Returns the number of entries in the map.
        /// @return Entry count.
        [[nodiscard]] virtual auto size() const -> std::size_t = 0;

        /// @brief Checks whether the map is empty.
        /// @return true if size() == 0.
        [[nodiscard]] virtual auto empty() const -> bool = 0;

        /// @brief Returns the current load factor (approximate).
        /// @return Load factor as a double.
        [[nodiscard]] virtual auto load_factor() const -> double = 0;

        /// @brief Returns the total number of buckets across all segments.
        /// @return Bucket count.
        [[nodiscard]] virtual auto bucket_count() const -> std::size_t = 0;

        // ── Traversal (NVI) ────────────────────────────────────────────

        /// @brief Applies a callable to every key-value pair in the map.
        ///
        /// The order of traversal is unspecified but stable for the lifetime
        /// of the call (no concurrent modifications).
        /// @param func Callable invoked as func(key, value) for each entry.
        template <typename Func>
        void for_each(Func&& func) const
        {
            for_each_impl(std::forward<Func>(func));
        }

    protected:
        /// @brief NVI hook for for_each; subclasses implement the actual
        ///        locked traversal here.
        /// @param func Callable invoked as func(key, value) for each entry.
        virtual void for_each_impl(std::function<void(const K&, const V&)>) const = 0;
    };

} // namespace common::data_structure::concurrent
