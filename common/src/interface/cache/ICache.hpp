/**
 * @file cache/ICache.hpp
 * @brief ICache class declaration
 * @details This header defines the ICache class that provides functionality for Common interface definitions for the framework.
 */

#pragma once
#include <cstddef>
#include <optional>

namespace common::interface::cache
{
    /// @brief Abstract interface for cache implementations
    /// @tparam Key Type of the key used to identify cache entries
    /// @tparam Value Type of the value stored in the cache
    template <typename Key, typename Value>
    class ICache
    {
    public:
        virtual ~ICache() = default;

        /// @brief Retrieves a value from the cache
        /// @param key The key to look up in the cache
        /// @return Optional value if found, std::nullopt otherwise
        [[nodiscard]] virtual std::optional<Value> get(const Key& key) = 0;

        /// @brief Inserts or updates a key-value pair in the cache
        /// @param key The key to insert or update
        /// @param value The value to store
        /// @return true if the operation was successful, false otherwise
        [[nodiscard]] virtual bool put(const Key& key, const Value& value) = 0;

        /// @brief Inserts or updates a key-value pair in the cache with rvalue reference
        /// @param key The key to insert or update
        /// @param value The value to store (will be moved)
        /// @return true if the operation was successful, false otherwise
        [[nodiscard]] virtual bool put(const Key& key, Value&& value) = 0;

        /// @brief Removes an entry from the cache
        /// @param key The key to remove
        /// @return true if the key was found and removed, false otherwise
        [[nodiscard]] virtual bool remove(const Key& key) = 0;

        /// @brief Clears all entries from the cache
        virtual void clear()  = 0;

        /// @brief Returns the current number of entries in the cache
        /// @return Number of entries currently in the cache
        [[nodiscard]] virtual size_t size() const  = 0;

        /// @brief Returns the maximum capacity of the cache
        /// @return Maximum number of entries the cache can hold
        [[nodiscard]] virtual size_t capacity() const  = 0;

        /// @brief Checks if the cache is empty
        /// @return true if the cache is empty, false otherwise
        [[nodiscard]] virtual bool empty() const  = 0;

        /// @brief Checks if a key exists in the cache
        /// @param key The key to check for
        /// @return true if the key exists in the cache, false otherwise
        [[nodiscard]] virtual bool contains(const Key& key) const  = 0;
    };
}
