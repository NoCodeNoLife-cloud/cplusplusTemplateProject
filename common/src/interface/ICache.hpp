#pragma once
#include <optional>

namespace common::interfaces {
    /// @brief Abstract interface for cache implementations
    /// @tparam Key Type of the key used to identify cache entries
    /// @tparam Value Type of the value stored in the cache
    template<typename Key, typename Value>
    class ICache {
    public:
        virtual ~ICache() = default;

        /// @brief Retrieves a value from the cache
        /// @param key The key to look up in the cache
        /// @return Optional value if found, std::nullopt otherwise
        [[nodiscard]] virtual auto get(const Key &key) -> std::optional<Value> = 0;

        /// @brief Inserts or updates a key-value pair in the cache
        /// @param key The key to insert or update
        /// @param value The value to store
        /// @return true if the operation was successful, false otherwise
        [[nodiscard]] virtual auto put(const Key &key, const Value &value) -> bool = 0;

        /// @brief Inserts or updates a key-value pair in the cache with rvalue reference
        /// @param key The key to insert or update
        /// @param value The value to store (will be moved)
        /// @return true if the operation was successful, false otherwise
        [[nodiscard]] virtual auto put(const Key &key, Value &&value) -> bool = 0;

        /// @brief Removes an entry from the cache
        /// @param key The key to remove
        /// @return true if the key was found and removed, false otherwise
        [[nodiscard]] virtual auto remove(const Key &key) -> bool = 0;

        /// @brief Clears all entries from the cache
        virtual void clear() noexcept = 0;

        /// @brief Returns the current number of entries in the cache
        /// @return Number of entries currently in the cache
        [[nodiscard]] virtual auto size() const noexcept -> size_t = 0;

        /// @brief Returns the maximum capacity of the cache
        /// @return Maximum number of entries the cache can hold
        [[nodiscard]] virtual auto capacity() const noexcept -> size_t = 0;

        /// @brief Checks if the cache is empty
        /// @return true if the cache is empty, false otherwise
        [[nodiscard]] virtual auto empty() const noexcept -> bool = 0;

        /// @brief Checks if a key exists in the cache
        /// @param key The key to check for
        /// @return true if the key exists in the cache, false otherwise
        [[nodiscard]] virtual auto contains(const Key &key) const noexcept -> bool = 0;
    };
}
