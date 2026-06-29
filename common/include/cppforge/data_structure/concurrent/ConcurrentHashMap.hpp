/**
 * @file ConcurrentHashMap.hpp
 * @brief ConcurrentHashMap �?segment-locked concurrent hash map
 * @details A thread-safe hash map using striped locking with per-segment
 *          std::shared_mutex.  The hash space is divided into 2^SegmentBits
 *          segments, each protected by an independent read-write lock.
 *          Read operations (get, contains, for_each) acquire a shared lock
 *          on the relevant segment, allowing concurrent readers to proceed
 *          in parallel.  Write operations (insert, erase, clear) acquire an
 *          exclusive lock, blocking both other writers and readers on the
 *          same segment.  Operations on different segments never block each
 *          other.  Each segment uses separate chaining (singly-linked list)
 *          and grows independently without blocking the entire map.
 *
 * @par Thread Safety
 * All public methods are thread-safe.  Concurrent reads on the same segment
 * are allowed; writes serialize per segment.  Iteration via for_each holds
 * a shared lock on each segment in turn �?no snapshot consistency guarantee
 * across segments.
 *
 * Reference: Doug Lea, "ConcurrentHashMap in JSR 166" (Java), adapted for
 *            C++26 with std::shared_mutex.
 */

#pragma once

#include <cppforge/data_structure/concurrent/IConcurrentMap.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <type_traits>
#include <utility>

namespace cppforge::data_structure::concurrent
{
    /// @brief A thread-safe hash map with striped locking and per-segment
    ///        rehash.
    ///
    /// Implements IConcurrentMap<K, V> using a segmented design where each
    /// segment has its own read-write lock (std::shared_mutex) and owns a
    /// contiguous sub-table implemented via separate chaining with singly-
    /// linked lists.  When a segment's load factor exceeds the threshold,
    /// only that segment is rehashed (capacity doubled), leaving all other
    /// segments fully operational.
    ///
    /// @tparam K           Key type.
    /// @tparam V           Value type.
    /// @tparam Hasher      Hash functor, defaults to std::hash<K>.
    /// @tparam KeyEqual    Equality comparison functor, defaults to
    ///                     std::equal_to<K>.
    /// @tparam SegmentBits Number of bits used to select a segment (must be
    ///                     in [0, 8 * sizeof(std::size_t)).  The segment
    ///                     count is 1 << SegmentBits.  Default (6) yields
    ///                     64 segments.
    ///
    /// @par Thread Safety
    /// All public methods are thread-safe.  Read operations acquire a shared
    /// lock on the relevant segment; write operations acquire an exclusive
    /// lock.  Mutations to different segments are fully concurrent.
    ///
    /// Note: `size()` and `empty()` return approximate values computed from
    /// relaxed atomic loads without locking.  Under concurrent insertions and
    /// erasures the returned value may momentarily lag behind the true count.
    ///
    /// @par Usage Example
    /// @code
    /// ConcurrentHashMap<std::string, int> map;
    /// map.insert("alice", 100);
    /// map.insert_or_assign("bob", 200);
    ///
    /// if (auto val = map.get("alice")) {
    ///     // use *val
    /// }
    ///
    /// map.for_each([](const std::string& k, int v) {
    ///     // process each entry
    /// });
    /// @endcode
    template <typename K, typename V,
              typename Hasher   = std::hash<K>,
              typename KeyEqual = std::equal_to<K>,
              uint8_t SegmentBits = 6>
    class ConcurrentHashMap final : public IConcurrentMap<K, V>
    {
    public:
        // ── Constants ──────────────────────────────────────────────────

        /// @brief Number of segments = 1 << SegmentBits.
        static constexpr std::size_t SEGMENT_COUNT = 1ULL << SegmentBits;

        /// @brief Default maximum load factor before a segment rehashes.
        static constexpr double DEFAULT_MAX_LOAD_FACTOR = 0.75;

        /// @brief Initial bucket capacity per segment (must be a power of 2).
        static constexpr std::size_t INITIAL_SEGMENT_CAPACITY = 16;

