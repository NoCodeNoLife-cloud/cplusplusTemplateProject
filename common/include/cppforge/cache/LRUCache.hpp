/**
 * @file LRUCache.hpp
 * @brief Least-Recently-Used (LRU) cache with O(1) get/put and eviction
 * @details A cache that evicts the least recently accessed entry when capacity
 *          is exceeded.  Uses a hash map for O(1) key lookup and a doubly linked
 *          list to track access order.  On every get/put the accessed entry is
 *          moved to the front.  When full, the back (least recently used) entry
 *          is evicted.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Complexity
 * - get / put: O(1) amortised
 *
 * @par Usage Example
 * @code
 * LRUCache<int, std::string> cache(2);
 * cache.put(1, "one");
 * cache.put(2, "two");
 * cache.get(1);
 * cache.put(3, "three"); // evicts key 2
 * @endcode
 */

#pragma once
#include <iterator>
#include <list>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <fmt/format.h>
#include <cppforge/interface/ICache.hpp>

namespace cppforge::cache
{
    /// @brief Template class implementing an LRU (Least Recently Used) cache
    /// @tparam Key Type of the key used to identify cache entries
    /// @tparam Value Type of the value stored in the cache
    /// @tparam Map Type of the map used internally to store key-iterator mappings
    template <typename Key, typename Value, typename Map = std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator>>
    class LRUCache : public interface::cache::ICache<Key, Value>
    {
    public:
        /// @brief Constructs an LRU cache with the specified capacity
        /// @param capacity The maximum number of entries the cache can hold
        /// @throw std::invalid_argument if capacity is 0 or less
        explicit LRUCache(size_t capacity);

        /// @brief Retrieves a value from the cache (const version)
        /// @param key The key to look up in the cache
        /// @return Optional value if found, std::nullopt otherwise
        [[nodiscard]] std::optional<Value> get(const Key& key) const;

        /// @brief Retrieves a value from the cache (non-const version)
        /// @param key The key to look up in the cache
        /// @return Optional value if found, std::nullopt otherwise
        [[nodiscard]] std::optional<Value> get(const Key& key) override;

        /// @brief Inserts or updates a key-value pair in the cache (const value)
        /// @param key The key to insert or update
        /// @param value The value to store
        /// @return true if the operation was successful, false otherwise
        /// @throws std::invalid_argument If key is invalid for the container type
        [[nodiscard]] bool put(const Key& key, const Value& value) override;

        /// @brief Inserts or updates a key-value pair in the cache (rvalue reference)
        /// @param key The key to insert or update
        /// @param value The value to store (will be moved)
        /// @return true if the operation was successful, false otherwise
        /// @throws std::invalid_argument If key is invalid for the container type
        [[nodiscard]] bool put(const Key& key, Value&& value) override;

        /// @brief Removes an entry from the cache
        /// @param key The key to remove
        /// @return true if the key was found and removed, false otherwise
        [[nodiscard]] bool remove(const Key& key) noexcept override;

        /// @brief Clears all entries from the cache
        void clear()  noexcept override;

        /// @brief Returns the current number of entries in the cache
        /// @return Number of entries currently in the cache
        [[nodiscard]] size_t size() const noexcept override;

        /// @brief Returns the maximum capacity of the cache
        /// @return Maximum number of entries the cache can hold
        [[nodiscard]] size_t capacity() const noexcept override;

        /// @brief Checks if the cache is empty
        /// @return true if the cache is empty, false otherwise
        [[nodiscard]] bool empty() const noexcept override;

        /// @brief Checks if a key exists in the cache
        /// @param key The key to check for
        /// @return true if the key exists in the cache, false otherwise
        [[nodiscard]] bool contains(const Key& key) const noexcept override;

    private:
        mutable std::list<std::pair<Key, Value>> cache_list_;
        Map cache_map_;
        size_t capacity_;

        /// @brief Moves the specified iterator to the front of the list (most recently used)
        /// @param it Iterator to the element to move to the front
        void move_to_front(std::list<std::pair<Key, Value>>::const_iterator it) const noexcept;

        /// @brief Helper method to handle both const and non-const get operations
        /// @tparam CacheType Type of the cache instance (const or non-const)
        /// @param cache Reference to the cache instance
        /// @param key The key to look up
        /// @return Optional value if found, std::nullopt otherwise
        template <typename CacheType>
        [[nodiscard]] static std::optional<Value> get_impl(CacheType& cache, const Key& key);

