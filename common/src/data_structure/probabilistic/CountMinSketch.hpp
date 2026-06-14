/**
 * @file CountMinSketch.hpp
 * @brief Count-Min Sketch probabilistic frequency estimator
 * @details Count-Min Sketch is a probabilistic data structure for estimating
 *          the frequency of elements in a streaming data set. Uses a 2D array
 *          of counters with Depth independent hash functions. Insert and query
 *          are O(1) with space O(Depth * Width). The estimate never
 *          underestimates the true frequency; overestimation error is bounded
 *          by (totalCount / Width) with probability 1 - 2^{-Depth}.
 *
 * Reference: Cormode & Muthukrishnan, "An Improved Data Stream Summary:
 *            The Count-Min Sketch and its Applications" (2005).
 */

#pragma once

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <functional>
#include <limits>
#include <string_view>
#include <type_traits>
#include <vector>

namespace common::data_structure::probabilistic
{
    /// @brief Default hash functor for CountMinSketch.
    ///
    /// Uses std::hash<T> and mixes the result with the row-specific seed via
    /// the MurmurHash3 fmix64 finaliser to produce independent hash values
    /// for each row.
    struct HashStd
    {
        /// @brief Computes a seed-mixed hash for an item.
        /// @tparam T The item type to hash.
        /// @param item The item to hash.
        /// @param seed The row-specific seed to mix in.
        /// @return 64-bit hash value uniformly distributed across the output
        ///         space.
        template <typename T>
        [[nodiscard]] auto operator()(const T& item, size_t seed) const -> uint64_t
        {
            uint64_t h;
            if constexpr (std::is_array_v<T>)
            {
                // Array types (e.g. C-string literals "hello") cannot be hashed
                // directly via std::hash.  Decay to string_view first.
                h = std::hash<std::string_view>{}(std::string_view(item));
            }
            else
            {
                h = std::hash<T>{}(item);
            }
            h += seed;
            // MurmurHash3 fmix64 finaliser
            h ^= h >> 33;
            h *= 0xff51afd7ed558ccdULL;
            h ^= h >> 33;
            h *= 0xc4ceb9fe1a85ec53ULL;
            h ^= h >> 33;
            return h;
        }
    };

    /// @brief Concept for a hash functor usable with CountMinSketch.
    ///
    /// A valid hash functor must be callable as:
    /// @code
    /// uint64_t operator()(const T& item, size_t seed) const
    /// @endcode
    /// @tparam H The hash functor type.
    /// @tparam T The item type to hash.
    template <typename H, typename T>
    concept HashFunctor = requires(H h, const T& item, size_t seed) {
        { h(item, seed) } -> std::convertible_to<uint64_t>;
    };

    /// @brief Count-Min Sketch probabilistic frequency estimator.
    ///
    /// @tparam Width Number of counters per hash function (default 65536 = 2^16).
    ///         Larger Width reduces overestimation error.
    /// @tparam Depth Number of independent hash functions / rows (default 5).
    ///         Larger Depth increases confidence at the cost of memory.
    /// @tparam Hash Hash functor type. Must satisfy HashFunctor<Hash, T> for
    ///         the item types used with add() and estimate().
    ///
    /// @par Algorithm
    /// Each item is hashed Depth times with independent seeds derived from the
    /// golden ratio (0x9E3779B97F4A7C15). For each hash function the counter at
    /// index (hash % Width) is incremented by the given count. The frequency
    /// estimate is the minimum of the Depth counter values. This guarantees
    /// zero underestimation; overestimation is bounded by (totalCount / Width)
    /// with probability at least 1 - 2^{-Depth}.
    ///
    /// Internally uses a flat std::vector<uint32_t> of size Depth * Width laid
    /// out as [row0, row1, ..., row{Depth-1}] for cache-friendly access.
    ///
    /// @par Thread Safety
    /// This class is **not** thread-safe. External synchronization is required
    /// for concurrent access.
    ///
    /// @par Memory
    /// Depth * Width * sizeof(uint32_t) plus class overhead.
    /// Default (Width=65536, Depth=5): ~1.3 MB.
    ///
    /// @par Reference
    /// Cormode & Muthukrishnan, "An Improved Data Stream Summary: The Count-Min
    /// Sketch and its Applications" (2005).
    ///
    /// @par Usage Example
    /// @code
    /// CountMinSketch<> cms;
    /// cms.add("apple", 3);
    /// cms.add("banana", 1);
    /// uint32_t est = cms.estimate("apple");  // est >= 3
    /// uint64_t total = cms.totalCount();     // total >= 4
    /// @endcode
    template <size_t Width = 65536, size_t Depth = 5, typename Hash = HashStd>
    class CountMinSketch final
    {
        static_assert(Width > 0, "Width must be greater than 0");
        static_assert(Depth > 0, "Depth must be greater than 0");

