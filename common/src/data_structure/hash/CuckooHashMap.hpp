/**
 * @file CuckooHashMap.hpp
 * @brief Open-addressing hash map using Cuckoo hashing with a stash
 * @details A hash map employing Cuckoo hashing with two independent tables,
 *          a small stash for overflow resolution, and seed-multiplexed hash
 *          functions.  Each key has exactly two candidate positions (one in
 *          each table); on insertion collision the incumbent is displaced
 *          (kicked) to its alternate position.  After MAX_KICKS displacements
 *          without success, the entry is placed in the stash.  If the stash
 *          is also full, the tables are grown and all entries rehashed.
 *          Erase marks the slot empty with no back-shift needed — the two
 *          candidate positions guarantee find correctness without tombstones.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Complexity
 * - insert: O(1) amortised expected, O(log n) worst-case (stash fallback)
 * - find / erase: O(1) worst-case (2 table lookups + at most 8 stash slots)
 * - Memory overhead: two tables each at 50% of total capacity + 8-entry stash
 *
 * @par Usage Example
 * @code
 * CuckooHashMap<std::string, int> map;
 * map.insert("alice", 100);
 * map.insert("bob", 200);
 * assert(*map.find("alice") == 100);
 * @endcode
 */

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace common::data_structure::hash
{
    /// @brief A hash map using Cuckoo hashing with two tables and a stash.
    /// @tparam K Key type.
    /// @tparam V Value type.
    /// @tparam Hasher Hash function object type, defaults to std::hash<K>.
    /// @tparam KeyEqual Equality comparison object type, defaults to std::equal_to<K>.
    template <typename K, typename V, typename Hasher = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class CuckooHashMap
    {
    public:
        using size_type = std::size_t;

        /// @brief Storage region identifiers for the three-region iterator.
        enum class Region
        {
            T1,
            T2,
            STASH,
            END
        };

    private:
        static constexpr size_type INITIAL_CAPACITY = 4;
        static constexpr size_type STASH_SIZE = 8;
        static constexpr uint64_t GOLDEN_RATIO = 0x9E3779B97F4A7C15;
        static constexpr double DEFAULT_MAX_LOAD_FACTOR = 0.75;

        /// @brief Table slot with state, key, and value.
        struct Slot
        {
            enum State : uint8_t
            {
                EMPTY,
                OCCUPIED
            };

            State state = EMPTY;
            K key{};
            V value{};
        };

    public:
        // ── Iterator ──────────────────────────────────────────────────

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

            /// @brief Dereferences to a pair of key/value references.
            reference operator*() const
            {
                return {slot_->key, slot_->value};
            }

            /// @brief Advances to the next occupied slot.
            iterator& operator++()
            {
                if (!slot_)
                {
                    return *this;
                }
                ++slot_;
                advance_to_next_occupied();
                return *this;
            }

            /// @brief Post-increment.
            iterator operator++(int)
            {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const iterator& other) const { return slot_ == other.slot_; }
            bool operator!=(const iterator& other) const { return slot_ != other.slot_; }

        private:
            friend class CuckooHashMap;

            Slot* slot_ = nullptr;
            Region region_ = Region::END;
            Slot* t1_end_ = nullptr;
            Slot* t2_begin_ = nullptr;
            Slot* t2_end_ = nullptr;
            Slot* stash_begin_ = nullptr;
            Slot* stash_end_ = nullptr;

            /// @brief Scans forward from the current (incremented) slot
            ///        across region boundaries until an OCCUPIED slot is
            ///        found or all regions are exhausted.
            void advance_to_next_occupied()
            {
                while (region_ != Region::END)
                {
                    Slot* region_end = nullptr;
                    switch (region_)
                    {
                        case Region::T1:    region_end = t1_end_;      break;
                        case Region::T2:    region_end = t2_end_;      break;
                        case Region::STASH: region_end = stash_end_;   break;
                        default:                                        break;
                    }

                    while (slot_ != region_end)
                    {
                        if (slot_->state == Slot::OCCUPIED)
                        {
                            return;
                        }
                        ++slot_;
                    }

                    // Transition to the next region
                    switch (region_)
                    {
                        case Region::T1:
                            region_ = Region::T2;
                            slot_ = t2_begin_;
                            break;
                        case Region::T2:
                            region_ = Region::STASH;
                            slot_ = stash_begin_;
                            break;
                        case Region::STASH:
                            region_ = Region::END;
                            slot_ = nullptr;
                            break;
                        default:
                            region_ = Region::END;
                            slot_ = nullptr;
                            break;
                    }
                }
            }
        };

        // ── Const Iterator ────────────────────────────────────────────

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
                if (!slot_)
                {
                    return *this;
                }
                ++slot_;
                advance_to_next_occupied();
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
            friend class CuckooHashMap;

            const Slot* slot_ = nullptr;
            Region region_ = Region::END;
            const Slot* t1_end_ = nullptr;
            const Slot* t2_begin_ = nullptr;
            const Slot* t2_end_ = nullptr;
            const Slot* stash_begin_ = nullptr;
            const Slot* stash_end_ = nullptr;

            void advance_to_next_occupied()
            {
                while (region_ != Region::END)
                {
                    const Slot* region_end = nullptr;
                    switch (region_)
                    {
                        case Region::T1:    region_end = t1_end_;      break;
                        case Region::T2:    region_end = t2_end_;      break;
                        case Region::STASH: region_end = stash_end_;   break;
                        default:                                        break;
                    }

                    while (slot_ != region_end)
                    {
                        if (slot_->state == Slot::OCCUPIED)
                        {
                            return;
                        }
                        ++slot_;
                    }

                    switch (region_)
                    {
                        case Region::T1:
                            region_ = Region::T2;
                            slot_ = t2_begin_;
                            break;
                        case Region::T2:
                            region_ = Region::STASH;
                            slot_ = stash_begin_;
                            break;
                        case Region::STASH:
                            region_ = Region::END;
                            slot_ = nullptr;
                            break;
                        default:
                            region_ = Region::END;
                            slot_ = nullptr;
                            break;
                    }
                }
            }
        };

        // ── Construction ──────────────────────────────────────────────

        /// @brief Default constructor.  Creates an empty map with
        ///        INITIAL_CAPACITY per table.
        CuckooHashMap() noexcept
        {
            alloc_tables(INITIAL_CAPACITY);
        }

        /// @brief Constructs with a given minimum capacity (total across both tables).
        /// @param capacity Minimum total capacity hint.
        explicit CuckooHashMap(size_type capacity)
        {
            const auto per_table = std::max(INITIAL_CAPACITY, next_power_of_two(capacity / 2 + 1));
            alloc_tables(per_table);
        }

        /// @brief Copy constructor.
        CuckooHashMap(const CuckooHashMap& other)
            : table1_(other.table1_)
            , table2_(other.table2_)
            , stash_(other.stash_)
            , size_(other.size_)
            , seed_(other.seed_)
            , mask1_(other.mask1_)
            , mask2_(other.mask2_)
            , max_load_factor_(other.max_load_factor_)
            , hasher_(other.hasher_)
            , key_equal_(other.key_equal_)
        {
        }

        /// @brief Move constructor.
        CuckooHashMap(CuckooHashMap&& other) noexcept
            : table1_(std::move(other.table1_))
            , table2_(std::move(other.table2_))
            , stash_(std::move(other.stash_))
            , size_(other.size_)
            , seed_(other.seed_)
            , mask1_(other.mask1_)
            , mask2_(other.mask2_)
            , max_load_factor_(other.max_load_factor_)
            , hasher_(std::move(other.hasher_))
            , key_equal_(std::move(other.key_equal_))
        {
            other.size_ = 0;
            other.seed_ = 0;
            other.mask1_ = 0;
            other.mask2_ = 0;
            other.max_load_factor_ = DEFAULT_MAX_LOAD_FACTOR;
        }

        /// @brief Copy assignment.
        CuckooHashMap& operator=(const CuckooHashMap& other)
        {
            if (this != &other)
            {
                table1_ = other.table1_;
                table2_ = other.table2_;
                stash_ = other.stash_;
                size_ = other.size_;
                seed_ = other.seed_;
                mask1_ = other.mask1_;
                mask2_ = other.mask2_;
                max_load_factor_ = other.max_load_factor_;
                hasher_ = other.hasher_;
                key_equal_ = other.key_equal_;
            }
            return *this;
        }

        /// @brief Move assignment.
        CuckooHashMap& operator=(CuckooHashMap&& other) noexcept
        {
            if (this != &other)
            {
                table1_ = std::move(other.table1_);
                table2_ = std::move(other.table2_);
                stash_ = std::move(other.stash_);
                size_ = other.size_;
                seed_ = other.seed_;
                mask1_ = other.mask1_;
                mask2_ = other.mask2_;
                max_load_factor_ = other.max_load_factor_;
                hasher_ = std::move(other.hasher_);
                key_equal_ = std::move(other.key_equal_);
                other.size_ = 0;
                other.seed_ = 0;
                other.mask1_ = 0;
                other.mask2_ = 0;
                other.max_load_factor_ = DEFAULT_MAX_LOAD_FACTOR;
            }
            return *this;
        }

        /// @brief Destructor.
        ~CuckooHashMap() noexcept = default;

        // ── Iterators ─────────────────────────────────────────────────

        /// @brief Returns an iterator to the first occupied element.
        [[nodiscard]] iterator begin()
        {
            auto it = make_iter<iterator>(nullptr);
            init_iter_begin(it);
            return it;
        }

        /// @brief Returns a const iterator to the first occupied element.
        [[nodiscard]] const_iterator begin() const
        {
            auto it = make_const_iter(nullptr);
            init_iter_begin(it);
            return it;
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

        // ── Modifiers: insert ─────────────────────────────────────────

        /// @brief Inserts a key-value pair.  Does nothing if the key already exists.
        /// @param key The key to insert.
        /// @param value The value to insert.
        /// @return A pair of iterator to the element and a bool (true if inserted).
        std::pair<iterator, bool> insert(const K& key, const V& value)
        {
            return insert_impl(key, value);
        }

        /// @brief Inserts a key-value pair using move semantics.  Does nothing if the key already exists.
        /// @param key The key to insert.
        /// @param value The value to insert.
        /// @return A pair of iterator to the element and a bool (true if inserted).
        std::pair<iterator, bool> insert(K&& key, V&& value)
        {
            return insert_impl(std::move(key), std::move(value));
        }

        /// @brief Inserts a key-value pair, or assigns the value if the key already exists.
        /// @param key The key to insert or assign.
        /// @param value The value to insert or assign.
        /// @return A pair of iterator to the element and a bool (true if inserted, false if assigned).
        std::pair<iterator, bool> insert_or_assign(const K& key, const V& value)
        {
            return insert_or_assign_impl(key, value);
        }

        /// @brief Inserts a key-value pair with move semantics, or assigns if key exists.
        /// @param key The key to insert or assign.
        /// @param value The value to insert or assign.
        /// @return A pair of iterator to the element and a bool (true if inserted, false if assigned).
        std::pair<iterator, bool> insert_or_assign(K&& key, V&& value)
        {
            return insert_or_assign_impl(std::move(key), std::move(value));
        }

        // ── Modifiers: erase ──────────────────────────────────────────

        /// @brief Erases the element with the given key.
        /// @param key The key to erase.
        /// @return True if the element was erased, false if the key was not found.
        bool erase(const K& key)
        {
            auto* slot = find_slot(key);
            if (!slot)
            {
                return false;
            }
            slot->state = Slot::EMPTY;
            --size_;
            return true;
        }

        /// @brief Erases the element at the given const iterator position.
        /// @param it Iterator to the element to erase (must be valid and dereferenceable).
        /// @return Iterator to the next occupied element.
        iterator erase(const_iterator it)
        {
            if (!it.slot_ || it.slot_->state != Slot::OCCUPIED)
            {
                return end();
            }
            // const_cast is safe: the slot belongs to this non-const map
            const_cast<Slot*>(it.slot_)->state = Slot::EMPTY;
            --size_;
            auto* next = find_next_occupied(it.slot_);
            return make_iter<iterator>(next);
        }

        /// @brief Erases the element at the given iterator position.
        /// @param it Iterator to the element to erase (must be valid and dereferenceable).
        /// @return Iterator to the next occupied element.
        iterator erase(iterator it)
        {
            if (!it.slot_ || it.slot_->state != Slot::OCCUPIED)
            {
                return end();
            }
            it.slot_->state = Slot::EMPTY;
            --size_;
            auto* next = find_next_occupied(it.slot_);
            return make_iter<iterator>(next);
        }

        // ── Lookup ────────────────────────────────────────────────────

        /// @brief Finds an element by key.
        /// @param key The key to search for.
        /// @return Iterator to the element, or end() if not found.
        [[nodiscard]] iterator find(const K& key)
        {
            auto* slot = find_slot(key);
            if (!slot)
            {
                return end();
            }
            return make_iter<iterator>(slot);
        }

        /// @brief Finds an element by key (const).
        /// @param key The key to search for.
        /// @return Const iterator to the element, or cend() if not found.
        [[nodiscard]] const_iterator find(const K& key) const
        {
            const auto* slot = find_slot(key);
            if (!slot)
            {
                return end();
            }
            return make_const_iter(slot);
        }

        /// @brief Checks if a key exists in the map.
        /// @param key The key to check.
        /// @return True if the key exists, false otherwise.
        [[nodiscard]] bool contains(const K& key) const
        {
            return find_slot(key) != nullptr;
        }

        /// @brief Accesses the value associated with the given key.  Throws if not found.
        /// @param key The key to look up.
        /// @return Reference to the value.
        /// @throws std::out_of_range If the key is not found.
        [[nodiscard]] V& at(const K& key)
        {
            auto* slot = find_slot(key);
            if (!slot)
            {
                throw std::out_of_range("CuckooHashMap::at: key not found");
            }
            return slot->value;
        }

        /// @brief Accesses the value associated with the given key (const).  Throws if not found.
        /// @param key The key to look up.
        /// @return Const reference to the value.
        /// @throws std::out_of_range If the key is not found.
        [[nodiscard]] const V& at(const K& key) const
        {
            const auto* slot = find_slot(key);
            if (!slot)
            {
                throw std::out_of_range("CuckooHashMap::at: key not found");
            }
            return slot->value;
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

        // ── Capacity ──────────────────────────────────────────────────

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

        /// @brief Returns the total capacity (both tables combined).
        [[nodiscard]] size_type bucket_count() const noexcept
        {
            return table1_.size() + table2_.size();
        }

        /// @brief Returns the current load factor.
        [[nodiscard]] double load_factor() const noexcept
        {
            const auto cap = bucket_count();
            return cap > 0 ? static_cast<double>(size_) / static_cast<double>(cap) : 0.0;
        }

        /// @brief Returns the maximum load factor.
        [[nodiscard]] double max_load_factor() const noexcept
        {
            return max_load_factor_;
        }

        /// @brief Sets the maximum load factor.  Rehashes if current load factor exceeds new value.
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
            rehash(needed);
        }

        /// @brief Rehashes to the specified number of buckets (total across both tables).
        void rehash(size_type new_total_capacity)
        {
            auto per_table = std::max(INITIAL_CAPACITY, next_power_of_two(new_total_capacity / 2 + 1));
            if (per_table == table1_.size())
            {
                return;
            }
            rehash_impl(per_table);
        }

        // ── Modifiers: clear / swap ───────────────────────────────────

        /// @brief Removes all elements from the map.
        void clear() noexcept
        {
            for (auto& slot : table1_)
            {
                slot.state = Slot::EMPTY;
            }
            for (auto& slot : table2_)
            {
                slot.state = Slot::EMPTY;
            }
            for (auto& slot : stash_)
            {
                slot.state = Slot::EMPTY;
            }
            size_ = 0;
        }

        /// @brief Swaps the contents with another CuckooHashMap.
        void swap(CuckooHashMap& other) noexcept
        {
            using std::swap;
            swap(table1_, other.table1_);
            swap(table2_, other.table2_);
            swap(stash_, other.stash_);
            swap(size_, other.size_);
            swap(seed_, other.seed_);
            swap(mask1_, other.mask1_);
            swap(mask2_, other.mask2_);
            swap(max_load_factor_, other.max_load_factor_);
            swap(hasher_, other.hasher_);
            swap(key_equal_, other.key_equal_);
        }

    private:
        // ── Member Variables ─────────────────────────────────────────

        std::vector<Slot> table1_{};
        std::vector<Slot> table2_{};
        std::array<Slot, STASH_SIZE> stash_{};
        size_type size_ = 0;
        uint64_t seed_ = 0;
        size_type mask1_ = 0;
        size_type mask2_ = 0;
        double max_load_factor_ = DEFAULT_MAX_LOAD_FACTOR;
        Hasher hasher_{};
        KeyEqual key_equal_{};

        // ── Hash Helpers ─────────────────────────────────────────────

        /// @brief Avalanche-style hash mixer: xors with seed, then
        ///        applies three-round MurmurHash3 fmix64.
        static auto hash_mix(uint64_t h, uint64_t seed) -> uint64_t
        {
            h ^= seed;
            h ^= h >> 33;
            h *= 0xFF51AFD7ED558CCDULL;
            h ^= h >> 33;
            h *= 0xC4CEB9FE1A85EC53ULL;
            h ^= h >> 33;
            return h;
        }

        /// @brief Returns the candidate index in table1 for a given key.
        auto h1(const K& key) const -> size_type
        {
            return static_cast<size_type>(hash_mix(static_cast<uint64_t>(hasher_(key)), seed_)) & mask1_;
        }

        /// @brief Returns the candidate index in table2 for a given key.
        auto h2(const K& key) const -> size_type
        {
            return static_cast<size_type>(hash_mix(static_cast<uint64_t>(hasher_(key)) + GOLDEN_RATIO, seed_)) & mask2_;
        }

        // ── Internal Helpers ─────────────────────────────────────────

        /// @brief Rounds up to the next power of two.
        static auto next_power_of_two(size_type n) noexcept -> size_type
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

        /// @brief Returns the maximum number of kick attempts before
        ///        falling back to the stash.
        [[nodiscard]] auto max_kicks() const noexcept -> size_type
        {
            const auto total_cap = table1_.size() + table2_.size();
            return std::max<size_type>(8, static_cast<size_type>(std::log2(static_cast<double>(total_cap))));
        }

        // ── Table Management ─────────────────────────────────────────

        /// @brief Allocates both tables with the given per-table capacity
        ///        (must be a power of two).
        void alloc_tables(size_type per_table_cap)
        {
            table1_.assign(per_table_cap, Slot{});
            table2_.assign(per_table_cap, Slot{});
            mask1_ = per_table_cap - 1;
            mask2_ = per_table_cap - 1;
        }

        // ── Insert Implementation ────────────────────────────────────

        /// @brief Core insert logic for lvalue references.
        std::pair<iterator, bool> insert_impl(const K& key, const V& value)
        {
            // Check duplicate
            auto* existing = find_slot(key);
            if (existing)
            {
                return {make_iter<iterator>(existing), false};
            }

            ensure_capacity();

            Slot entry{Slot::OCCUPIED, key, value};
            return insert_new_entry(std::move(entry));
        }

        /// @brief Core insert logic for rvalue references.
        std::pair<iterator, bool> insert_impl(K&& key, V&& value)
        {
            // Check duplicate
            auto* existing = find_slot(key);
            if (existing)
            {
                return {make_iter<iterator>(existing), false};
            }

            ensure_capacity();

            Slot entry{Slot::OCCUPIED, std::move(key), std::move(value)};
            return insert_new_entry(std::move(entry));
        }

        /// @brief Core insert_or_assign for lvalue references.
        std::pair<iterator, bool> insert_or_assign_impl(const K& key, const V& value)
        {
            auto* existing = find_slot(key);
            if (existing)
            {
                existing->value = value;
                return {make_iter<iterator>(existing), false};
            }

            ensure_capacity();

            Slot entry{Slot::OCCUPIED, key, value};
            return insert_new_entry(std::move(entry));
        }

        /// @brief Core insert_or_assign for rvalue references.
        std::pair<iterator, bool> insert_or_assign_impl(K&& key, V&& value)
        {
            auto* existing = find_slot(key);
            if (existing)
            {
                existing->value = std::move(value);
                return {make_iter<iterator>(existing), false};
            }

            ensure_capacity();

            Slot entry{Slot::OCCUPIED, std::move(key), std::move(value)};
            return insert_new_entry(std::move(entry));
        }

        /// @brief Inserts a new entry using cuckoo hashing.
        /// @pre The key is guaranteed not to exist in the map.
        /// @pre Capacity has been ensured.
        std::pair<iterator, bool> insert_new_entry(Slot&& entry)
        {
            for (size_type kicks = 0; kicks < max_kicks(); ++kicks)
            {
                // Try table1 at h1
                {
                    auto& s1 = table1_[h1(entry.key)];
                    if (s1.state == Slot::EMPTY)
                    {
                        s1 = std::move(entry);
                        s1.state = Slot::OCCUPIED;
                        ++size_;
                        return {make_iter<iterator>(&s1), true};
                    }
                    std::swap(entry, s1);
                }

                // Try table2 at h2
                {
                    auto& s2 = table2_[h2(entry.key)];
                    if (s2.state == Slot::EMPTY)
                    {
                        s2 = std::move(entry);
                        s2.state = Slot::OCCUPIED;
                        ++size_;
                        return {make_iter<iterator>(&s2), true};
                    }
                    std::swap(entry, s2);
                }
            }

            // MAX_KICKS exceeded — try stash
            for (auto& s : stash_)
            {
                if (s.state == Slot::EMPTY)
                {
                    s = std::move(entry);
                    s.state = Slot::OCCUPIED;
                    ++size_;
                    return {make_iter<iterator>(&s), true};
                }
            }

            // Stash full — rehash and retry
            rehash_impl(table1_.size() * 2);
            return insert_new_entry(std::move(entry));
        }

        /// @brief Inserts an entry without checking capacity or
        ///        duplicate key.  Used during rehash.
        /// @note This method may recursively grow tables in the
        ///       astronomically rare case of stash overflow even
        ///       in an empty table.
        void insert_no_resize(Slot&& entry)
        {
            for (size_type kicks = 0; kicks < max_kicks(); ++kicks)
            {
                {
                    auto& s1 = table1_[h1(entry.key)];
                    if (s1.state == Slot::EMPTY)
                    {
                        s1 = std::move(entry);
                        s1.state = Slot::OCCUPIED;
                        ++size_;
                        return;
                    }
                    std::swap(entry, s1);
                }
                {
                    auto& s2 = table2_[h2(entry.key)];
                    if (s2.state == Slot::EMPTY)
                    {
                        s2 = std::move(entry);
                        s2.state = Slot::OCCUPIED;
                        ++size_;
                        return;
                    }
                    std::swap(entry, s2);
                }
            }

            // Stash fallback
            for (auto& s : stash_)
            {
                if (s.state == Slot::EMPTY)
                {
                    s = std::move(entry);
                    s.state = Slot::OCCUPIED;
                    ++size_;
                    return;
                }
            }

            // Extremely rare — grow and retry
            rehash_impl(table1_.size() * 2);
            insert_no_resize(std::move(entry));
        }

        // ── Lookup Helpers ───────────────────────────────────────────

        /// @brief Finds the slot for a key, returning nullptr if absent.
        [[nodiscard]] auto find_slot(const K& key) -> Slot*
        {
            return const_cast<Slot*>(const_cast<const CuckooHashMap*>(this)->find_slot(key));
        }

        /// @brief Finds the slot for a key (const), returning nullptr if absent.
        [[nodiscard]] auto find_slot(const K& key) const -> const Slot*
        {
            if (table1_.empty())
            {
                return nullptr;
            }

            // Check table1
            const auto i1 = h1(key);
            const auto& s1 = table1_[i1];
            if (s1.state == Slot::OCCUPIED && key_equal_(s1.key, key))
            {
                return &s1;
            }

            // Check table2
            const auto i2 = h2(key);
            const auto& s2 = table2_[i2];
            if (s2.state == Slot::OCCUPIED && key_equal_(s2.key, key))
            {
                return &s2;
            }

            // Check stash
            for (const auto& s : stash_)
            {
                if (s.state == Slot::OCCUPIED && key_equal_(s.key, key))
                {
                    return &s;
                }
            }

            return nullptr;
        }

        /// @brief Finds the next occupied slot after a given slot.
        ///        Scans through table1, table2, and stash in that order.
        /// @note Uses std::less for cross-allocation pointer comparisons
        ///       to guarantee well-defined behaviour.
        [[nodiscard]] auto find_next_occupied(const Slot* after) -> Slot*
        {
            const Slot* t1d = table1_.data();
            const Slot* t2d = table2_.data();
            const Slot* std = stash_.data();
            const size_type t1s = table1_.size();
            const size_type t2s = table2_.size();

            std::less<const Slot*> less;

            // Determine region of 'after' using std::less for safety
            auto in_range = [&](const Slot* p, const Slot* base, size_type sz) -> bool
            {
                return base && !less(p, base) && less(p, base + sz);
            };

            int start_region = -1;  // 0=T1, 1=T2, 2=STASH
            if (in_range(after, t1d, t1s))       start_region = 0;
            else if (in_range(after, t2d, t2s))  start_region = 1;
            else if (in_range(after, std, STASH_SIZE)) start_region = 2;

            const Slot* scan = after + 1;

            for (int r = start_region; r <= 2; ++r)
            {
                const Slot* begin_ptr = nullptr;
                const Slot* end_ptr = nullptr;
                switch (r)
                {
                    case 0: begin_ptr = t1d; end_ptr = t1d + t1s; break;
                    case 1: begin_ptr = t2d; end_ptr = t2d + t2s; break;
                    case 2: begin_ptr = std; end_ptr = std + STASH_SIZE; break;
                    default: continue;
                }

                const Slot* iter = (r == start_region) ? scan : begin_ptr;

                // Intra-allocation scan — normal < is safe (same allocation)
                while (iter < end_ptr)
                {
                    if (iter->state == Slot::OCCUPIED)
                    {
                        return const_cast<Slot*>(iter);
                    }
                    ++iter;
                }
            }

            return nullptr;
        }

        // ── Capacity Management ──────────────────────────────────────

        /// @brief Ensures enough capacity for one more insert.
        void ensure_capacity()
        {
            const auto total_cap = bucket_count();
            if (total_cap == 0 || static_cast<double>(size_ + 1) / static_cast<double>(total_cap) > max_load_factor_)
            {
                const auto new_per_table = total_cap == 0 ? INITIAL_CAPACITY : table1_.size() * 2;
                rehash_impl(new_per_table);
            }
        }

        /// @brief Rehashes all entries into tables of the given per-table
        ///        capacity.  Increments the seed to obtain fresh hash
        ///        functions.
        void rehash_impl(size_type per_table_cap)
        {
            per_table_cap = std::max(INITIAL_CAPACITY, next_power_of_two(per_table_cap));

            auto old_t1 = std::move(table1_);
            auto old_t2 = std::move(table2_);
            auto old_stash = stash_;

            // Clear stash for reuse
            for (auto& s : stash_)
            {
                s.state = Slot::EMPTY;
            }

            alloc_tables(per_table_cap);
            size_ = 0;
            ++seed_;

            // Re-insert all old entries
            auto reinsert = [this](Slot& s)
            {
                if (s.state == Slot::OCCUPIED)
                {
                    insert_no_resize(std::move(s));
                }
            };

            for (auto& s : old_t1) { reinsert(s); }
            for (auto& s : old_t2) { reinsert(s); }
            for (auto& s : old_stash) { reinsert(s); }
        }

        // ── Iterator Factory ─────────────────────────────────────────

        /// @brief Creates an iterator positioned at the given slot.
        template <typename IterType>
        auto make_iter(Slot* ptr) -> IterType
        {
            IterType it;
            if (ptr)
            {
                it.slot_ = ptr;
                it.t1_end_ = table1_.data() + table1_.size();
                it.t2_begin_ = table2_.data();
                it.t2_end_ = table2_.data() + table2_.size();
                it.stash_begin_ = stash_.data();
                it.stash_end_ = stash_.data() + STASH_SIZE;

                // Determine the region for ptr
                if (ptr >= table1_.data() && ptr < it.t1_end_)
                {
                    it.region_ = Region::T1;
                }
                else if (ptr >= it.t2_begin_ && ptr < it.t2_end_)
                {
                    it.region_ = Region::T2;
                }
                else
                {
                    it.region_ = Region::STASH;
                }
            }
            return it;
        }

        /// @brief Creates a const iterator positioned at the given slot.
        auto make_const_iter(const Slot* ptr) const -> const_iterator
        {
            const_iterator it;
            if (ptr)
            {
                it.slot_ = ptr;
                it.t1_end_ = table1_.data() + table1_.size();
                it.t2_begin_ = table2_.data();
                it.t2_end_ = table2_.data() + table2_.size();
                it.stash_begin_ = stash_.data();
                it.stash_end_ = stash_.data() + STASH_SIZE;

                if (ptr >= table1_.data() && ptr < it.t1_end_)
                {
                    it.region_ = Region::T1;
                }
                else if (ptr >= it.t2_begin_ && ptr < it.t2_end_)
                {
                    it.region_ = Region::T2;
                }
                else
                {
                    it.region_ = Region::STASH;
                }
            }
            return it;
        }

        /// @brief Initialises a non-const iterator to the first occupied slot.
        void init_iter_begin(iterator& it)
        {
            if (table1_.empty()) { return; }

            it.t1_end_ = table1_.data() + table1_.size();
            it.t2_begin_ = table2_.data();
            it.t2_end_ = table2_.data() + table2_.size();
            it.stash_begin_ = stash_.data();
            it.stash_end_ = stash_.data() + STASH_SIZE;

            for (auto* s = table1_.data(); s != it.t1_end_; ++s)
            {
                if (s->state == Slot::OCCUPIED) { it.slot_ = s; it.region_ = Region::T1; return; }
            }
            for (auto* s = it.t2_begin_; s != it.t2_end_; ++s)
            {
                if (s->state == Slot::OCCUPIED) { it.slot_ = s; it.region_ = Region::T2; return; }
            }
            for (auto* s = it.stash_begin_; s != it.stash_end_; ++s)
            {
                if (s->state == Slot::OCCUPIED) { it.slot_ = s; it.region_ = Region::STASH; return; }
            }
        }

        /// @brief Initialises a const iterator to the first occupied slot.
        void init_iter_begin(const_iterator& it) const
        {
            if (table1_.empty()) { return; }

            it.t1_end_ = table1_.data() + table1_.size();
            it.t2_begin_ = table2_.data();
            it.t2_end_ = table2_.data() + table2_.size();
            it.stash_begin_ = stash_.data();
            it.stash_end_ = stash_.data() + STASH_SIZE;

            for (const auto* s = table1_.data(); s != it.t1_end_; ++s)
            {
                if (s->state == Slot::OCCUPIED) { it.slot_ = s; it.region_ = Region::T1; return; }
            }
            for (const auto* s = it.t2_begin_; s != it.t2_end_; ++s)
            {
                if (s->state == Slot::OCCUPIED) { it.slot_ = s; it.region_ = Region::T2; return; }
            }
            for (const auto* s = it.stash_begin_; s != it.stash_end_; ++s)
            {
                if (s->state == Slot::OCCUPIED) { it.slot_ = s; it.region_ = Region::STASH; return; }
            }
        }
    };
}  // namespace common::data_structure::hash
