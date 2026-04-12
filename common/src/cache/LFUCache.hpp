#pragma once
#include <list>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <fmt/format.h>

#include "interface/ICache.hpp"

namespace common::cache {
    /// @brief Template class implementing an LFU (Least Frequently Used) cache
    /// @tparam Key Type of the key used to identify cache entries
    /// @tparam Value Type of the value stored in the cache
    /// @tparam Map Type of the map used internally to store key-iterator mappings
    template<typename Key, typename Value, typename Map = std::unordered_map<Key, typename std::list<std::pair<Key, std::pair<Value, size_t> > >::iterator> >
    class LFUCache : public interfaces::ICache<Key, Value> {
    public:
        /// @brief Constructs an LFU cache with the specified capacity
        /// @param capacity The maximum number of entries the cache can hold
        /// @throw std::invalid_argument if capacity is 0 or less
        explicit LFUCache(int capacity);

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
        // Structure to hold frequency to list mapping
        std::unordered_map<size_t, std::list<std::pair<Key, std::pair<Value, size_t> > > > freq_list_map_;
        // Key map: maps keys to iterators pointing to their positions in the frequency lists
        Map key_map_;
        size_t capacity_;

        // Minimum frequency in the cache
        mutable size_t min_freq_;

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

        /// @brief Updates the frequency of a key after access
        /// @param it Iterator to the element in the cache
        auto update_frequency(std::list<std::pair<Key, std::pair<Value, size_t> > >::iterator it) -> void;

        /// @brief Evicts the least frequently used item when cache is at capacity
        /// @return true if eviction was successful, false otherwise
        auto evict_lfu_item() -> bool;

        /// @brief Gets or creates a frequency list for the given frequency
        /// @param freq The frequency level
        /// @return Reference to the list for the given frequency
        auto get_or_create_freq_list(size_t freq) -> std::list<std::pair<Key, std::pair<Value, size_t> > > &;

        /// @brief Removes an empty frequency list and updates min_freq_ if necessary
        /// @param freq The frequency level to potentially remove
        auto remove_empty_freq_list(size_t freq) -> void;
    };

    template<typename Key, typename Value, typename Map>
    LFUCache<Key, Value, Map>::LFUCache(const int capacity) : capacity_(static_cast<size_t>(capacity)), min_freq_(0) {
        if (capacity <= 0) {
            throw std::invalid_argument(fmt::format("LFUCache::LFUCache: Cache capacity must be greater than 0, got {}", capacity));
        }
    }

    template<typename Key, typename Value, typename Map>
    template<typename CacheType>
    auto LFUCache<Key, Value, Map>::get_impl(CacheType &cache, const Key &key) -> std::optional<Value> {
        auto it = cache.key_map_.find(key);
        if (it == cache.key_map_.end()) {
            return std::nullopt;
        }

        // Extract the iterator to the actual cache entry
        auto cache_entry_it = it->second;
        Value value = cache_entry_it->second.first; // Store value to return

        // Update frequency of accessed item (only for non-const version)
        if constexpr (!std::is_const_v<CacheType>) {
            cache.update_frequency(cache_entry_it);
        }

        return value;
    }

    template<typename Key, typename Value, typename Map>
    auto LFUCache<Key, Value, Map>::get(const Key &key) const -> std::optional<Value> {
        return get_impl(*this, key);
    }

    template<typename Key, typename Value, typename Map>
    auto LFUCache<Key, Value, Map>::get(const Key &key) -> std::optional<Value> {
        return get_impl(*this, key);
    }

    template<typename Key, typename Value, typename Map>
    template<typename ValueType>
    auto LFUCache<Key, Value, Map>::put_impl(const Key &key, ValueType &&value) -> bool {
        auto it = key_map_.find(key);
        if (it != key_map_.end()) {
            // Key exists, update the value and increment frequency
            auto cache_entry_it = it->second;
            cache_entry_it->second.first = std::forward<ValueType>(value);
            update_frequency(cache_entry_it);
            return true;
        }

        // Key doesn't exist, need to insert new key
        if (key_map_.size() >= capacity_) {
            if (capacity_ == 0) return false;

            if (!evict_lfu_item()) {
                return false; // Failed to evict an item
            }
        }

        // Add new key-value pair with frequency 1
        auto &freq_list = get_or_create_freq_list(1);
        freq_list.emplace_front(key, std::make_pair(std::forward<ValueType>(value), 1));

        // Update key map
        key_map_[key] = freq_list.begin();

        // Since we added a new item with frequency 1, update min_freq_
        min_freq_ = 1;

        return true;
    }

    template<typename Key, typename Value, typename Map>
    auto LFUCache<Key, Value, Map>::put(const Key &key, const Value &value) -> bool {
        return put_impl(key, value);
    }

