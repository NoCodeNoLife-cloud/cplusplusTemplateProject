/**
 * @file LRUCache.hpp
 * @brief LRU (Least Recently Used) cache class declaration
 * @details This header defines the LRUCache class that provides O(1) get and put
 *          operations with automatic eviction of the least recently used item
 *          when capacity is exceeded. Internally uses a hash map for key lookup
 *          and a doubly-linked list to track access order.
 */

#pragma once
#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace common::data_structure::hash
{
    /// @brief A Least Recently Used (LRU) cache with O(1) get/put operations.
    /// @tparam K Key type (must be hashable and equality-comparable).
    /// @tparam V Value type (must be copyable).
    template <typename K, typename V>
    class LRUCache
    {
    public:
        using size_type = std::size_t;

        /// @brief Constructs an LRU cache with the given capacity.
        /// @param capacity Maximum number of entries the cache can hold.
        /// @throws std::invalid_argument If capacity is zero.
        explicit LRUCache(size_type capacity)
            : capacity_(capacity)
        {
            if (capacity == 0)
            {
                throw std::invalid_argument("LRUCache: capacity must be greater than zero");
            }
            init_sentinels();
        }

        /// @brief Copy is disabled (cache ownership semantics).
        LRUCache(const LRUCache&) = delete;
        LRUCache& operator=(const LRUCache&) = delete;

        /// @brief Move constructor.
        LRUCache(LRUCache&& other) noexcept
            : map_(std::move(other.map_))
            , head_(other.head_)
            , tail_(other.tail_)
            , capacity_(other.capacity_)
            , size_(other.size_)
        {
            other.head_ = nullptr;
            other.tail_ = nullptr;
            other.size_ = 0;
        }

        /// @brief Move assignment.
        LRUCache& operator=(LRUCache&& other) noexcept
        {
            if (this != &other)
            {
                clear();
                map_ = std::move(other.map_);
                head_ = other.head_;
                tail_ = other.tail_;
                capacity_ = other.capacity_;
                size_ = other.size_;
                other.head_ = nullptr;
                other.tail_ = nullptr;
                other.size_ = 0;
            }
            return *this;
        }

        /// @brief Destructor.
        ~LRUCache() noexcept
        {
            clear();
            delete head_;
            delete tail_;
        }

        /// @brief Returns the value associated with the key, or std::nullopt if not found.
        ///        On hit, the entry is promoted as most recently used.
        /// @param key The key to look up.
        /// @return Optional value, present if key exists.
        [[nodiscard]] std::optional<V> get(const K& key)
        {
            auto it = map_.find(key);
            if (it == map_.end())
            {
                return std::nullopt;
            }
            move_to_head(it->second.get());
            return it->second->value;
        }

        /// @brief Inserts or updates a key-value pair. Evicts the least recently used
        ///        entry if the cache is at capacity and the key is new.
        /// @param key The key.
        /// @param value The value.
        void put(const K& key, const V& value)
        {
            auto it = map_.find(key);
            if (it != map_.end())
            {
                it->second->value = value;
                move_to_head(it->second.get());
                return;
            }
            if (size_ >= capacity_)
            {
                evict_tail();
            }
            auto node = std::make_unique<Node>(key, value);
            Node* raw = node.get();
            add_to_head(raw);
            map_.emplace(key, std::move(node));
            ++size_;
        }

        /// @brief Inserts or updates a key-value pair using move semantics.
        /// @param key The key.
        /// @param value The value.
        void put(K&& key, V&& value)
        {
            auto it = map_.find(key);
            if (it != map_.end())
            {
                it->second->value = std::move(value);
                move_to_head(it->second.get());
                return;
            }
            if (size_ >= capacity_)
            {
                evict_tail();
            }
            auto node = std::make_unique<Node>(std::move(key), std::move(value));
            Node* raw = node.get();
            add_to_head(raw);
            map_.emplace(raw->key, std::move(node));
            ++size_;
        }

        /// @brief Checks if the cache contains the given key.
        /// @param key The key to check.
        /// @return True if the key exists, false otherwise.
        [[nodiscard]] bool contains(const K& key) const
        {
            return map_.find(key) != map_.end();
        }

        /// @brief Erases the entry with the given key.
        /// @param key The key to erase.
        /// @return True if the entry was erased, false if the key was not found.
        bool erase(const K& key)
        {
            auto it = map_.find(key);
            if (it == map_.end())
            {
                return false;
            }
            remove_node(it->second.get());
            map_.erase(it);
            --size_;
            return true;
        }

        /// @brief Returns the number of entries currently in the cache.
        [[nodiscard]] size_type size() const noexcept
        {
            return size_;
        }

        /// @brief Returns the maximum capacity of the cache.
        [[nodiscard]] size_type capacity() const noexcept
        {
            return capacity_;
        }

        /// @brief Checks if the cache is empty.
        [[nodiscard]] bool empty() const noexcept
        {
            return size_ == 0;
        }

        /// @brief Removes all entries from the cache.
        void clear() noexcept
        {
            map_.clear();
            if (head_ != nullptr)
            {
                head_->next = tail_;
                tail_->prev = head_;
            }
            size_ = 0;
        }

    private:
        struct Node
        {
            K key;
            V value;
            Node* prev = nullptr;
            Node* next = nullptr;

            Node(const K& k, const V& v) : key(k), value(v) {}
            Node(K&& k, V&& v) : key(std::move(k)), value(std::move(v)) {}
        };

        std::unordered_map<K, std::unique_ptr<Node>> map_{};
        Node* head_ = nullptr;
        Node* tail_ = nullptr;
        size_type capacity_ = 0;
        size_type size_ = 0;

        /// @brief Creates sentinel head and tail nodes.
        void init_sentinels()
        {
            head_ = new Node(K{}, V{});
            tail_ = new Node(K{}, V{});
            head_->next = tail_;
            tail_->prev = head_;
        }

        /// @brief Links a node at the head (most recently used position).
        void add_to_head(Node* node)
        {
            node->next = head_->next;
            node->prev = head_;
            head_->next->prev = node;
            head_->next = node;
        }

        /// @brief Unlinks a node from the linked list.
        void remove_node(Node* node)
        {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }

        /// @brief Moves an existing node to the head (MRU position).
        void move_to_head(Node* node)
        {
            remove_node(node);
            add_to_head(node);
        }

        /// @brief Evicts the node at the tail (least recently used).
        void evict_tail()
        {
            Node* lru = tail_->prev;
            if (lru == head_)
            {
                return;
            }
            remove_node(lru);
            map_.erase(lru->key);
            --size_;
        }
    };
}