        // Ensure SegmentBits is safe: shifting by >= bit-width is UB.
        static_assert(SegmentBits < sizeof(std::size_t) * 8,
                      "SegmentBits must be less than the bit-width of std::size_t");

        // Prevent pathological segment counts (practical sanity limit).
        static_assert(SEGMENT_COUNT <= 65536,
                      "SEGMENT_COUNT exceeds 65536 �?use fewer segment bits");

        // Ensure hash / equality functors are safe to move in noexcept contexts.
        static_assert(std::is_nothrow_move_constructible_v<Hasher>,
                      "Hasher must be nothrow move constructible for noexcept move semantics");
        static_assert(std::is_nothrow_move_constructible_v<KeyEqual>,
                      "KeyEqual must be nothrow move constructible for noexcept move semantics");

    private:
        // ── Internal types ─────────────────────────────────────────────

        /// @brief A singly-linked list node holding a key-value pair.
        struct Node
        {
            K     key;
            V     value;
            Node* next;

            Node(K&& k, V&& v, Node* nxt)
                : key(std::move(k))
                , value(std::move(v))
                , next(nxt)
            {
            }
        };

        /// @brief A segment of the hash table, aligned to a cache line.
        ///
        /// Each segment owns its own mutex, bucket array, capacity, and
        /// element count.  The alignas(64) prevents false sharing between
        /// adjacent segments in the segments_ array.
        struct alignas(64) Segment
        {
            mutable std::shared_mutex mtx;
            Node**                    buckets  = nullptr;
            std::size_t               capacity = 0;
            std::atomic<std::size_t>  size{0};
        };

    public:
        // ── Construction / destruction ─────────────────────────────────

        /// @brief Constructs an empty ConcurrentHashMap.
        ConcurrentHashMap()
            : max_load_factor_(DEFAULT_MAX_LOAD_FACTOR)
            , hasher_()
            , key_equal_()
        {
            try
            {
                for (auto& seg : segments_)
                {
                    seg.capacity = INITIAL_SEGMENT_CAPACITY;
                    seg.buckets  = new Node*[seg.capacity]();  // value-initialised to nullptr
                }
            }
            catch (...)
            {
                destroy_all_segments();
                throw;
            }
        }

        /// @brief Constructs an empty ConcurrentHashMap with a custom
        ///        maximum load factor.
        /// @param max_load_factor  Maximum load factor per segment before
        ///                         rehash (must be > 0).
        explicit ConcurrentHashMap(double max_load_factor)
            : max_load_factor_(max_load_factor > 0.0 ? max_load_factor
                                                     : DEFAULT_MAX_LOAD_FACTOR)
            , hasher_()
            , key_equal_()
        {
            try
            {
                for (auto& seg : segments_)
                {
                    seg.capacity = INITIAL_SEGMENT_CAPACITY;
                    seg.buckets  = new Node*[seg.capacity]();
                }
            }
            catch (...)
            {
                destroy_all_segments();
                throw;
            }
        }

        // Disable copy (shared_mutex is non-copyable)
        ConcurrentHashMap(const ConcurrentHashMap&) = delete;
        auto operator=(const ConcurrentHashMap&) -> ConcurrentHashMap& = delete;

        // ── Move ───────────────────────────────────────────────────────

        /// @brief Move constructor.
        ///
        /// Transfers all segment data from @p other.  The moved-from object
        /// is left in a valid empty state (all bucket pointers null).
        /// std::shared_mutex is not movable, so each segment's mutex remains
        /// in its new home and the moved-from object gets a fresh unlocked
        /// mutex.
        ConcurrentHashMap(ConcurrentHashMap&& other) noexcept
            : max_load_factor_(other.max_load_factor_)
            , hasher_(std::move(other.hasher_))
            , key_equal_(std::move(other.key_equal_))
        {
            for (std::size_t i = 0; i < SEGMENT_COUNT; ++i)
            {
                segments_[i].buckets               = other.segments_[i].buckets;
                segments_[i].capacity              = other.segments_[i].capacity;
                segments_[i].size.store(other.segments_[i].size.load(std::memory_order_relaxed),
                                        std::memory_order_relaxed);

                other.segments_[i].buckets  = nullptr;
                other.segments_[i].capacity = 0;
                other.segments_[i].size.store(0, std::memory_order_relaxed);
            }
        }