        /// @brief Helper method to handle both const and non-const put operations
        /// @tparam ValueType Type of the value to store (const reference or rvalue reference)
        /// @param key The key to insert or update
        /// @param value The value to store
        /// @return true if the operation was successful, false otherwise
        template <typename ValueType>
        [[nodiscard]] bool put_impl(const Key& key, ValueType&& value);
    };

    template <typename Key, typename Value, typename Map>
    LRUCache<Key, Value, Map>::LRUCache(size_t capacity) : capacity_(capacity)
    {
        if (capacity == 0)
        {
            throw std::invalid_argument(fmt::format("Cache capacity must be greater than 0, got {}", capacity));
        }
    }

    template <typename Key, typename Value, typename Map>
    std::optional<Value> LRUCache<Key, Value, Map>::get(const Key& key) const
    {
        return get_impl(*this, key);
    }

    template <typename Key, typename Value, typename Map>
    std::optional<Value> LRUCache<Key, Value, Map>::get(const Key& key)
    {
        return get_impl(*this, key);
    }

    template <typename Key, typename Value, typename Map>
    bool LRUCache<Key, Value, Map>::put(const Key& key, const Value& value)
    {
        return put_impl(key, value);
    }

    template <typename Key, typename Value, typename Map>
    bool LRUCache<Key, Value, Map>::put(const Key& key, Value&& value)
    {
        return put_impl(key, std::forward<Value>(value));
    }

    template <typename Key, typename Value, typename Map>
    bool LRUCache<Key, Value, Map>::remove(const Key& key) noexcept
    {
        auto it = cache_map_.find(key);
        if (it == cache_map_.end())
        {
            return false;
        }

        cache_list_.erase(it->second);
        cache_map_.erase(it);
        return true;
    }

    template <typename Key, typename Value, typename Map>
    void LRUCache<Key, Value, Map>::clear() noexcept
    {
        cache_list_.clear();
        cache_map_.clear();
    }

    template <typename Key, typename Value, typename Map>
    size_t LRUCache<Key, Value, Map>::size() const noexcept
    {
        return cache_list_.size();
    }

    template <typename Key, typename Value, typename Map>
    size_t LRUCache<Key, Value, Map>::capacity() const noexcept
    {
        return capacity_;
    }

    template <typename Key, typename Value, typename Map>
    bool LRUCache<Key, Value, Map>::empty() const noexcept
    {
        return cache_list_.empty();
    }

    template <typename Key, typename Value, typename Map>
    bool LRUCache<Key, Value, Map>::contains(const Key& key) const noexcept
    {
        return cache_map_.find(key) != cache_map_.end();
    }

    template <typename Key, typename Value, typename Map>
    void LRUCache<Key, Value, Map>::move_to_front(typename std::list<std::pair<Key, Value>>::const_iterator it) const noexcept
    {
        if (it == cache_list_.begin())
        {
            return;
        }

        // Using splice to move the element to the front maintains list validity
        cache_list_.splice(cache_list_.begin(), cache_list_, it);
    }

    template <typename Key, typename Value, typename Map>
    template <typename CacheType>
    std::optional<Value> LRUCache<Key, Value, Map>::get_impl(CacheType& cache, const Key& key)
    {
        auto it = cache.cache_map_.find(key);
        if (it == cache.cache_map_.end())
        {
            return std::nullopt;
        }

        cache.move_to_front(it->second);
        return it->second->second;
    }

    template <typename Key, typename Value, typename Map>
    template <typename ValueType>
    bool LRUCache<Key, Value, Map>::put_impl(const Key& key, ValueType&& value)
    {
        auto it = cache_map_.find(key);
        if (it != cache_map_.end())
        {
            it->second->second = std::forward<ValueType>(value);
            move_to_front(it->second);
            return true;
        }

        if (cache_list_.size() >= capacity_)
        {
            auto last_it = std::prev(cache_list_.end());
            cache_map_.erase(last_it->first);
            cache_list_.pop_back();
        }

        cache_list_.emplace_front(key, std::forward<ValueType>(value));
        cache_map_[key] = cache_list_.begin();
        return true;
    }
}