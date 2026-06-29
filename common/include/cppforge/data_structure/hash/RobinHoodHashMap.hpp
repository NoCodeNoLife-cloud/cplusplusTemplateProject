/**
 * @file RobinHoodHashMap.hpp
 * @brief Open-addressing hash map using Robin Hood hashing with backward-shift deletion
 * @details An open-addressing hash map using Robin Hood hashing with linear
 *          probing.  On collision, the entry with larger displacement (farther
 *          from its ideal bucket) steals the slot from the entry with smaller
 *          displacement.  This keeps probe lengths uniformly low and provides
 *          excellent cache locality.  Backward-shift deletion avoids tombstone
 *          accumulation without requiring a full rehash.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Complexity
 * - insert / find / erase: O(1) average, O(n) worst-case
 * - Memory overhead:       ~1 byte per bucket (metadata) + key + value
 *
 * @par Usage Example
 * @code
 * RobinHoodHashMap<std::string, int> map;
 * map.insert("alice", 100);
 * map.insert("bob", 200);
 * assert(*map.find("alice") == 100);
 * @endcode
 */

#pragma once
#include <cstdint>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace cppforge::data_structure::hash
{
    /// @brief A hash map using open addressing with Robin Hood hashing.
    /// @tparam K Key type (must be copyable).
    /// @tparam V Value type (must be copyable).
    /// @tparam Hasher Hash function object type, defaults to std::hash<K>.
    /// @tparam KeyEqual Equality comparison object type, defaults to std::equal_to<K>.
    template <typename K, typename V, typename Hasher = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RobinHoodHashMap
    {
    public:
        using size_type = std::size_t;

    private:
        static constexpr size_type INITIAL_CAPACITY = 8;
        static constexpr size_type NOT_FOUND = static_cast<size_type>(-1);

        struct Slot
        {
            enum State : uint8_t
            {
                EMPTY,
                OCCUPIED,
                TOMBSTONE
            };

            State state = EMPTY;
            int32_t psr = 0;    /* probe-sequence residual: distance from ideal bucket */
            K key{};
            V value{};

            Slot() = default;
            Slot(const K& k, const V& v) : key(k), value(v) {}
            Slot(K&& k, V&& v) : key(std::move(k)), value(std::move(v)) {}
        };

    public:
        /// @brief Forward iterator over occupied slots.
        class iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::pair<const K&, V&>;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type;

            iterator() = default;

            reference operator*() const
            {
                return {slot_->key, slot_->value};
            }

            iterator& operator++()
            {
                if (!slots_end_)
                {
                    return *this;
                }
                for (++slot_; slot_ != slots_end_; ++slot_)
                {
                    if (slot_->state == Slot::OCCUPIED)
                    {
                        return *this;
                    }
                }
                slot_ = nullptr;
                slots_end_ = nullptr;
                return *this;
            }

            iterator operator++(int)
            {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const iterator& other) const { return slot_ == other.slot_; }
            bool operator!=(const iterator& other) const { return slot_ != other.slot_; }

        private:
            friend class RobinHoodHashMap;

            Slot* slot_ = nullptr;
            Slot* slots_end_ = nullptr;
        };

        /// @brief Const forward iterator over occupied slots.
        class const_iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::pair<const K&, const V&>;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type;

            const_iterator() = default;

            reference operator*() const
            {
                return {slot_->key, slot_->value};
            }

            const_iterator& operator++()
            {
                if (!slots_end_)
                {
                    return *this;
                }
                for (++slot_; slot_ != slots_end_; ++slot_)
                {
                    if (slot_->state == Slot::OCCUPIED)
                    {
                        return *this;
                    }
                }
                slot_ = nullptr;
                slots_end_ = nullptr;
                return *this;
            }

            const_iterator operator++(int)
            {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const const_iterator& other) const { return slot_ == other.slot_; }
            bool operator!=(const const_iterator& other) const { return slot_ != other.slot_; }

        private:
            friend class RobinHoodHashMap;

            const Slot* slot_ = nullptr;
            const Slot* slots_end_ = nullptr;
        };

        /// @brief Default constructor.
        RobinHoodHashMap() noexcept
        {
            reserve(INITIAL_CAPACITY);
        }

        /// @brief Constructs with a given capacity.
        /// @param capacity Minimum capacity to reserve.
        explicit RobinHoodHashMap(size_type capacity)
        {
            reserve(capacity > 0 ? capacity : INITIAL_CAPACITY);
        }

        /// @brief Copy constructor.
        RobinHoodHashMap(const RobinHoodHashMap& other)
            : size_(other.size_)
            , max_load_factor_(other.max_load_factor_)
            , hasher_(other.hasher_)
            , key_equal_(other.key_equal_)
        {
            slots_ = other.slots_;
            capacity_mask_ = other.capacity_mask_;
        }

        /// @brief Move constructor.
        RobinHoodHashMap(RobinHoodHashMap&& other) noexcept
            : slots_(std::move(other.slots_))
            , size_(other.size_)
            , capacity_mask_(other.capacity_mask_)
            , max_load_factor_(other.max_load_factor_)
            , hasher_(std::move(other.hasher_))
            , key_equal_(std::move(other.key_equal_))
        {
            other.size_ = 0;
            other.capacity_mask_ = 0;
        }

        /// @brief Copy assignment.
        RobinHoodHashMap& operator=(const RobinHoodHashMap& other)
        {
            if (this != &other)
            {
                slots_ = other.slots_;
                size_ = other.size_;
                capacity_mask_ = other.capacity_mask_;
                max_load_factor_ = other.max_load_factor_;
                hasher_ = other.hasher_;
                key_equal_ = other.key_equal_;
            }
            return *this;
        }

        /// @brief Move assignment.
        RobinHoodHashMap& operator=(RobinHoodHashMap&& other) noexcept
        {
            if (this != &other)
            {
                slots_ = std::move(other.slots_);
                size_ = other.size_;
                capacity_mask_ = other.capacity_mask_;
                max_load_factor_ = other.max_load_factor_;
                hasher_ = std::move(other.hasher_);
                key_equal_ = std::move(other.key_equal_);
                other.size_ = 0;
                other.capacity_mask_ = 0;
            }
            return *this;
        }

        /// @brief Destructor.
        ~RobinHoodHashMap() noexcept = default;

        /// @brief Returns an iterator to the first occupied element.
        [[nodiscard]] iterator begin()
        {
            return make_iter<iterator>(find_first_occupied());
        }

        /// @brief Returns a const iterator to the first occupied element.
        [[nodiscard]] const_iterator begin() const
        {
            return make_const_iter(find_first_occupied());
        }

        /// @brief Returns a const iterator to the first occupied element.
        [[nodiscard]] const_iterator cbegin() const
        {
            return begin();
        }

        /// @brief Returns an iterator representing the end.
        [[nodiscard]] iterator end()
        {
            return make_iter<iterator>(nullptr);
        }

        /// @brief Returns a const iterator representing the end.
        [[nodiscard]] const_iterator end() const
        {
            return make_const_iter(nullptr);
        }

        /// @brief Returns a const iterator representing the end.
        [[nodiscard]] const_iterator cend() const
        {
            return end();
        }

        /// @brief Inserts a key-value pair. Does nothing if the key already exists.
        /// @param key The key to insert.
        /// @param value The value to insert.
        /// @return A pair of iterator to the element and a bool (true if inserted).
        std::pair<iterator, bool> insert(const K& key, const V& value)
        {
            return insert_impl(key, value);
        }

        /// @brief Inserts a key-value pair using move semantics. Does nothing if the key already exists.
        /// @param key The key to insert.
        /// @param value The value to insert.
        /// @return A pair of iterator to the element and a bool (true if inserted).
        std::pair<iterator, bool> insert(K&& key, V&& value)
        {
            return insert_impl(std::move(key), std::move(value));
        }

        /// @brief Inserts a key-value pair, or assigns the value if the key already exists.
        /// @param key The key to insert.
        /// @param value The value to insert.
        /// @return A pair of iterator to the element and a bool (true if inserted, false if assigned).
        std::pair<iterator, bool> insert_or_assign(const K& key, const V& value)
        {
            return insert_or_assign_impl(key, value);
        }

        /// @brief Inserts a key-value pair with move semantics, or assigns if key exists.
        /// @param key The key to insert.
        /// @param value The value to insert.
        /// @return A pair of iterator to the element and a bool (true if inserted, false if assigned).
        std::pair<iterator, bool> insert_or_assign(K&& key, V&& value)
        {
            return insert_or_assign_impl(std::move(key), std::move(value));
        }

        /// @brief Erases the element with the given key.
        /// @param key The key to erase.
        /// @return True if the element was erased, false if the key was not found.
        bool erase(const K& key)
        {
            const size_type idx = find_slot(key);
            if (idx == NOT_FOUND)
            {
                return false;
            }
            return erase_at(idx);
        }

        /// @brief Erases the element at the given iterator position.
        /// @param it Iterator to the element to erase (must be valid and dereferenceable).
        /// @return Iterator to the next occupied element.
        iterator erase(const_iterator it)
        {
            const size_type idx = static_cast<size_type>(it.slot_ - slots_.data());
            erase_at(idx);
            return make_iter<iterator>(find_first_occupied_from(idx));
        }

        /// @brief Erases the element at the given iterator position.
        /// @param it Iterator to the element to erase (must be valid and dereferenceable).
        /// @return Iterator to the next occupied element.
        iterator erase(iterator it)
        {
            const_iterator cit;
            cit.slot_ = it.slot_;
            cit.slots_end_ = it.slots_end_;
            return erase(cit);
        }

        /// @brief Finds an element by key.
        /// @param key The key to search for.
        /// @return Iterator to the element, or end() if not found.
        [[nodiscard]] iterator find(const K& key)
        {
            const size_type idx = find_slot(key);
            if (idx == NOT_FOUND)
            {
                return end();
            }
            return make_iter<iterator>(slots_.data() + idx);
        }

        /// @brief Finds an element by key (const).
        /// @param key The key to search for.
        /// @return Const iterator to the element, or cend() if not found.
        [[nodiscard]] const_iterator find(const K& key) const
        {
            const size_type idx = find_slot(key);
            if (idx == NOT_FOUND)
            {
                return end();
            }
            return make_const_iter(slots_.data() + idx);
        }

        /// @brief Checks if a key exists in the map.
        /// @param key The key to check.
        /// @return True if the key exists, false otherwise.
        [[nodiscard]] bool contains(const K& key) const
        {
            return find_slot(key) != NOT_FOUND;
        }

        /// @brief Accesses the value associated with the given key. Throws if not found.
        /// @param key The key to look up.
        /// @return Reference to the value.
        /// @throws std::out_of_range If the key is not found.
        [[nodiscard]] V& at(const K& key)
        {
            const size_type idx = find_slot(key);
            if (idx == NOT_FOUND)
            {
                throw std::out_of_range("RobinHoodHashMap::at: key not found");
            }
            return slots_[idx].value;
        }

        /// @brief Accesses the value associated with the given key (const). Throws if not found.
        /// @param key The key to look up.
        /// @return Const reference to the value.
        /// @throws std::out_of_range If the key is not found.
        [[nodiscard]] const V& at(const K& key) const
        {
            const size_type idx = find_slot(key);
            if (idx == NOT_FOUND)
            {
                throw std::out_of_range("RobinHoodHashMap::at: key not found");
            }
            return slots_[idx].value;
        }

        /// @brief Accesses or creates the element with the given key.
        /// @param key The key to look up or insert.
        /// @return Reference to the value.
        V& operator[](const K& key)
        {
            auto [it, _] = insert(key, V{});
            return it.slot_->value;
        }

        /// @brief Accesses or creates the element with the given key using move semantics.
        /// @param key The key to look up or insert.
        /// @return Reference to the value.
        V& operator[](K&& key)
        {
            auto [it, _] = insert(std::move(key), V{});
            return it.slot_->value;
        }

        /// @brief Returns the number of elements in the map.
        [[nodiscard]] size_type size() const noexcept
        {
            return size_;
        }

        /// @brief Checks if the map is empty.
        [[nodiscard]] bool empty() const noexcept
        {
            return size_ == 0;
        }

        /// @brief Returns the capacity (number of buckets).
        [[nodiscard]] size_type bucket_count() const noexcept
        {
            return slots_.size();
        }

        /// @brief Returns the current load factor.
        [[nodiscard]] double load_factor() const noexcept
        {
            const auto sz = slots_.size();
            return sz > 0 ? static_cast<double>(size_) / static_cast<double>(sz) : 0.0;
        }

        /// @brief Returns the maximum load factor.
        [[nodiscard]] double max_load_factor() const noexcept
        {
            return max_load_factor_;
        }

        /// @brief Sets the maximum load factor. Rehashes if current load factor exceeds new value.
        void max_load_factor(double mlf)
        {
            max_load_factor_ = mlf;
            if (load_factor() > max_load_factor_)
            {
                rehash(bucket_count());
            }
        }

        /// @brief Reserves space for at least 'count' elements.
        void reserve(size_type count)
        {
            const auto needed = static_cast<size_type>(static_cast<double>(count) / max_load_factor_);
            rehash(next_power_of_two(needed));
        }

        /// @brief Rehashes to the specified number of buckets.
        void rehash(size_type new_capacity)
        {
            new_capacity = next_power_of_two(new_capacity);
            if (new_capacity < INITIAL_CAPACITY)
            {
                new_capacity = INITIAL_CAPACITY;
            }
            if (new_capacity == bucket_count())
            {
                return;
            }
            std::vector<Slot> old_slots = std::move(slots_);
            slots_ = std::vector<Slot>(new_capacity);
            capacity_mask_ = new_capacity - 1;
            size_ = 0;
            for (auto& slot : old_slots)
            {
                if (slot.state == Slot::OCCUPIED)
                {
                    insert_no_resize(std::move(slot));
                }
            }
        }

        /// @brief Removes all elements from the map.
        void clear() noexcept
        {
            for (auto& slot : slots_)
            {
                slot.state = Slot::EMPTY;
                slot.psr = 0;
            }
            size_ = 0;
        }

        /// @brief Swaps the contents with another RobinHoodHashMap.
        void swap(RobinHoodHashMap& other) noexcept
        {
            using std::swap;
            swap(slots_, other.slots_);
            swap(size_, other.size_);
            swap(capacity_mask_, other.capacity_mask_);
            swap(max_load_factor_, other.max_load_factor_);
            swap(hasher_, other.hasher_);
            swap(key_equal_, other.key_equal_);
        }

    private:
        std::vector<Slot> slots_{};
        size_type size_ = 0;
        size_type capacity_mask_ = 0;
        double max_load_factor_ = 0.75;
        Hasher hasher_{};
        KeyEqual key_equal_{};

        /// @brief Rounds up to the next power of two.
        static size_type next_power_of_two(size_type n) noexcept
        {
            if (n <= 1)
            {
                return 1;
            }
            --n;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
            n |= n >> 32;
            return n + 1;
        }

        /// @brief Returns the ideal bucket index for a key.
        [[nodiscard]] size_type ideal_index(const K& key) const noexcept
        {
            return static_cast<size_type>(hasher_(key)) & capacity_mask_;
        }

        /// @brief Finds the slot index for a key, returns NOT_FOUND if absent.
        [[nodiscard]] size_type find_slot(const K& key) const noexcept
        {
            if (slots_.empty())
            {
                return NOT_FOUND;
            }
            size_type idx = ideal_index(key);
            int32_t dist = 0;
            while (true)
            {
                const auto& slot = slots_[idx];
                if (slot.state == Slot::EMPTY || dist > slot.psr)
                {
                    return NOT_FOUND;
                }
                if (slot.state == Slot::OCCUPIED && key_equal_(slot.key, key))
                {
                    return idx;
                }
                idx = (idx + 1) & capacity_mask_;
                ++dist;
            }
        }

        /// @brief Inserts a slot without triggering rehash (internal helper).
        ///        Only called during rehash or when capacity is guaranteed sufficient.
        void insert_no_resize(Slot&& entry)
        {
            size_type idx = ideal_index(entry.key);
            int32_t psr = 0;
            entry.state = Slot::OCCUPIED;
            entry.psr = 0;

            while (true)
            {
                auto& slot = slots_[idx];
                if (slot.state != Slot::OCCUPIED)
                {
                    slot = std::move(entry);
                    slot.psr = psr;
                    ++size_;
                    return;
                }
                if (psr > slot.psr)
                {
                    std::swap(entry, slot);
                    slot.psr = psr;
                    psr = entry.psr;
                }
                idx = (idx + 1) & capacity_mask_;
                ++psr;
            }
        }

        /// @brief Core insert implementation for lvalue references.
        std::pair<iterator, bool> insert_impl(const K& key, const V& value)
        {
            ensure_capacity();
            return insert_entry(Slot(key, value), key);
        }

        /// @brief Core insert implementation for rvalue references.
        std::pair<iterator, bool> insert_impl(K&& key, V&& value)
        {
            ensure_capacity();
            auto key_copy = key;
            return insert_entry(Slot(std::move(key), std::move(value)), key_copy);
        }

        /// @brief Core insert_or_assign implementation for lvalue references.
        std::pair<iterator, bool> insert_or_assign_impl(const K& key, const V& value)
        {
            const size_type existing = find_slot(key);
            if (existing != NOT_FOUND)
            {
                slots_[existing].value = value;
                return {make_iter<iterator>(slots_.data() + existing), false};
            }
            return insert(key, value);
        }

        /// @brief Core insert_or_assign implementation for rvalue references.
        std::pair<iterator, bool> insert_or_assign_impl(K&& key, V&& value)
        {
            const size_type existing = find_slot(key);
            if (existing != NOT_FOUND)
            {
                slots_[existing].value = std::move(value);
                return {make_iter<iterator>(slots_.data() + existing), false};
            }
            return insert(std::move(key), std::move(value));
        }

        /// @brief Inserts a pre-constructed slot, retrying if key already exists.
        std::pair<iterator, bool> insert_entry(Slot&& entry, const K& key_check)
        {
            size_type idx = ideal_index(entry.key);
            int32_t psr = 0;

            while (true)
            {
                auto& slot = slots_[idx];
                if (slot.state == Slot::OCCUPIED && key_equal_(slot.key, key_check))
                {
                    return {make_iter<iterator>(slots_.data() + idx), false};
                }
                if (slot.state != Slot::OCCUPIED)
                {
                    slot = std::move(entry);
                    slot.state = Slot::OCCUPIED;
                    slot.psr = psr;
                    ++size_;
                    return {make_iter<iterator>(slots_.data() + idx), true};
                }
                if (psr > slot.psr)
                {
                    std::swap(entry, slot);
                    slot.state = Slot::OCCUPIED;
                    slot.psr = psr;
                    psr = entry.psr;
                }
                idx = (idx + 1) & capacity_mask_;
                ++psr;
            }
        }

        /// @brief Erases the slot at the given index using backward-shift deletion.
        bool erase_at(size_type idx)
        {
            --size_;

            /* backward-shift deletion */
            while (true)
            {
                size_type next = (idx + 1) & capacity_mask_;
                if (slots_[next].state != Slot::OCCUPIED || slots_[next].psr <= 0)
                {
                    break;
                }
                slots_[idx] = std::move(slots_[next]);
                slots_[idx].psr--;
                idx = next;
            }
            slots_[idx].state = Slot::EMPTY;
            slots_[idx].psr = 0;
            return true;
        }

        /// @brief Ensures enough capacity for one more insert.
        void ensure_capacity()
        {
            if (slots_.empty() || static_cast<double>(size_ + 1) / static_cast<double>(slots_.size()) > max_load_factor_)
            {
                const auto new_cap = slots_.empty() ? INITIAL_CAPACITY : slots_.size() * 2;
                rehash(new_cap);
            }
        }

        Slot* find_first_occupied()
        {
            if (slots_.empty())
            {
                return nullptr;
            }
            for (auto& slot : slots_)
            {
                if (slot.state == Slot::OCCUPIED)
                {
                    return &slot;
                }
            }
            return nullptr;
        }

        const Slot* find_first_occupied() const
        {
            if (slots_.empty())
            {
                return nullptr;
            }
            for (const auto& slot : slots_)
            {
                if (slot.state == Slot::OCCUPIED)
                {
                    return &slot;
                }
            }
            return nullptr;
        }

        Slot* find_first_occupied_from(size_type idx)
        {
            for (; idx < slots_.size(); ++idx)
            {
                if (slots_[idx].state == Slot::OCCUPIED)
                {
                    return slots_.data() + idx;
                }
            }
            return nullptr;
        }

        template <typename IterType>
        IterType make_iter(Slot* ptr)
        {
            IterType it;
            it.slot_ = ptr;
            it.slots_end_ = ptr ? slots_.data() + slots_.size() : nullptr;
            return it;
        }

        const_iterator make_const_iter(const Slot* ptr) const
        {
            const_iterator it;
            it.slot_ = ptr;
            it.slots_end_ = ptr ? slots_.data() + slots_.size() : nullptr;
            return it;
        }
    };
}
