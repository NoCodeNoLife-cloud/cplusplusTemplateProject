#pragma once
#include <glog/logging.h>
#include <fmt/format.h>
#include <list>
#include <optional>
#include <stdexcept>
#include <unordered_map>

#include "src/interface/ICache.hpp"

namespace common::cache {
    /// @brief Template class implementing an LRU (Least Recently Used) cache
    /// @tparam Key Type of the key used to identify cache entries
    /// @tparam Value Type of the value stored in the cache
    /// @tparam Map Type of the map used internally to store key-iterator mappings
    template<typename Key, typename Value, typename Map = std::unordered_map<Key, typename std::list<std::pair<Key, Value> >::iterator> >
    class LRUCache : public interfaces::ICache<Key, Value> {
    public:
        /// @brief Constructs an LRU cache with the specified capacity
        /// @param capacity The maximum number of entries the cache can hold
        /// @throw std::invalid_argument if capacity is 0 or less
        explicit LRUCache(int capacity);

        /// @brief Retrieves a value from the cache (const version)
        /// @param key The key to look up in the cache
        /// @return Optional value if found, std::nullopt otherwise
        [[nodiscard]] auto get(const Key &key) const -> std::optional<Value>;

        /// @brief Retrieves a value from the cache (non-const version)
        /// @param key The key to look up in the cache
        /// @return Optional value if found, std::nullopt otherwise
        [[nodiscard]] auto get(const Key &key) -> std::optional<Value> override;

        /// @brief Inserts or updates a key-value pair in the cache (const value)
        /// @param key The key to insert or update
        /// @param value The value to store
        /// @return true if the operation was successful, false otherwise
        [[nodiscard]] auto put(const Key &key, const Value &value) -> bool override;

        /// @brief Inserts or updates a key-value pair in the cache (rvalue reference)
        /// @param key The key to insert or update
        /// @param value The value to store (will be moved)
        /// @return true if the operation was successful, false otherwise
        [[nodiscard]] auto put(const Key &key, Value &&value) -> bool override;

        /// @brief Removes an entry from the cache
        /// @param key The key to remove
        /// @return true if the key was found and removed, false otherwise
        [[nodiscard]] auto remove(const Key &key) -> bool override;

        /// @brief Clears all entries from the cache
        void clear() noexcept override;

        /// @brief Returns the current number of entries in the cache
        /// @return Number of entries currently in the cache
        [[nodiscard]] auto size() const noexcept -> size_t override;

        /// @brief Returns the maximum capacity of the cache
        /// @return Maximum number of entries the cache can hold
        [[nodiscard]] auto capacity() const noexcept -> size_t override;

        /// @brief Checks if the cache is empty
        /// @return true if the cache is empty, false otherwise
        [[nodiscard]] auto empty() const noexcept -> bool override;

        /// @brief Checks if a key exists in the cache
        /// @param key The key to check for
        /// @return true if the key exists in the cache, false otherwise
        [[nodiscard]] auto contains(const Key &key) const noexcept -> bool override;

    private:
        mutable std::list<std::pair<Key, Value> > cache_list_;
        Map cache_map_;
        size_t capacity_;

        /// @brief Moves the specified iterator to the front of the list (most recently used)
        /// @param it Iterator to the element to move to the front
        auto move_to_front(std::list<std::pair<Key, Value> >::const_iterator it) const -> void;

        /// @brief Helper method to handle both const and non-const get operations
        /// @tparam CacheType Type of the cache instance (const or non-const)
        /// @param cache Reference to the cache instance
        /// @param key The key to look up
        /// @return Optional value if found, std::nullopt otherwise
        template<typename CacheType>
        [[nodiscard]] static auto get_impl(CacheType &cache, const Key &key) -> std::optional<Value>;

        /// @brief Helper method to handle both const and non-const put operations
        /// @tparam ValueType Type of the value to store (const reference or rvalue reference)
        /// @param key The key to insert or update
        /// @param value The value to store
        /// @return true if the operation was successful, false otherwise
        template<typename ValueType>
        [[nodiscard]] auto put_impl(const Key &key, ValueType &&value) -> bool;
    };