        /// @brief Move assignment.
        ///
        /// Releases the current object's resources, then transfers ownership
        /// of @p other's segment data.
        auto operator=(ConcurrentHashMap&& other) noexcept -> ConcurrentHashMap&
        {
            if (this != &other)
            {
                // Release our own resources
                destroy_all_segments();

                // Steal other's data segment by segment
                for (std::size_t i = 0; i < SEGMENT_COUNT; ++i)
                {
                    segments_[i].buckets               = other.segments_[i].buckets;
                    segments_[i].capacity              = other.segments_[i].capacity;
                    segments_[i].size.store(other.segments_[i].size.load(std::memory_order_relaxed),
                                            std::memory_order_relaxed);

                    other.segments_[i].buckets  = nullptr;
                    other.segments_[i].capacity = 0;
                    other.segments_[i].size.store(0, std::memory_order_relaxed);
                }

                max_load_factor_ = other.max_load_factor_;
                hasher_          = std::move(other.hasher_);
                key_equal_       = std::move(other.key_equal_);
            }
            return *this;
        }

        /// @brief Destructor �?frees all nodes and bucket arrays.
        ~ConcurrentHashMap() noexcept
        {
            destroy_all_segments();
        }

        // ── IConcurrentMap interface ───────────────────────────────────

        /// @brief Inserts a key-value pair if the key does not already exist.
        /// @param key   Key to insert.
        /// @param value Value to associate with the key.
        /// @return true if inserted, false if the key already exists.
        [[nodiscard]] auto insert(K key, V value) -> bool override
        {
            auto& seg            = segment_for(key);
            bool  inserted       = false;

            {
                std::unique_lock lock(seg.mtx);

                auto  bucket_idx = bucket_index(key, seg.capacity);
                // Scan the chain; if key already present, do nothing.
                Node* cur = seg.buckets[bucket_idx];
                while (cur)
                {
                    if (key_equal_(cur->key, key))
                    {
                        return false;
                    }
                    cur = cur->next;
                }

                // Head insertion.
                seg.buckets[bucket_idx] = new Node(std::move(key), std::move(value),
                                                   seg.buckets[bucket_idx]);
                seg.size.fetch_add(1, std::memory_order_relaxed);
                inserted = true;

                // Rehash check inside the lock (no TOCTOU race).
                if (need_rehash(seg))
                {
                    rehash(seg);
                }
            }

            return inserted;
        }

        /// @brief Inserts or assigns a key-value pair.
        /// @param key   Key to insert or assign.
        /// @param value Value to associate.
        /// @return true if a new entry was inserted, false if an existing
        ///         entry was updated.
        [[nodiscard]] auto insert_or_assign(K key, V value) -> bool override
        {
            auto& seg        = segment_for(key);
            bool  inserted   = false;

            {
                std::unique_lock lock(seg.mtx);

                auto  bucket_idx = bucket_index(key, seg.capacity);
                Node* cur = seg.buckets[bucket_idx];
                while (cur)
                {
                    if (key_equal_(cur->key, key))
                    {
                        cur->value = std::move(value);
                        return false;  // assigned, not inserted
                    }
                    cur = cur->next;
                }

                // Key not found �?insert.
                seg.buckets[bucket_idx] = new Node(std::move(key), std::move(value),
                                                   seg.buckets[bucket_idx]);
                seg.size.fetch_add(1, std::memory_order_relaxed);
                inserted = true;

                // Rehash check inside the lock (no TOCTOU race).
                if (need_rehash(seg))
                {
                    rehash(seg);
                }
            }

            return inserted;
        }