    template<typename Key, typename Value, typename Map>
    auto LFUCache<Key, Value, Map>::put(const Key &key, Value &&value) -> bool {
        return put_impl(key, std::forward<Value>(value));
    }

    template<typename Key, typename Value, typename Map>
    [[nodiscard]] auto LFUCache<Key, Value, Map>::remove(const Key &key) -> bool {
        auto it = key_map_.find(key);
        if (it == key_map_.end()) {
            return false;
        }

        auto cache_entry_it = it->second;
        size_t freq = cache_entry_it->second.second;

        // Remove from frequency map
        auto &freq_list = freq_list_map_[freq];
        freq_list.erase(cache_entry_it);

        // If the frequency list becomes empty, remove the frequency entry
        remove_empty_freq_list(freq);

        // Remove from key map
        key_map_.erase(it);
        return true;
    }

    template<typename Key, typename Value, typename Map>
    void LFUCache<Key, Value, Map>::clear() noexcept {
        freq_list_map_.clear();
        key_map_.clear();
        min_freq_ = 0;
    }

    template<typename Key, typename Value, typename Map>
    [[nodiscard]] auto LFUCache<Key, Value, Map>::size() const noexcept -> size_t {
        return key_map_.size(); // Size is the number of entries in the key map
    }

    template<typename Key, typename Value, typename Map>
    [[nodiscard]] auto LFUCache<Key, Value, Map>::capacity() const noexcept -> size_t {
        return capacity_;
    }

    template<typename Key, typename Value, typename Map>
    [[nodiscard]] auto LFUCache<Key, Value, Map>::empty() const noexcept -> bool {
        return key_map_.empty();
    }

    template<typename Key, typename Value, typename Map>
    [[nodiscard]] auto LFUCache<Key, Value, Map>::contains(const Key &key) const noexcept -> bool {
        return key_map_.find(key) != key_map_.end();
    }

    template<typename Key, typename Value, typename Map>
    auto LFUCache<Key, Value, Map>::evict_lfu_item() -> bool {
        // Find the list with minimum frequency
        auto min_freq_it = freq_list_map_.find(min_freq_);
        if (min_freq_it == freq_list_map_.end()) {
            return false; // Shouldn't happen if capacity > 0 and cache is not empty
        }

        auto &min_freq_list = min_freq_it->second;

        // Remove the last item in the min frequency list (least recently used among those with same frequency)
        auto lfu_it = min_freq_list.end();
        if (lfu_it != min_freq_list.begin()) {
            --lfu_it; // Move to the last element (LRU within this frequency)
        }

        Key key_to_remove = lfu_it->first;

        // Remove from key map
        key_map_.erase(key_to_remove);

        // Remove from frequency map
        min_freq_list.erase(lfu_it);

        // If the list becomes empty after removal, remove the frequency entry too
        remove_empty_freq_list(min_freq_);

        return true;
    }

    template<typename Key, typename Value, typename Map>
    auto LFUCache<Key, Value, Map>::get_or_create_freq_list(size_t freq) -> std::list<std::pair<Key, std::pair<Value, size_t> > > & {
        if (!freq_list_map_.contains(freq)) {
            freq_list_map_[freq] = std::list<std::pair<Key, std::pair<Value, size_t> > >();
        }
        return freq_list_map_[freq];
    }

    template<typename Key, typename Value, typename Map>
    auto LFUCache<Key, Value, Map>::remove_empty_freq_list(size_t freq) -> void {
        auto &freq_list = freq_list_map_[freq];
        if (freq_list.empty()) {
            freq_list_map_.erase(freq);

            // If we removed the minimum frequency, update min_freq_
            if (freq == min_freq_) {
                // Find the next minimum frequency
                if (!freq_list_map_.empty()) {
                    min_freq_ = freq_list_map_.begin()->first;
                } else {
                    min_freq_ = 0;
                }
            }
        }
    }

    template<typename Key, typename Value, typename Map>
    auto LFUCache<Key, Value, Map>::update_frequency(typename std::list<std::pair<Key, std::pair<Value, size_t> > >::iterator it) -> void {
        Key key = it->first;
        Value value = it->second.first;
        size_t old_freq = it->second.second;
        size_t new_freq = old_freq + 1;

        // Get the old frequency list and remove the element
        auto &old_freq_list = freq_list_map_[old_freq];
        old_freq_list.erase(it);

        // If the old frequency list becomes empty, remove the frequency entry
        remove_empty_freq_list(old_freq);

        // Add to new frequency list
        auto &new_freq_list = get_or_create_freq_list(new_freq);
        new_freq_list.push_front({key, {value, new_freq}});

        // Update key map to point to the new location
        key_map_[key] = new_freq_list.begin();

        // Update min frequency if necessary
        if (min_freq_ == 0 || new_freq < min_freq_) {
            min_freq_ = new_freq;
        }
    }
}