    template<typename Key, typename Value, typename Map>
    LRUCache<Key, Value, Map>::LRUCache(const int capacity) : capacity_(static_cast<size_t>(capacity)) {
        if (capacity <= 0) {
            throw std::invalid_argument(fmt::format("Cache capacity must be greater than 0, got {}", capacity));
        }
    }

    template<typename Key, typename Value, typename Map>
    template<typename CacheType>
    auto LRUCache<Key, Value, Map>::get_impl(CacheType &cache, const Key &key) -> std::optional<Value> {
        auto it = cache.cache_map_.find(key);
        if (it == cache.cache_map_.end()) {
            return std::nullopt;
        }

        cache.move_to_front(it->second);
        return it->second->second;
    }

    template<typename Key, typename Value, typename Map>
    auto LRUCache<Key, Value, Map>::get(const Key &key) const -> std::optional<Value> {
        return get_impl(*this, key);
    }

    template<typename Key, typename Value, typename Map>
    auto LRUCache<Key, Value, Map>::get(const Key &key) -> std::optional<Value> {
        return get_impl(*this, key);
    }

    template<typename Key, typename Value, typename Map>
    template<typename ValueType>
    auto LRUCache<Key, Value, Map>::put_impl(const Key &key, ValueType &&value) -> bool {
        auto it = cache_map_.find(key);
        if (it != cache_map_.end()) {
            it->second->second = std::forward<ValueType>(value);
            move_to_front(it->second);
            return true;
        }

        if (cache_list_.size() >= capacity_) {
            if (capacity_ == 0) {
                return false;
            }

            auto last_it = cache_list_.end();
            --last_it;
            cache_map_.erase(last_it->first);
            cache_list_.pop_back();
        }

        cache_list_.emplace_front(key, std::forward<ValueType>(value));
        cache_map_[key] = cache_list_.begin();
        return true;
    }

    template<typename Key, typename Value, typename Map>
    auto LRUCache<Key, Value, Map>::put(const Key &key, const Value &value) -> bool {
        return put_impl(key, value);
    }

    template<typename Key, typename Value, typename Map>
    auto LRUCache<Key, Value, Map>::put(const Key &key, Value &&value) -> bool {
        return put_impl(key, std::forward<Value>(value));
    }

    template<typename Key, typename Value, typename Map>
    [[nodiscard]] auto LRUCache<Key, Value, Map>::remove(const Key &key) -> bool {
        auto it = cache_map_.find(key);
        if (it == cache_map_.end()) {
            return false;
        }

        cache_list_.erase(it->second);
        cache_map_.erase(it);
        return true;
    }

    template<typename Key, typename Value, typename Map>
    void LRUCache<Key, Value, Map>::clear() noexcept {
        const size_t previous_size = cache_list_.size();
        cache_list_.clear();
        cache_map_.clear();
    }

    template<typename Key, typename Value, typename Map>
    [[nodiscard]] auto LRUCache<Key, Value, Map>::size() const noexcept -> size_t {
        return cache_list_.size();
    }

    template<typename Key, typename Value, typename Map>
    [[nodiscard]] auto LRUCache<Key, Value, Map>::capacity() const noexcept -> size_t {
        return capacity_;
    }

    template<typename Key, typename Value, typename Map>
    [[nodiscard]] auto LRUCache<Key, Value, Map>::empty() const noexcept -> bool {
        return cache_list_.empty();
    }

    template<typename Key, typename Value, typename Map>
    [[nodiscard]] auto LRUCache<Key, Value, Map>::contains(const Key &key) const noexcept -> bool {
        return cache_map_.find(key) != cache_map_.end();
    }

    template<typename Key, typename Value, typename Map>
    auto LRUCache<Key, Value, Map>::move_to_front(typename std::list<std::pair<Key, Value> >::const_iterator it) const -> void {
        if (it == cache_list_.begin()) {
            return;
        }

        // Using splice to move the element to the front maintains list validity
        cache_list_.splice(cache_list_.begin(), cache_list_, it);
    }
}