        /// @brief Retrieves the value for a key.
        /// @param key Key to look up.
        /// @return The value if found, or std::nullopt.
        [[nodiscard]] auto get(const K& key) const -> std::optional<V> override
        {
            const auto& seg = segment_for(key);

            std::shared_lock lock(seg.mtx);

            auto  bucket_idx = bucket_index(key, seg.capacity);
            Node* cur = seg.buckets[bucket_idx];
            while (cur)
            {
                if (key_equal_(cur->key, key))
                {
                    return cur->value;
                }
                cur = cur->next;
            }
            return std::nullopt;
        }

        /// @brief Checks whether a key exists.
        /// @param key Key to check.
        /// @return true if the key is present.
        [[nodiscard]] auto contains(const K& key) const -> bool override
        {
            const auto& seg = segment_for(key);

            std::shared_lock lock(seg.mtx);

            auto  bucket_idx = bucket_index(key, seg.capacity);
            Node* cur = seg.buckets[bucket_idx];
            while (cur)
            {
                if (key_equal_(cur->key, key))
                {
                    return true;
                }
                cur = cur->next;
            }
            return false;
        }

        /// @brief Erases the entry for the given key.
        /// @param key Key to erase.
        /// @return true if the key existed and was removed.
        [[nodiscard]] auto erase(const K& key) -> bool override
        {
            auto& seg = segment_for(key);

            std::unique_lock lock(seg.mtx);

            auto  bucket_idx = bucket_index(key, seg.capacity);
            Node* cur  = seg.buckets[bucket_idx];
            Node* prev = nullptr;

            while (cur)
            {
                if (key_equal_(cur->key, key))
                {
                    // Unlink
                    if (prev)
                    {
                        prev->next = cur->next;
                    }
                    else
                    {
                        seg.buckets[bucket_idx] = cur->next;
                    }

                    delete cur;
                    seg.size.fetch_sub(1, std::memory_order_relaxed);
                    return true;
                }
                prev = cur;
                cur  = cur->next;
            }
            return false;
        }

        /// @brief Returns the total number of entries in the map.
        /// @return Entry count (sum over all segments).
        [[nodiscard]] auto size() const -> std::size_t override
        {
            std::size_t total = 0;
            for (const auto& seg : segments_)
            {
                total += seg.size.load(std::memory_order_relaxed);
            }
            return total;
        }

        /// @brief Checks whether the map is empty.
        /// @return true if size() == 0.
        [[nodiscard]] auto empty() const -> bool override
        {
            return size() == 0;
        }

        /// @brief Removes all entries from the map.
        ///
        /// Each segment is locked exclusively, its nodes are freed, and the
        /// bucket pointers are reset to nullptr.  Bucket arrays themselves
        /// are preserved (no deallocation).
        void clear() override
        {
            for (auto& seg : segments_)
            {
                std::unique_lock lock(seg.mtx);

                for (std::size_t i = 0; i < seg.capacity; ++i)
                {
                    Node* cur = seg.buckets[i];
                    while (cur)
                    {
                        Node* to_delete = cur;
                        cur = cur->next;
                        delete to_delete;
                    }
                    seg.buckets[i] = nullptr;
                }
                seg.size.store(0, std::memory_order_relaxed);
            }
        }

        /// @brief Returns the current approximate load factor.
        ///
        /// Computed as a global average: total entries / total buckets.
        /// @return Load factor, or 0.0 if the map has no buckets.
        [[nodiscard]] auto load_factor() const -> double override
        {
            std::size_t total_entries = 0;
            std::size_t total_buckets = 0;
            for (const auto& seg : segments_)
            {
                total_entries += seg.size.load(std::memory_order_relaxed);
                total_buckets += seg.capacity;
            }
            return total_buckets > 0
                       ? static_cast<double>(total_entries) / static_cast<double>(total_buckets)
                       : 0.0;
        }

        /// @brief Returns the total number of buckets (sum over all segments).
        /// @return Total bucket count.
        [[nodiscard]] auto bucket_count() const -> std::size_t override
        {
            std::size_t total = 0;
            for (const auto& seg : segments_)
            {
                total += seg.capacity;
            }
            return total;
        }

