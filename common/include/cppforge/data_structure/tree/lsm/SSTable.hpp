/**
 * @file SSTable.hpp
 * @brief In-memory Sorted String Table (SSTable) for LSM-Tree
 * @details An SSTable is an immutable, sorted collection of key-value pairs
 *          with an associated Bloom filter for fast negative lookups.  Each
 *          SSTable is identified by a monotonically increasing sequence
 *          number, tracks its minimum and maximum key for range-based
 *          filtering, and supports binary-search lookup.
 *
 *          Entries can optionally be tombstones (deletion markers) that
 *          shadow older values for the same key.  Tombstone entries are
 *          stored in the sorted data vector alongside regular values and
 *          are subject to the same Bloom-filter and binary-search lookups.
 *
 * @par Immutability
 * Once constructed, an SSTable is never modified.  This simplifies
 * concurrent reads and makes compaction the only mutation path.
 *
 * @par Thread Safety
 * Fully thread-safe for concurrent reads because all data members are
 * const after construction.
 *
 * @tparam Key   Key type (must be LessThanComparable).
 * @tparam Value Value type (must be default-constructible, copyable and
 *               movable).
 */

#pragma once

#include <cppforge/data_structure/filter/BloomFilter.hpp>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <optional>
#include <utility>
#include <vector>

namespace cppforge::data_structure::tree::lsm
{
    /// @brief A monotonically increasing SSTable identifier.
    using SSTableId = uint64_t;

    /// @brief A single entry in an SSTable, possibly a tombstone.
    ///
    /// @tparam Key   Key type.
    /// @tparam Value Value type.
    template <typename Key, typename Value>
    struct SSTableEntry final
    {
        Key key; ///< The entry key.
        Value value; ///< The associated value (meaningful only when
                             ///< @p tombstone is false).
        bool tombstone = false; ///< If true, @p value is ignored and this
                             ///< entry is a deletion marker.
    };

    /// @brief In-memory Sorted String Table.
    ///
    /// Stores a sorted vector of entries plus a Bloom filter that covers
    /// all keys.  Lookups first probe the Bloom filter then fall back to
    /// binary search on the data vector.
    template <typename Key, typename Value>
    class SSTable final
    {
    public:
        /// @brief The entry type stored in this SSTable.
        using Entry = SSTableEntry<Key, Value>;

        /// @brief Read-only view of the underlying sorted entries.
        using DataView = const std::vector<Entry>&;

        // ©¤©¤ Construction ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

        /// @brief Constructs an SSTable from a sorted sequence of entries.
        ///
        /// @param[in] id       Unique identifier (monotonically increasing).
        /// @param[in] entries  Sorted entries.  The caller must guarantee
        ///                      the vector is sorted by key.
        /// @param[in] fpp      Target Bloom filter false-positive
        ///                      probability (default 0.01 ¡Ö 1 %).
        ///
        /// @pre @p entries must be sorted in ascending key order.
        /// @post The SSTable is immutable for its lifetime.
        SSTable(SSTableId id, std::vector<Entry> entries, double fpp = 0.01) : id_(id), entries_(std::move(entries)), bloom_(buildBloom(entries_, fpp))
        {
            if (!entries_.empty())
            {
                minKey_ = entries_.front().key;
                maxKey_ = entries_.back().key;
            }
        }

        // ©¤©¤ Non-copyable / movable ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

        SSTable(const SSTable&) = delete;
        auto operator=(const SSTable&) -> SSTable& = delete;

        SSTable(SSTable&&) = default;
        auto operator=(SSTable&&) -> SSTable& = default;

        // ©¤©¤ Accessors ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

        /// @brief Returns the unique SSTable identifier.
        [[nodiscard]] auto id() const noexcept -> SSTableId
        {
            return id_;
        }

        /// @brief Number of entries in this SSTable.
        [[nodiscard]] auto size() const noexcept -> std::size_t
        {
            return entries_.size();
        }

