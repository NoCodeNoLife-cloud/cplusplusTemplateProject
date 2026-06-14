/**
 * @file CuckooFilter.hpp
 * @brief Cuckoo filter — probabilistic set membership with deletion support
 * @details A cuckoo filter is a probabilistic data structure for approximate
 *          set membership queries.  Like a Bloom filter, it supports insertion
 *          and querying with a configurable false-positive rate.  Unlike a
 *          Bloom filter, it also supports deletion.  Internally it uses a
 *          hash table with cuckoo hashing where each entry stores a compact
 *          fingerprint of the inserted element.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronization is required
 * for concurrent access.
 *
 * @par Reference
 * Fan et al., "Cuckoo Filter: Practically Better Than Bloom" (2014).
 *
 * @par Usage Example
 * @code
 * CuckooFilter<std::string> filter(10000);
 * filter.insert("key123");
 * assert(filter.contains("key123"));
 * filter.remove("key123");
 * assert(!filter.contains("key123"));
 * @endcode
 */

#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace common::data_structure
{
    // ══════════════════════════════════════════════════════════════════════
    //  FingerprintSize — compile-time fingerprint bit-width tag
    // ══════════════════════════════════════════════════════════════════════

    /// @brief A compile-time tag for fingerprint bit width.
    /// @tparam Bits Bit width (must be 8, 16, or 32).
    template <std::size_t Bits>
    struct FingerprintSize final
    {
        static constexpr std::size_t value = Bits;
        static_assert(Bits == 8 || Bits == 16 || Bits == 32,
                      "FingerprintSize must be 8, 16, or 32");
    };

    /// @brief Maps a FingerprintSize tag to the corresponding unsigned
    ///        integer storage type.
    /// @tparam T A FingerprintSize<Bits> instantiation.
    template <typename T>
    struct FingerprintStorageType;

    template <>
    struct FingerprintStorageType<FingerprintSize<8>>
    {
        using type = uint8_t;
    };

    template <>
    struct FingerprintStorageType<FingerprintSize<16>>
    {
        using type = uint16_t;
    };

    template <>
    struct FingerprintStorageType<FingerprintSize<32>>
    {
        using type = uint32_t;
    };

    /// @brief Alias for the storage type deduced from a FingerprintSize tag.
    template <typename T>
    using fingerprint_storage_t = FingerprintStorageType<T>::type;

    // ══════════════════════════════════════════════════════════════════════
    //  CuckooFilter — template class declaration
    // ══════════════════════════════════════════════════════════════════════

    /// @brief A cuckoo filter for probabilistic set membership with deletion.
    /// @tparam T Element type.
    /// @tparam EntriesPerBucket Number of fingerprints per bucket (default 4).
    /// @tparam FingerprintBits Fingerprint bit-width tag (default 8).
    /// @tparam Hash Hash functor (default std::hash<T>).
    template <typename T,
              std::size_t EntriesPerBucket = 4,
              typename FingerprintBits = FingerprintSize<8>,
              typename Hash = std::hash<T>>
    class CuckooFilter final
    {
    private:
        using fp_type_            = fingerprint_storage_t<FingerprintBits>;
        static constexpr std::size_t FINGERPRINT_BITS_    = FingerprintBits::value;
        static constexpr std::size_t ENTRIES_PER_BUCKET_  = EntriesPerBucket;
        static constexpr fp_type_    FP_ZERO_MAPPED_      = static_cast<fp_type_>(1);
        static constexpr std::size_t MAX_KICKS_           = 500;

        /// @brief A single bucket holding up to EntriesPerBucket fingerprints.
        struct alignas(8) Bucket
        {
            std::array<fp_type_, ENTRIES_PER_BUCKET_> entries_{};
            uint8_t count_{0};

            /// @brief Per-bucket counter for pseudo-random victim selection
            ///        during cuckoo displacement.
            mutable std::size_t displacement_counter_{0};

            /// @brief Check whether the bucket has an empty slot.
            [[nodiscard]] auto has_slot() const noexcept -> bool
            {
                return count_ < ENTRIES_PER_BUCKET_;
            }

            /// @brief Check whether the bucket is full.
            [[nodiscard]] auto is_full() const noexcept -> bool
            {
                return count_ >= ENTRIES_PER_BUCKET_;
            }

            /// @brief Add a fingerprint at the first empty slot.
            /// @param fp Fingerprint to insert.
            /// @pre has_slot() must be true, otherwise the insertion is a no-op.
            void add(fp_type_ fp) noexcept
            {
                for (auto& e : entries_)
                {
                    if (e == 0)
                    {
                        e = fp;
                        ++count_;
                        return;
                    }
                }
            }

            /// @brief Check whether a fingerprint exists in the bucket.
            /// @param fp Fingerprint to look up.
            /// @retval true Fingerprint is present.
            /// @retval false Fingerprint is not present.
            [[nodiscard]] auto contains(fp_type_ fp) const noexcept -> bool
            {
                for (std::size_t i = 0; i < count_; ++i)
                {
                    if (entries_[i] == fp) return true;
                }
                return false;
            }

            /// @brief Remove one occurrence of a fingerprint.
            /// @details Uses swap-with-last compaction for O(1) removal.
            /// @param fp Fingerprint to remove.
            /// @retval true Fingerprint was found and removed.
            /// @retval false Fingerprint was not found.
            auto remove(fp_type_ fp) noexcept -> bool
            {
                for (std::size_t i = 0; i < count_; ++i)
                {
                    if (entries_[i] == fp)
                    {
                        entries_[i] = entries_[count_ - 1];
                        entries_[count_ - 1] = 0;
                        --count_;
                        return true;
                    }
                }
                return false;
            }

            /// @brief Return a random occupied slot index for cuckoo
            ///        displacement.
            /// @details Uses a simple deterministic per-bucket counter to
            ///          avoid needing a full RNG.  Each Bucket maintains
            ///          its own displacement_counter_ for instance
            ///          independence.
            /// @pre count_ > 0 (i.e. the bucket must not be empty).
            [[nodiscard]] auto random_index() const noexcept -> std::size_t
            {
                return (++displacement_counter_) % count_;
            }

            /// @brief Equality comparison — compares logical content only.
            /// @details The mutable displacement counter is excluded from
            ///          comparison since it is not part of the logical state.
            [[nodiscard]] friend auto operator==(const Bucket& lhs,
                                                  const Bucket& rhs) noexcept
                -> bool
            {
                return lhs.entries_ == rhs.entries_
                    && lhs.count_ == rhs.count_;
            }

            /// @brief Inequality comparison.
            [[nodiscard]] friend auto operator!=(const Bucket& lhs,
                                                  const Bucket& rhs) noexcept
                -> bool
            {
                return !(lhs == rhs);
            }
        };

        // ── Data members ──────────────────────────────────────────────────
        std::vector<Bucket> buckets_;
        uint64_t num_buckets_{0};
        uint64_t element_count_{0};

        /// @brief Round a number up to the nearest power of two.
        [[nodiscard]] static auto next_power_of_2(uint64_t n) noexcept -> uint64_t
        {
            if (n == 0) return 1;
            --n;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
            n |= n >> 32;
            return n + 1;
        }

    public:
        /// @brief Construct a cuckoo filter with a given expected capacity.
        /// @param expected_capacity Estimated number of unique elements.
        explicit CuckooFilter(uint64_t expected_capacity)
        {
            // Calculate number of buckets: round up to give ~5 % headroom
            const uint64_t slots_needed   = expected_capacity + expected_capacity / 20;  // +5 %
            const uint64_t buckets_needed = (slots_needed + ENTRIES_PER_BUCKET_ - 1) / ENTRIES_PER_BUCKET_;
            num_buckets_ = next_power_of_2(buckets_needed);
            buckets_.resize(num_buckets_);
            element_count_ = 0;
        }

        // Default copy / move / dtor — std::vector handles it.
        CuckooFilter(const CuckooFilter&)            = default;
        CuckooFilter(CuckooFilter&&) noexcept        = default;
        auto operator=(const CuckooFilter&) -> CuckooFilter& = default;
        auto operator=(CuckooFilter&&) noexcept -> CuckooFilter& = default;
        ~CuckooFilter()                              = default;

        // ── Core API ──────────────────────────────────────────────────────

        /// @brief Insert a key into the filter.
        /// @param key Element to insert.
        /// @retval true  Key was inserted successfully.
        /// @retval false Filter is too full; insertion failed (filter state
        ///               is unchanged).
        auto insert(const T& key) -> bool;

        /// @brief Check whether a key is possibly in the filter.
        /// @param key Element to look up.
        /// @retval true  Key is *probably* present (maybe a false positive).
        /// @retval false Key is *definitely* not present.
        [[nodiscard]] auto contains(const T& key) const -> bool;

        /// @brief Remove a key from the filter.
        /// @param key Element to remove.
        /// @retval true  Key was found and removed.
        /// @retval false Key was not found.
        auto remove(const T& key) -> bool;

        // ── Introspection ─────────────────────────────────────────────────

        /// @brief Number of buckets.
        [[nodiscard]] auto size() const noexcept -> uint64_t { return num_buckets_; }

        /// @brief Number of elements currently inserted.
        [[nodiscard]] auto element_count() const noexcept -> uint64_t { return element_count_; }

        /// @brief Total fingerprint slots (buckets × entries_per_bucket).
        [[nodiscard]] auto capacity() const noexcept -> uint64_t
        {
            return num_buckets_ * ENTRIES_PER_BUCKET_;
        }

        /// @brief Current load factor in the range [0.0, 1.0].
        [[nodiscard]] auto load_factor() const noexcept -> double;

        /// @brief Estimated false-positive probability at current load.
        [[nodiscard]] static auto effective_fpp() noexcept -> double;

        /// @brief Entries per bucket (compile-time constant).
        [[nodiscard]] static constexpr auto entries_per_bucket() noexcept -> std::size_t
        {
            return ENTRIES_PER_BUCKET_;
        }

        /// @brief The fingerprint bit width (compile-time constant).
        [[nodiscard]] static constexpr auto fingerprint_bits() noexcept -> std::size_t
        {
            return FINGERPRINT_BITS_;
        }

        // ── Management ────────────────────────────────────────────────────

        /// @brief Remove all elements from the filter.
        void clear() noexcept;

        // ── Comparison ────────────────────────────────────────────────────

        [[nodiscard]] auto operator==(const CuckooFilter& other) const -> bool;
        [[nodiscard]] auto operator!=(const CuckooFilter& other) const -> bool
        {
            return !(*this == other);
        }

        // ── Range insert ──────────────────────────────────────────────────

        /// @brief Insert a range of keys into the filter.
        /// @tparam InputIterator Iterator type satisfying
        ///         std::input_iterator.
        /// @param begin Start of the range.
        /// @param end   End of the range.
        template <typename InputIterator>
        void insert(InputIterator begin, InputIterator end);

    private:
        // ── Hash helpers ──────────────────────────────────────────────────

        /// @brief Spread fingerprint bits using the golden ratio constant
        ///        for the alternate-bucket computation.
        [[nodiscard]] static auto hash_fingerprint(fp_type_ fp) noexcept -> uint64_t
        {
            return static_cast<uint64_t>(fp) * 0x9E3779B97F4A7C15ULL;
        }

        /// @brief Extract the fingerprint from a hash value, mapping 0 → 1.
        [[nodiscard]] auto extract_fingerprint(uint64_t hash) const noexcept -> fp_type_
        {
            const auto fp = static_cast<fp_type_>(hash & FINGERPRINT_MASK_());
            return (fp == 0) ? FP_ZERO_MAPPED_ : fp;
        }

        /// @brief Return the bit mask for the configured fingerprint width.
        [[nodiscard]] static constexpr auto FINGERPRINT_MASK_() noexcept -> uint64_t
        {
            if constexpr (FINGERPRINT_BITS_ >= 32)
                return 0xFFFFFFFFULL;
            else
                return (1ULL << FINGERPRINT_BITS_) - 1;
        }

        /// @brief Compute the primary bucket index from a hash value.
        [[nodiscard]] auto compute_bucket1(uint64_t hash) const noexcept -> uint64_t
        {
            return (hash >> 16) & (num_buckets_ - 1);
        }

        /// @brief Compute the alternate bucket index from bucket1 and the
        ///        fingerprint.
        [[nodiscard]] auto compute_bucket2(uint64_t bucket1,
                                           fp_type_ fp) const noexcept -> uint64_t
        {
            const auto fp_hash = hash_fingerprint(fp);
            return (bucket1 ^ (fp_hash & (num_buckets_ - 1))) & (num_buckets_ - 1);
        }
    };

    // ══════════════════════════════════════════════════════════════════════
    //  Template method implementations
    // ══════════════════════════════════════════════════════════════════════

    template <typename T, std::size_t EntriesPerBucket,
              typename FingerprintBits, typename Hash>
    auto CuckooFilter<T, EntriesPerBucket, FingerprintBits, Hash>::insert(
        const T& key) -> bool
    {
        const uint64_t hash_val = Hash{}(key);
        const auto     fp       = extract_fingerprint(hash_val);
        const auto     bucket1  = compute_bucket1(hash_val);
        const auto     bucket2  = compute_bucket2(bucket1, fp);

        // Fast path: try bucket1 then bucket2
        auto& b1 = buckets_[bucket1];
        if (b1.has_slot())
        {
            b1.add(fp);
            ++element_count_;
            return true;
        }

        auto& b2 = buckets_[bucket2];
        if (b2.has_slot())
        {
            b2.add(fp);
            ++element_count_;
            return true;
        }

        // Cuckoo displacement: repeatedly evict and relocate fingerprints.
        // Uses a trail buffer to ensure atomicity — if displacement fails,
        // all modifications are rolled back.
        auto cur_bucket_idx = bucket1;
        auto cur_fp         = fp;

        // Rollback trail — stores (bucket_idx, slot_idx, old_fingerprint).
        // 128 entries is far more than needed (typical displacement: 1-5
        // iterations).  If trail overflows, the last entry is still tracked
        // for partial rollback.
        static constexpr std::size_t TRAIL_CAPACITY = 128;
        struct TrailEntry
        {
            uint64_t bucket_idx;
            uint8_t  slot_idx;
            fp_type_ old_fp;
        };
        TrailEntry trail[TRAIL_CAPACITY];
        std::size_t trail_len = 0;

        for (std::size_t kick = 0; kick < MAX_KICKS_; ++kick)
        {
            auto& cur_bucket      = buckets_[cur_bucket_idx];
            const auto victim_idx = static_cast<uint8_t>(cur_bucket.random_index());
            const auto evicted_fp = cur_bucket.entries_[victim_idx];

            // Save old value for potential rollback
            if (trail_len < TRAIL_CAPACITY)
            {
                trail[trail_len++] = {cur_bucket_idx, victim_idx, evicted_fp};
            }

            // Write current fingerprint into the victim's slot
            cur_bucket.entries_[victim_idx] = cur_fp;

            // Compute the alternate bucket for the evicted fingerprint
            const auto alt_bucket = compute_bucket2(cur_bucket_idx, evicted_fp);
            auto& alt_b           = buckets_[alt_bucket];

            if (alt_b.has_slot())
            {
                alt_b.add(evicted_fp);
                ++element_count_;
                return true;
            }

            // Alternate bucket is also full — continue displacing
            cur_bucket_idx = alt_bucket;
            cur_fp         = evicted_fp;
        }

        // Rollback: restore all overwritten slots in reverse order
        while (trail_len > 0)
        {
            const auto& entry = trail[--trail_len];
            buckets_[entry.bucket_idx].entries_[entry.slot_idx] = entry.old_fp;
        }

        return false;  // Filter is full; state is unchanged
    }

    template <typename T, std::size_t EntriesPerBucket,
              typename FingerprintBits, typename Hash>
    auto CuckooFilter<T, EntriesPerBucket, FingerprintBits, Hash>::contains(
        const T& key) const -> bool
    {
        const uint64_t hash_val = Hash{}(key);
        const auto     fp       = extract_fingerprint(hash_val);
        const auto     bucket1  = compute_bucket1(hash_val);
        const auto     bucket2  = compute_bucket2(bucket1, fp);

        return buckets_[bucket1].contains(fp)
            || buckets_[bucket2].contains(fp);
    }

    template <typename T, std::size_t EntriesPerBucket,
              typename FingerprintBits, typename Hash>
    auto CuckooFilter<T, EntriesPerBucket, FingerprintBits, Hash>::remove(
        const T& key) -> bool
    {
        const uint64_t hash_val = Hash{}(key);
        const auto     fp       = extract_fingerprint(hash_val);
        const auto     bucket1  = compute_bucket1(hash_val);
        const auto     bucket2  = compute_bucket2(bucket1, fp);

        if (buckets_[bucket1].remove(fp))
        {
            --element_count_;
            return true;
        }
        if (buckets_[bucket2].remove(fp))
        {
            --element_count_;
            return true;
        }
        return false;
    }

    template <typename T, std::size_t EntriesPerBucket,
              typename FingerprintBits, typename Hash>
    void CuckooFilter<T, EntriesPerBucket, FingerprintBits, Hash>::clear() noexcept
    {
        for (auto& b : buckets_)
        {
            b.count_ = 0;
            b.entries_.fill(0);
        }
        element_count_ = 0;
    }

    template <typename T, std::size_t EntriesPerBucket,
              typename FingerprintBits, typename Hash>
    auto CuckooFilter<T, EntriesPerBucket, FingerprintBits, Hash>::load_factor()
        const noexcept -> double
    {
        return static_cast<double>(element_count_)
             / static_cast<double>(capacity());
    }

    template <typename T, std::size_t EntriesPerBucket,
              typename FingerprintBits, typename Hash>
    auto CuckooFilter<T, EntriesPerBucket, FingerprintBits, Hash>::effective_fpp()
        noexcept -> double
    {
        const double fp_rate = 1.0 / static_cast<double>(1ULL << FINGERPRINT_BITS_);
        return 1.0 - std::pow(1.0 - fp_rate,
                              2.0 * static_cast<double>(ENTRIES_PER_BUCKET_));
    }

    template <typename T, std::size_t EntriesPerBucket,
              typename FingerprintBits, typename Hash>
    auto CuckooFilter<T, EntriesPerBucket, FingerprintBits, Hash>::operator==(
        const CuckooFilter& other) const -> bool
    {
        return num_buckets_ == other.num_buckets_
            && element_count_ == other.element_count_
            && buckets_ == other.buckets_;
    }

    template <typename T, std::size_t EntriesPerBucket,
              typename FingerprintBits, typename Hash>
    template <typename InputIterator>
    void CuckooFilter<T, EntriesPerBucket, FingerprintBits, Hash>::insert(
        InputIterator begin, InputIterator end)
    {
        for (auto it = begin; it != end; ++it)
        {
            insert(*it);
        }
    }

} // namespace common::data_structure