    protected:
        /// @brief Applies a callable to every key-value pair while holding
        ///        a shared lock on each segment.
        /// @param func Callable accepting (const K&, const V&).
        void for_each_impl(std::function<void(const K&, const V&)> func) const override
        {
            for (const auto& seg : segments_)
            {
                std::shared_lock lock(seg.mtx);

                for (std::size_t i = 0; i < seg.capacity; ++i)
                {
                    Node* cur = seg.buckets[i];
                    while (cur)
                    {
                        func(cur->key, cur->value);
                        cur = cur->next;
                    }
                }
            }
        }

    private:
        // ── Member data ────────────────────────────────────────────────

        std::array<Segment, SEGMENT_COUNT> segments_;
        double                             max_load_factor_ = DEFAULT_MAX_LOAD_FACTOR;
        [[no_unique_address]] Hasher       hasher_;
        [[no_unique_address]] KeyEqual     key_equal_;

        // ── Segment helpers ────────────────────────────────────────────

        /// @brief Returns the mutable segment responsible for the given key.
        auto segment_for(const K& key) -> Segment&
        {
            return segments_[hasher_(key) & (SEGMENT_COUNT - 1)];
        }

        /// @brief Returns the const segment responsible for the given key.
        auto segment_for(const K& key) const -> const Segment&
        {
            return segments_[hasher_(key) & (SEGMENT_COUNT - 1)];
        }

        /// @brief Computes the bucket index within a segment for a given key.
        /// @param key      The key to hash.
        /// @param capacity The segment's current bucket count (power of 2).
        /// @return Index into the bucket array.
        [[nodiscard]] auto bucket_index(const K& key, std::size_t capacity) const -> std::size_t
        {
            return (hasher_(key) >> SegmentBits) & (capacity - 1);
        }

        /// @brief Checks whether a segment's load factor exceeds the max.
        [[nodiscard]] auto need_rehash(const Segment& seg) const -> bool
        {
            return seg.capacity > 0 &&
                   static_cast<double>(seg.size.load(std::memory_order_relaxed)) /
                           static_cast<double>(seg.capacity) >= max_load_factor_;
        }

        // ── Rehash ─────────────────────────────────────────────────────

        /// @brief Doubles the capacity of a segment and redistributes all
        ///        entries into the new bucket array.
        ///
        /// Must be called with an exclusive lock held on @p seg.mtx.
        /// Existing Node objects are reused (moved to new buckets); only
        /// the bucket array is replaced.
        /// @param seg The segment to rehash.
        void rehash(Segment& seg)
        {
            const auto new_capacity = seg.capacity * 2;
            auto* new_buckets       = new Node*[new_capacity]();  // nullptr-initialised

            // Redistribute existing nodes into the new bucket array
            for (std::size_t i = 0; i < seg.capacity; ++i)
            {
                Node* cur = seg.buckets[i];
                while (cur)
                {
                    Node* next          = cur->next;
                    auto  new_idx       = (hasher_(cur->key) >> SegmentBits) & (new_capacity - 1);
                    cur->next           = new_buckets[new_idx];
                    new_buckets[new_idx] = cur;
                    cur                 = next;
                }
            }

            // Swap in the new bucket array and discard the old one
            Node** old_buckets = seg.buckets;
            seg.buckets        = new_buckets;
            seg.capacity       = new_capacity;
            delete[] old_buckets;
        }

        // ── Resource cleanup ───────────────────────────────────────────

        /// @brief Destroys all nodes and bucket arrays in every segment.
        ///        Used by the destructor and move assignment.
        void destroy_all_segments() noexcept
        {
            for (auto& seg : segments_)
            {
                if (seg.buckets)
                {
                    for (std::size_t i = 0; i < seg.capacity; ++i)
                    {
                        Node* cur = seg.buckets[i];
                        while (cur)
                        {
                            Node* to_delete = cur;
                            cur = cur->next;
                            delete to_delete;
                        }
                    }
                    delete[] seg.buckets;
                    seg.buckets  = nullptr;
                    seg.capacity = 0;
                    seg.size.store(0, std::memory_order_relaxed);
                }
            }
        }
    };

} // namespace cppforge::data_structure::concurrent