        /// @brief True if the SSTable contains no entries.
        [[nodiscard]] auto empty() const noexcept -> bool
        {
            return entries_.empty();
        }

        /// @brief The smallest key stored (only valid when !empty()).
        [[nodiscard]] auto minKey() const -> const Key&
        {
            return minKey_;
        }

        /// @brief The largest key stored (only valid when !empty()).
        [[nodiscard]] auto maxKey() const -> const Key&
        {
            return maxKey_;
        }

        /// @brief Returns a const reference to the entry vector.
        [[nodiscard]] auto data() const noexcept -> DataView
        {
            return entries_;
        }

        // ©¤©¤ Lookups ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

        /// @brief Looks up a key in this SSTable.
        ///
        /// First probes the Bloom filter.  If the filter reports "not
        /// present", the key is definitely absent.  Otherwise, performs
        /// a binary search over the sorted entries.
        ///
        /// @param[in] key The key to look up.
        /// @return A pair of (value, isTombstone) if the key exists, or
        ///         @c std::nullopt if the key is not present.  When
        ///         @p isTombstone is true, the value should be ignored
        ///         (the key has been deleted).
        [[nodiscard]] auto get(const Key& key) const -> std::optional<std::pair<Value, bool>>
        {
            // Quick negative via Bloom filter
            if (!bloom_.contains(key))
            {
                return std::nullopt;
            }

            // Range check
            if (empty() || key < minKey_ || maxKey_ < key)
            {
                return std::nullopt;
            }

            // Binary search
            auto it = lowerBound(key);
            if (it != entries_.end() && it->key == key)
            {
                return std::make_pair(it->value, it->tombstone);
            }
            return std::nullopt;
        }

        /// @brief Checks whether a key exists in this SSTable with a
        ///        non-tombstone value.
        ///
        /// @param[in] key The key to check.
        /// @return true if the key exists and is not a tombstone.
        [[nodiscard]] auto contains(const Key& key) const -> bool
        {
            auto result = get(key);
            return result.has_value() && !result->second;
        }

        /// @brief Returns an iterator to the first entry whose key is
        ///        not less than @p key (std::lower_bound semantics).
        ///
        /// @param[in] key The search key.
        /// @return Iterator into the internal entries vector.
        [[nodiscard]] auto lowerBound(const Key& key) const -> std::vector<Entry>::const_iterator
        {
            return std::lower_bound(entries_.begin(), entries_.end(), key, [](const Entry& e, const Key& k) { return e.key < k; });
        }

        /// @brief Returns an iterator to the first entry.
        [[nodiscard]] auto begin() const noexcept -> std::vector<Entry>::const_iterator
        {
            return entries_.begin();
        }

        /// @brief Returns an end iterator.
        [[nodiscard]] auto end() const noexcept -> std::vector<Entry>::const_iterator
        {
            return entries_.end();
        }

    private:
        // ©¤©¤ Data ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

        SSTableId id_;
        std::vector<Entry> entries_;
        BloomFilter bloom_;
        Key minKey_{};
        Key maxKey_{};

        // ©¤©¤ Helpers ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

        /// @brief Builds a Bloom filter from the entry keys.
        ///
        /// Computes optimal parameters based on the number of entries and
        /// the desired false-positive probability, then inserts every key.
        ///
        /// @param[in] entries  Sorted key-value pairs.
        /// @param[in] fpp      Desired false-positive probability.
        /// @return Configured BloomFilter covering all keys.
        static auto buildBloom(const std::vector<Entry>& entries, double fpp) -> BloomFilter
        {
            BloomParameters params;
            params.projected_element_count = entries.size();
            params.false_positive_probability = fpp;
            if (!params.compute_optimal_parameters())
            {
                // Fallback: use parameters as-is if optimization fails
                // (should not happen with valid inputs).
            }

            BloomFilter filter(params);
            for (const auto& e : entries)
            {
                filter.insert(e.key);
            }
            return filter;
        }
    };
} // namespace cppforge::data_structure::tree::lsm