    public:
        // ── Construction ──

        /// @brief Constructs an empty Count-Min Sketch with all counters zeroed.
        CountMinSketch()
            : counters_(Depth * Width, 0)
        {
        }

        ~CountMinSketch() = default;

        CountMinSketch(const CountMinSketch&) = default;
        auto operator=(const CountMinSketch&) -> CountMinSketch& = default;

        CountMinSketch(CountMinSketch&&) noexcept = default;
        auto operator=(CountMinSketch&&) noexcept -> CountMinSketch& = default;

        // ── Operations ──

        /// @brief Adds an item to the sketch, incrementing its counters.
        ///
        /// For each of the Depth hash functions, computes the hash of the item
        /// mixed with the row-specific seed and increments the corresponding
        /// counter by @p count.
        /// @tparam T The item type (must satisfy HashFunctor<Hash, T>).
        /// @param item The item whose frequency to record.
        /// @param count The number of occurrences to add (default 1).
        template <typename T>
            requires HashFunctor<Hash, T>
        void add(const T& item, uint32_t count = 1)
        {
            for (size_t i = 0; i < Depth; ++i)
            {
                const size_t seed = i * 0x9E3779B97F4A7C15ULL;
                const size_t col = hash_(item, seed) % Width;
                counters_[i * Width + col] += count;
            }
        }

        /// @brief Estimates the frequency of an item.
        ///
        /// Returns the minimum counter value across all Depth hash functions.
        /// This is guaranteed to never underestimate the true frequency;
        /// overestimation is possible but bounded.
        /// @tparam T The item type (must satisfy HashFunctor<Hash, T>).
        /// @param item The item to query.
        /// @return Estimated frequency (always >= true frequency).
        template <typename T>
            requires HashFunctor<Hash, T>
        [[nodiscard]] auto estimate(const T& item) const -> uint32_t
        {
            uint32_t minVal = std::numeric_limits<uint32_t>::max();
            for (size_t i = 0; i < Depth; ++i)
            {
                const size_t seed = i * 0x9E3779B97F4A7C15ULL;
                const size_t col = hash_(item, seed) % Width;
                const auto val = counters_[i * Width + col];
                if (val < minVal)
                {
                    minVal = val;
                }
            }
            return minVal;
        }

        /// @brief Merges another Count-Min Sketch into this one.
        ///
        /// Performs element-wise addition of counters. Both sketches must have
        /// identical dimensions (same Width, Depth, and Hash types), which is
        /// guaranteed since @p other has the same template parameters.
        /// @param other The other sketch whose counters are added to this one.
        void merge(const CountMinSketch& other)
        {
            for (size_t i = 0; i < Depth * Width; ++i)
            {
                counters_[i] += other.counters_[i];
            }
        }

        /// @brief Resets all counters to zero.
        void clear()
        {
            std::ranges::fill(counters_, static_cast<uint32_t>(0));
        }

        // ── Queries ──

        /// @brief Returns the approximate total number of items added.
        ///
        /// Computed as the sum of all counters divided by Depth.
        /// @return Estimated total count of items added to the sketch.
        [[nodiscard]] auto totalCount() const -> uint64_t
        {
            uint64_t sum = 0;
            for (const auto c : counters_)
            {
                sum += static_cast<uint64_t>(c);
            }
            return sum / Depth;
        }

        /// @brief Returns the approximate memory usage.
        /// @return Memory usage in bytes (counter array + class overhead).
        [[nodiscard]] auto memoryUsage() const -> uint64_t
        {
            return sizeof(*this) + counters_.capacity() * sizeof(uint32_t);
        }

    private:
        /// @brief Flat counter matrix of size Depth * Width.
        ///        Index = row * Width + col.
        std::vector<uint32_t> counters_;

        /// @brief Hash functor instance.
        ///        Marked [[no_unique_address]] to elide storage for stateless
        ///        functors (e.g. HashStd).
        [[no_unique_address]] Hash hash_;
    };

} // namespace common::data_structure::probabilistic
