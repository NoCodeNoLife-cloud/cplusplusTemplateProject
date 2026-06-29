/**
 * @file MinHash.hpp
 * @brief MinHash probabilistic Jaccard similarity estimator
 * @details MinHash is a probabilistic data structure for estimating the
 *          Jaccard similarity between sets. Uses a fixed-size signature
 *          array of k minimum hashes produced by k independent hash
 *          functions. Insert and jaccard are O(k) with space O(k). The
 *          estimate is unbiased with standard error approximately 1/sqrt(k).
 *
 * Reference: Broder, "On the resemblance and containment of documents"
 *            (1997).
 */

#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstdint>
#include <functional>
#include <limits>
#include <ranges>
#include <span>
#include <string_view>
#include <type_traits>

namespace cppforge::data_structure::probabilistic
{
    /// @brief Default hash functor for MinHash.
    ///
    /// Uses std::hash<T> and mixes the result with a seed via the
    /// MurmurHash3 fmix64 finaliser to produce independent hash values
    /// for each signature slot.
    struct MinHashDefaultHash
    {
        /// @brief Computes a seed-mixed hash for an item.
        /// @tparam T The item type to hash.
        /// @param item The item to hash.
        /// @param seed The slot-specific seed to mix in.
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

    /// @brief Concept for a hash functor usable with MinHash.
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

    /// @brief MinHash probabilistic Jaccard similarity estimator.
    ///
    /// @tparam SignatureSize Number of minimum-hash slots (k). Larger values
    ///         reduce the standard error (~1/sqrt(k)). Default: 128.
    /// @tparam Hash Hash functor type. Must satisfy HashFunctor<Hash, T> for
    ///         the item types used with insert().
    ///
    /// @par Algorithm
    /// Each item is hashed SignatureSize times with independent seeds derived
    /// from the golden ratio (0x9E3779B97F4A7C15). For each slot the minimum
    /// hash value seen so far is retained. The Jaccard similarity between two
    /// sets is estimated as the fraction of slots that contain identical values.
    ///
    /// @par Thread Safety
    /// This class is **not** thread-safe. External synchronization is required
    /// for concurrent access.
    ///
    /// @par Memory
    /// SignatureSize * sizeof(uint64_t) plus class overhead.
    /// Default (SignatureSize=128): ~1 KB.
    ///
    /// @par Reference
    /// Broder, "On the resemblance and containment of documents" (1997).
    ///
    /// @par Usage Example
    /// @code
    /// MinHash<> mh1, mh2;
    /// mh1.insert("apple");
    /// mh1.insert("banana");
    /// mh2.insert("banana");
    /// mh2.insert("cherry");
    /// double sim = mh1.jaccard(mh2);  // ~0.33
    /// @endcode
    template <size_t SignatureSize = 128, typename Hash = MinHashDefaultHash>
    class MinHash final
    {
        static_assert(SignatureSize > 0, "SignatureSize must be greater than 0");

    public:
        // ── Construction ──

        /// @brief Constructs an empty MinHash with all signature slots set to
        ///        UINT64_MAX.
        MinHash()
        {
            std::ranges::fill(signature_, std::numeric_limits<uint64_t>::max());
        }

        ~MinHash() = default;

        MinHash(const MinHash&) = default;
        auto operator=(const MinHash&) -> MinHash& = default;

        MinHash(MinHash&&) noexcept = default;
        auto operator=(MinHash&&) noexcept -> MinHash& = default;

        // ── Operations ──

        /// @brief Inserts an item into the sketch.
        ///
        /// For each of the SignatureSize hash functions, computes the hash of
        /// the item mixed with the slot-specific seed and retains the minimum.
        /// @tparam T The item type (must satisfy HashFunctor<Hash, T>).
        /// @param item The item to insert.
        template <typename T>
            requires HashFunctor<Hash, T>
        void insert(const T& item)
        {
            for (size_t i = 0; i < SignatureSize; ++i)
            {
                const auto seed = static_cast<uint64_t>(i) * 0x9E3779B97F4A7C15ULL;
                const uint64_t h = hash_(item, seed);
                if (h < signature_[i])
                {
                    signature_[i] = h;
                }
            }
        }

        /// @brief Estimates the Jaccard similarity between this sketch and
        ///        another.
        ///
        /// Computed as the fraction of signature slots where both sketches
        /// store identical minimum-hash values. Returns 0.0 when both sketches
        /// are empty (all slots at UINT64_MAX).
        /// @param other The other MinHash (must have the same SignatureSize).
        /// @return Jaccard similarity in [0.0, 1.0].
        [[nodiscard]] auto jaccard(const MinHash& other) const noexcept -> double
        {
            size_t equal = 0;
            for (size_t i = 0; i < SignatureSize; ++i)
            {
                if (signature_[i] == other.signature_[i])
                {
                    ++equal;
                }
            }

            // Both empty => similarity is 0.0
            if (equal == SignatureSize)
            {
                constexpr auto maxHash = std::numeric_limits<uint64_t>::max();
                if (signature_[0] == maxHash)
                {
                    return 0.0;
                }
            }

            return static_cast<double>(equal) /
                   static_cast<double>(SignatureSize);
        }

        /// @brief Merges another MinHash into this one.
        ///
        /// Performs element-wise min of signature slots. After merging, this
        /// sketch represents the union of the two original sets.
        /// @param other The other MinHash to merge from.
        void merge(const MinHash& other) noexcept
        {
            for (size_t i = 0; i < SignatureSize; ++i)
            {
                if (other.signature_[i] < signature_[i])
                {
                    signature_[i] = other.signature_[i];
                }
            }
        }

        /// @brief Resets all signature slots to UINT64_MAX.
        void clear() noexcept
        {
            std::ranges::fill(signature_, std::numeric_limits<uint64_t>::max());
        }

        // ── Queries ──

        /// @brief Checks whether the sketch is empty (no items inserted).
        /// @return true if all signature slots are UINT64_MAX.
        [[nodiscard]] auto isEmpty() const noexcept -> bool
        {
            constexpr auto maxHash = std::numeric_limits<uint64_t>::max();
            return std::ranges::all_of(
                signature_,
                [maxHash](uint64_t v) noexcept { return v == maxHash; });
        }

        /// @brief Returns a read-only view of the signature array.
        /// @return span<const uint64_t> over the internal signature storage.
        [[nodiscard]] auto signature() const noexcept -> std::span<const uint64_t>
        {
            return std::span<const uint64_t>(signature_.data(), signature_.size());
        }

        /// @brief Returns the approximate memory usage in bytes.
        /// @return sizeof(*this) �?the signature array is embedded inline.
        [[nodiscard]] auto memoryUsage() const noexcept -> uint64_t
        {
            return sizeof(*this);
        }

        /// @brief Returns the signature size (k), i.e. the number of minimum-hash
        ///        slots.
        /// @return SignatureSize template parameter.
        [[nodiscard]] static constexpr auto signatureSize() noexcept -> size_t
        {
            return SignatureSize;
        }

    private:
        /// @brief Fixed-size array of minimum hash values, one per independent
        ///        hash function.
        std::array<uint64_t, SignatureSize> signature_;

        /// @brief Hash functor instance.
        ///        Marked [[no_unique_address]] to elide storage for stateless
        ///        functors (e.g. MinHashDefaultHash).
        [[no_unique_address]] Hash hash_;
    };

} // namespace cppforge::data_structure::probabilistic
