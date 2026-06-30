/**
 * @file MultiLevelCache.hpp
 * @brief Multi-level cache with L1 (in-memory LRU) and optional L2 backend
 * @details Provides a two-tier caching strategy. L1 is an in-memory LRU cache.
 *          L2 is an optional remote backend (interface only). On L1 miss, L2 is
 *          checked; on L2 hit, the entry is promoted back to L1.
 *
 * @par Thread Safety
 * This class is thread-safe. All operations are protected by a mutex.
 */

#pragma once
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include <cppforge/cache/LRUCache.hpp>

namespace cppforge::starter::cache
{
    /// @brief Abstract interface for L2 cache backends
    /// @tparam K Key type
    /// @tparam V Value type
    template <typename K, typename V>
    class IL2CacheBackend
    {
    public:
        virtual ~IL2CacheBackend() = default;

        /// @brief Retrieve a value from the L2 backend
        /// @param key The key to look up
        /// @return Optional value if found
        [[nodiscard]] virtual std::optional<V> get(const K& key) = 0;

        /// @brief Store a value in the L2 backend
        /// @param key The key to store
        /// @param value The value to store
        /// @return true if successful
        [[nodiscard]] virtual bool put(const K& key, const V& value) = 0;

        /// @brief Remove a value from the L2 backend
        /// @param key The key to remove
        /// @return true if the key was found and removed
        [[nodiscard]] virtual bool remove(const K& key) = 0;

        /// @brief Clear all entries from the L2 backend
        virtual void clear() = 0;
    };

    /// @brief Multi-level cache with L1 (in-memory LRU) and optional L2 backend
    /// @tparam K Key type
    /// @tparam V Value type
    template <typename K, typename V>
    class MultiLevelCache
    {
    public:
        /// @brief Construct a multi-level cache with L1 capacity
        /// @param l1_capacity Maximum number of entries in L1 cache
        explicit MultiLevelCache(std::size_t l1_capacity);

        /// @brief Construct a multi-level cache with L1 capacity and L2 backend
        /// @param l1_capacity Maximum number of entries in L1 cache
        /// @param l2_backend Shared pointer to the L2 cache backend
        MultiLevelCache(std::size_t l1_capacity, std::shared_ptr<IL2CacheBackend<K, V>> l2_backend);

        /// @brief Retrieve a value from the cache
        /// @param key The key to look up
        /// @return Optional value if found in L1 or L2
        [[nodiscard]] std::optional<V> get(const K& key);

        /// @brief Store a value in the cache
        /// @param key The key to store
        /// @param value The value to store
        /// @return true if successful
        [[nodiscard]] bool put(const K& key, const V& value);

        /// @brief Remove a value from the cache
        /// @param key The key to remove
        /// @return true if the key was found and removed
        [[nodiscard]] bool remove(const K& key);

        /// @brief Clear all entries from both cache levels
        void clear();

        /// @brief Returns the current number of entries in L1
        /// @return Number of entries in L1 cache
        [[nodiscard]] std::size_t size() const;

        /// @brief Check if L2 backend is configured
        /// @return true if L2 backend is available
        [[nodiscard]] bool hasL2() const;

    private:
        mutable std::mutex mutex_;
        cppforge::cache::LRUCache<K, V> l1_cache_;
        std::shared_ptr<IL2CacheBackend<K, V>> l2_backend_;
    };

    // -- Template implementations ------------------------------------------------

    template <typename K, typename V>
    MultiLevelCache<K, V>::MultiLevelCache(std::size_t l1_capacity)
        : l1_cache_(l1_capacity), l2_backend_(nullptr)
    {
    }

    template <typename K, typename V>
    MultiLevelCache<K, V>::MultiLevelCache(std::size_t l1_capacity, std::shared_ptr<IL2CacheBackend<K, V>> l2_backend)
        : l1_cache_(l1_capacity), l2_backend_(std::move(l2_backend))
    {
    }

    template <typename K, typename V>
    std::optional<V> MultiLevelCache<K, V>::get(const K& key)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto l1_result = l1_cache_.get(key);
        if (l1_result.has_value())
        {
            return l1_result;
        }

        if (l2_backend_)
        {
            auto l2_result = l2_backend_->get(key);
            if (l2_result.has_value())
            {
                l1_cache_.put(key, l2_result.value());
                return l2_result;
            }
        }

        return std::nullopt;
    }

    template <typename K, typename V>
    bool MultiLevelCache<K, V>::put(const K& key, const V& value)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        bool l1_ok = l1_cache_.put(key, value);

        if (l2_backend_)
        {
            l2_backend_->put(key, value);
        }

        return l1_ok;
    }

    template <typename K, typename V>
    bool MultiLevelCache<K, V>::remove(const K& key)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        bool l1_removed = l1_cache_.remove(key);

        if (l2_backend_)
        {
            l2_backend_->remove(key);
        }

        return l1_removed;
    }

    template <typename K, typename V>
    void MultiLevelCache<K, V>::clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        l1_cache_.clear();

        if (l2_backend_)
        {
            l2_backend_->clear();
        }
    }

    template <typename K, typename V>
    std::size_t MultiLevelCache<K, V>::size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return l1_cache_.size();
    }

    template <typename K, typename V>
    bool MultiLevelCache<K, V>::hasL2() const
    {
        return l2_backend_ != nullptr;
    }
}
