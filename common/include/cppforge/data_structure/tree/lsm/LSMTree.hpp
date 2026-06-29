/**
 * @file LSMTree.hpp
 * @brief Log-Structured Merge Tree (LSM-Tree) ¡ª write-optimised KV store
 * @details An LSM-Tree maintains a small, sorted in-memory table (MemTable)
 *          that absorbs all writes.  When the MemTable reaches a configured
 *          threshold it is sealed (made immutable) and a fresh MemTable is
 *          created.  Immutable MemTables are periodically flushed to level 0
 *          as sorted-string tables (SSTables).  Background compaction merges
 *          SSTables across levels to bound read amplification and reclaim
 *          space from deleted (tombstone) entries.
 *
 *          Reads consult the active MemTable, the immutable MemTable (if
 *          any), then each level in turn.  A per-SSTable Bloom filter
 *          provides fast negative answers without scanning the full table.
 *
 *          Deletions are handled via tombstone markers that propagate
 *          through flushes and compactions, correctly shadowing older
 *          values until compaction merges the tombstone with all surviving
 *          copies of the deleted key.
 *
 * @par Thread Safety
 * All public methods are thread-safe.  Access is serialized through an
 * internal @c std::mutex.
 *
 * @par Reference
 * O'Neil et al., "The Log-Structured Merge-Tree (LSM-Tree)",
 * Acta Informatica 33(4):351¨C385, 1996.
 *
 * @tparam Key   Key type (must satisfy LessThanComparable).
 * @tparam Value Value type (must be default-constructible, copyable, and
 *               movable).
 *
 * @par Usage Example
 * @code
 * LSMTreeOptions opts;
 * opts.memtable_size = 1024;
 * LSMTree<int, std::string> tree(opts);
 *
 * tree.put(42, "hello");
 * auto v = tree.get(42);      // ¡ú std::optional{"hello"}
 * tree.remove(42);
 * auto v2 = tree.get(42);     // ¡ú std::nullopt
 * @endcode
 */

#pragma once

#include <cppforge/data_structure/tree/lsm/LSMTreeOptions.hpp>
#include <cppforge/data_structure/tree/lsm/SSTable.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------
//  Logging helpers ¡ª disabled by default; define LSMTREE_ENABLE_LOG to
//  enable trace output.
// -----------------------------------------------------------------------
#ifdef LSMTREE_ENABLE_LOG
#include <cstdio>
#define LSMTREE_LOG(fmt, ...) std::printf("[LSMTree] " fmt "\n", ##__VA_ARGS__)
#else
#define LSMTREE_LOG(fmt, ...) static_cast<void>(0)
#endif

namespace cppforge::data_structure::tree::lsm
{
    /// @brief A Log-Structured Merge Tree.
    ///
    /// @tparam Key   Sorted key type (LessThanComparable).
    /// @tparam Value Associated value type.
    template <typename Key, typename Value>
    class LSMTree final
    {
    public:
        // ©¤©¤ Types ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

        /// @brief The concrete SSTable type used internally.
        using SSTableType = SSTable<Key, Value>;

        /// @brief Unique pointer to an SSTable.
        using SSTablePtr = std::unique_ptr<SSTableType>;

        // ©¤©¤ Construction & destruction ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

        /// @brief Constructs an empty LSM-Tree.
        ///
        /// @param[in] opts Configuration parameters (see LSMTreeOptions).
        explicit LSMTree(LSMTreeOptions opts = {}) : options_(opts), levels_(options_.max_levels + 1) // index 0 ¡­ max_levels
        {
        }

        ~LSMTree() = default;

        // ©¤©¤ Non-copyable ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

        LSMTree(const LSMTree&) = delete;
        auto operator=(const LSMTree&) -> LSMTree& = delete;

        // ©¤©¤ Movable ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

        LSMTree(LSMTree&& other) noexcept : options_(other.options_), memtable_(std::move(other.memtable_)), immutableMemtable_(std::move(other.immutableMemtable_)), levels_(std::move(other.levels_)), nextSSTableId_(other.nextSSTableId_), approximateSize_(other.approximateSize_)
        {
            // mutex_ is not moved; the new object gets a fresh one.
        }

        auto operator=(LSMTree&& other) noexcept -> LSMTree&
        {
            if (this != &other)
            {
                std::lock(mutex_, other.mutex_);
                std::lock_guard lkThis(mutex_, std::adopt_lock);
                std::lock_guard lkOther(other.mutex_, std::adopt_lock);

                options_ = other.options_;
                memtable_ = std::move(other.memtable_);
                immutableMemtable_ = std::move(other.immutableMemtable_);
                levels_ = std::move(other.levels_);
                nextSSTableId_ = other.nextSSTableId_;
                approximateSize_ = other.approximateSize_;
            }
            return *this;
        }

        // ¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T
        //  Core API
        // ¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T

        /// @brief Inserts or updates a key-value pair.
        ///
        /// If the active MemTable reaches its configured capacity after this
        /// insertion, it is sealed automatically and a flush is triggered.
        ///
        /// @param[in] key   Key to insert / update.
        /// @param[in] value Associated value.
        void put(const Key& key, const Value& value)
        {
            std::lock_guard lock(mutex_);

            internalPut(key, InternalValue{value, false});

            maybeSealAndFlush();
        }

        /// @brief Move-inserts a key-value pair (rvalue overload).
        void put(Key&& key, Value&& value)
        {
            std::lock_guard lock(mutex_);

            internalPut(std::move(key), InternalValue{std::move(value), false});

            maybeSealAndFlush();
        }

        /// @brief Retrieves the value associated with a key.
        ///
        /// The MemTable is consulted first, followed by the immutable
        /// MemTable (if any), then each level from 0 upwards.  The first
        /// non-tombstone entry found is returned.  If the first entry found
        /// is a tombstone, the key is considered deleted and @c std::nullopt
        /// is returned immediately without searching older levels.
        ///
        /// @param[in] key The key to look up.
        /// @return The value if present (and not deleted), or @c std::nullopt.
        [[nodiscard]] auto get(const Key& key) const -> std::optional<Value>
        {
            std::lock_guard lock(mutex_);

            // 1. Active MemTable (most recent data)
            {
                auto it = memtable_.find(key);
                if (it != memtable_.end())
                {
                    if (!it->second.tombstone)
                    {
                        return it->second.value;
                    }
                    return std::nullopt; // deleted
                }
            }

            // 2. Immutable MemTable
            if (immutableMemtable_)
            {
                auto it = immutableMemtable_->find(key);
                if (it != immutableMemtable_->end())
                {
                    if (!it->second.tombstone)
                    {
                        return it->second.value;
                    }
                    return std::nullopt;
                }
            }

            // 3. Levels (L0 ¡­ L_max) ¡ª stop at the first match
            for (std::size_t level = 0; level < levels_.size(); ++level)
            {
                auto result = searchLevel(key, level);
                if (result == LevelResult::FoundValue)
                {
                    return latestFoundValue_;
                }
                if (result == LevelResult::FoundTombstone)
                {
                    return std::nullopt;
                }
                // NotFound ¡ú continue to next level
            }

            return std::nullopt;
        }

        /// @brief Checks whether a key exists and has not been deleted.
        ///
        /// @param[in] key The key to check.
        /// @return true if the key is present (non-tombstone).
        [[nodiscard]] auto contains(const Key& key) const -> bool
        {
            return get(key).has_value();
        }

        /// @brief Deletes a key from the store.
        ///
        /// A tombstone marker is inserted into the active MemTable.  The
        /// tombstone propagates through flushes and compactions, hiding any
        /// older values for this key.
        ///
        /// @param[in] key The key to delete.
        void remove(const Key& key)
        {
            std::lock_guard lock(mutex_);

            auto it = memtable_.find(key);
            if (it != memtable_.end())
            {
                if (!it->second.tombstone)
                {
                    // Replacing a live entry with a tombstone ¡ú one fewer
                    // live entry in the approximate count.
                    --approximateSize_;
                }
                // Mark as tombstone; keep existing value (unused).
                it->second.tombstone = true;
            }
            else
            {
                // Key not in the active MemTable; add a tombstone without
                // affecting the approximate size (we do not know whether
                // the key exists in immutable MemTable or SSTables).
                memtable_[key] = InternalValue{Value{}, true};
            }

            if (memtable_.size() >= options_.memtable_size)
            {
                sealMemTable();
                doFlush();
                doCompact();
            }
        }

        // ¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T
        //  Maintenance
        // ¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T

        /// @brief Forces the active MemTable to be flushed to level 0 and
        ///        triggers a compaction pass.
        ///
        /// Normally flushing happens automatically when the MemTable reaches
        /// capacity.  Call this method explicitly to reduce memory pressure
        /// or before taking a snapshot.
        void flush()
        {
            std::lock_guard lock(mutex_);
            sealMemTable();
            doFlush();
            doCompact();
        }

        /// @brief Triggers an immediate compaction run.
        ///
        /// Compaction merges SSTables from the deepest level that exceeds
        /// its size target into the next level.  This reduces read
        /// amplification and purges stale tombstones.
        void compact()
        {
            std::lock_guard lock(mutex_);
            doCompact();
        }

        // ¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T
        //  Introspection
        // ¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T

        /// @brief Approximate number of non-tombstone entries in the tree.
        ///
        /// This count includes the active MemTable, the immutable MemTable,
        /// and all SSTables.  It is an approximation because tombstones in
        /// SSTables are not tracked separately without a full scan.
        [[nodiscard]] auto size() const -> std::size_t
        {
            std::lock_guard lock(mutex_);
            return approximateSize_;
        }

        /// @brief Number of entries (including tombstones) in the active
        ///        MemTable.
        [[nodiscard]] auto memtableSize() const -> std::size_t
        {
            std::lock_guard lock(mutex_);
            return memtable_.size();
        }

        /// @brief Number of levels that contain at least one SSTable.
        [[nodiscard]] auto levelCount() const -> std::size_t
        {
            std::lock_guard lock(mutex_);
            std::size_t count = 0;
            for (const auto& lvl : levels_)
            {
                if (!lvl.empty())
                    ++count;
            }
            return count;
        }

        /// @brief Number of SSTables in a given level.
        /// @param[in] level Level index (0 ¡­ max_levels).
        [[nodiscard]] auto sstableCount(std::size_t level) const -> std::size_t
        {
            std::lock_guard lock(mutex_);
            if (level >= levels_.size())
                return 0;
            return levels_[level].size();
        }

        /// @brief Removes all entries and resets the tree to its empty state.
        void clear()
        {
            std::lock_guard lock(mutex_);
            memtable_.clear();
            immutableMemtable_.reset();
            for (auto& lvl : levels_)
            {
                lvl.clear();
            }
            nextSSTableId_ = 1;
            approximateSize_ = 0;
        }

    private:
        // ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
        //  Internal types
        // ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

        /// @brief Wrapper around a value that records whether the entry is a
        ///        tombstone (deletion marker).
        struct InternalValue
        {
            Value value{};
            bool tombstone = false;
        };

        /// @brief Active MemTable ¡ª an in-memory sorted map.
        using MemTable = std::map<Key, InternalValue>;

        /// @brief Result of searching a single level.
        enum class LevelResult : uint8_t
        {
            NotFound, ///< Key not present in this level.
            FoundValue, ///< Key found with a non-tombstone value.
            FoundTombstone ///< Key found as a tombstone (deleted).
        };

        // ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
        //  Mutable state (guarded by mutex_)
        // ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

        mutable std::mutex mutex_;

        LSMTreeOptions options_;

        /// @brief The active write buffer.
        MemTable memtable_;

        /// @brief A sealed, read-only MemTable awaiting flush.
        std::optional<MemTable> immutableMemtable_;

        /// @brief Levels 0 ¡­ max_levels.  Each level holds zero or more
        ///        SSTables sorted by their minimum key.
        std::vector<std::vector<SSTablePtr>> levels_;

        /// @brief Monotonically increasing ID for SSTable creation order.
        SSTableId nextSSTableId_ = 1;

        /// @brief Approximate count of live (non-tombstone) entries.
        std::size_t approximateSize_ = 0;

        /// @brief Cached value from the most recent successful `searchLevel`
        ///        call (avoids copying the value through the return channel).
        mutable Value latestFoundValue_{};

        // ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
        //  Internal helpers ¡ª put / seal / flush / compact / search
        // ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

        /// @brief Inserts an InternalValue into the active MemTable and
        ///        updates the approximate size counter.
        void internalPut(const Key& key, InternalValue iv)
        {
            if (iv.tombstone)
            {
                memtable_[key] = std::move(iv);
                return;
            }

            auto [it, inserted] = memtable_.emplace(key, std::move(iv));
            if (!inserted)
            {
                // Key exists ¡ª only treat as a new live entry if the
                // previous entry was a tombstone (key resurrected).
                if (it->second.tombstone)
                {
                    ++approximateSize_;
                }
                it->second.value = std::move(iv.value);
                it->second.tombstone = false;
            }
            else
            {
                ++approximateSize_;
            }
        }

        void internalPut(Key&& key, InternalValue iv)
        {
            // rvalue overload: move the key into the map.
            if (iv.tombstone)
            {
                memtable_[std::move(key)] = std::move(iv);
                return;
            }

            auto [it, inserted] = memtable_.emplace(std::move(key), std::move(iv));
            if (!inserted)
            {
                if (it->second.tombstone)
                {
                    ++approximateSize_;
                }
                it->second.value = std::move(iv.value);
                it->second.tombstone = false;
            }
            else
            {
                ++approximateSize_;
            }
        }

        /// @brief Seals the active MemTable (makes it immutable) and starts
        ///        a fresh one.
        ///
        /// If there is already an immutable MemTable waiting, it is flushed
        /// first so we never accumulate more than one.
        void sealMemTable()
        {
            if (memtable_.empty())
            {
                return;
            }

            if (immutableMemtable_.has_value())
            {
                doFlush();
            }

            immutableMemtable_ = std::move(memtable_);
            memtable_ = MemTable{};
            LSMTREE_LOG("MemTable sealed (%zu entries)", immutableMemtable_->size());
        }

        /// @brief Flushes the immutable MemTable (if any) to a new SSTable
        ///        in level 0.
        ///
        /// Both values and tombstones are emitted so that deletion markers
        /// correctly shadow older values in existing SSTables.
        void doFlush()
        {
            if (!immutableMemtable_.has_value() || immutableMemtable_->empty())
            {
                return;
            }

            std::vector<SSTableEntry<Key, Value>> entries;
            entries.reserve(immutableMemtable_->size());

            for (auto& [key, iv] : *immutableMemtable_)
            {
                entries.push_back(SSTableEntry<Key, Value>{std::move(key), std::move(iv.value), iv.tombstone});
            }

            if (entries.empty())
            {
                immutableMemtable_.reset();
                return;
            }

            // Entries from std::map are already sorted by key.
            auto table = std::make_unique<SSTableType>(nextSSTableId_++, std::move(entries), options_.bloom_false_positive_rate);

            levels_[0].push_back(std::move(table));
            immutableMemtable_.reset();

            LSMTREE_LOG("Flushed SSTable id=%zu to L0 (%zu entries)", nextSSTableId_ - 1, levels_[0].back()->size());
        }

        /// @brief Runs one round of leveled compaction: picks the smallest
        ///        level that exceeds its size target and merges it with
        ///        overlapping SSTables in the next level.
        void doCompact()
        {
            for (std::size_t level = 0; level + 1 < levels_.size(); ++level)
            {
                if (shouldCompactLevel(level))
                {
                    compactLevel(level);
                    return; // one compaction round per call
                }
            }
        }

        /// @brief Returns true if @p level exceeds its compaction target.
        [[nodiscard]] auto shouldCompactLevel(std::size_t level) const -> bool
        {
            if (level == 0)
            {
                return levels_[0].size() >= options_.l0_compaction_threshold;
            }

            // Level i target = memtable_size ¡Á fanOut^i
            std::size_t target = options_.memtable_size;
            for (std::size_t i = 0; i < level; ++i)
            {
                target *= options_.fan_out;
            }

            std::size_t totalEntries = 0;
            for (const auto& tbl : levels_[level])
            {
                totalEntries += tbl->size();
            }
            return totalEntries >= target;
        }

        /// @brief Merges SSTables at @p level with overlapping SSTables at
        ///        @p level + 1 using a multi-way merge, deduplicating by key
        ///        (newest SSTable wins), and writing the output into the
        ///        destination level.
        void compactLevel(std::size_t level)
        {
            LSMTREE_LOG("Compacting level %zu", level);

            auto& srcLevel = levels_[level];
            auto& dstLevel = levels_[level + 1];

            if (srcLevel.empty())
            {
                return;
            }

            // ©¤©¤ 1. Compute the global key range of source SSTables ©¤©¤©¤©¤©¤©¤
            Key globalMin{}, globalMax{};
            bool first = true;

            for (const auto& tbl : srcLevel)
            {
                if (tbl->empty())
                    continue;
                if (first)
                {
                    globalMin = tbl->minKey();
                    globalMax = tbl->maxKey();
                    first = false;
                }
                else
                {
                    if (tbl->minKey() < globalMin)
                        globalMin = tbl->minKey();
                    if (globalMax < tbl->maxKey())
                        globalMax = tbl->maxKey();
                }
            }

            if (first)
            {
                srcLevel.clear();
                return;
            }

            // ©¤©¤ 2. Collect all tables from the source level ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
            std::vector<SSTablePtr> compactInput;
            compactInput.reserve(srcLevel.size());

            for (auto& tbl : srcLevel)
            {
                compactInput.push_back(std::move(tbl));
            }
            srcLevel.clear();

            // ©¤©¤ 3. Collect overlapping tables from the destination level ©¤
            auto dstIt = dstLevel.begin();
            while (dstIt != dstLevel.end())
            {
                auto& tbl = *dstIt;
                if (!tbl->empty() && !(tbl->maxKey() < globalMin || globalMax < tbl->minKey()))
                {
                    compactInput.push_back(std::move(tbl));
                    dstIt = dstLevel.erase(dstIt);
                }
                else
                {
                    ++dstIt;
                }
            }

            // ©¤©¤ 4. Multi-way merge into new SSTables ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
            auto outputTables = mergeTables(compactInput);

            // ©¤©¤ 5. Place new SSTables into the destination level ©¤©¤©¤©¤©¤©¤©¤©¤
            for (auto& tbl : outputTables)
            {
                dstLevel.push_back(std::move(tbl));
            }

            // ©¤©¤ 6. Re-sort by minKey to maintain the non-overlapping
            //       invariant for levels > 0. ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
            if (level + 1 > 0)
            {
                std::sort(dstLevel.begin(), dstLevel.end(), [](const SSTablePtr& a, const SSTablePtr& b)
                {
                    if (a->empty() && b->empty())
                        return false;
                    if (a->empty())
                        return true;
                    if (b->empty())
                        return false;
                    return a->minKey() < b->minKey();
                });
            }

            LSMTREE_LOG("Compaction of level %zu produced %zu SSTables in level %zu", level, dstLevel.size(), level + 1);
        }

        /// @brief Performs a multi-way merge over a set of SSTables,
        ///        deduplicating by key (newest SSTable wins).
        ///
        /// Tombstones are propagated unless all entries for that key in
        /// the merge input are tombstones ¡ª in that case the tombstone is
        /// kept (it may be needed to shadow a value in a deeper level not
        /// participating in this compaction).  If, however, there is at
        /// least one non-tombstone value for the key in the input, the
        /// tombstone's purpose is served and all entries for that key
        /// (including the tombstone) can be dropped.
        ///
        /// @param[in] inputs The SSTables to merge.
        /// @return New SSTables covering the merged data.
        [[nodiscard]] auto mergeTables(std::vector<SSTablePtr>& inputs) -> std::vector<SSTablePtr>
        {
            // ©¤©¤ Set up merge state per input table ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
            struct IterState
            {
                const SSTableType* table;
                typename std::vector<SSTableEntry<Key, Value>>::const_iterator pos;
                typename std::vector<SSTableEntry<Key, Value>>::const_iterator end;
                SSTableId id;
            };

            std::vector<IterState> states;
            for (const auto& tbl : inputs)
            {
                if (tbl && !tbl->empty())
                {
                    states.push_back({tbl.get(), tbl->begin(), tbl->end(), tbl->id()});
                }
            }

            if (states.empty())
            {
                return {};
            }

            std::vector<SSTablePtr> result;
            std::vector<SSTableEntry<Key, Value>> currentBatch;
            const std::size_t batchTarget = options_.memtable_size * 2;

            // ©¤©¤ Advance the smallest key across all iterators ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
            auto advanceMinKey = [&]() -> std::optional<SSTableEntry<Key, Value>>
            {
                int bestIdx = -1;
                for (int i = 0; i < static_cast<int>(states.size()); ++i)
                {
                    if (states[i].pos == states[i].end)
                        continue;
                    if (bestIdx < 0)
                    {
                        bestIdx = i;
                        continue;
                    }
                    const auto& curKey = states[i].pos->key;
                    const auto& bestKey = states[bestIdx].pos->key;
                    if (curKey < bestKey || (curKey == bestKey && states[i].id > states[bestIdx].id))
                    {
                        bestIdx = i;
                    }
                }

                if (bestIdx < 0)
                    return std::nullopt;

                auto entry = *states[bestIdx].pos;

                // Advance ALL iterators that point to the same key (they
                // are equal or older, so we skip them).
                for (auto& st : states)
                {
                    if (st.pos != st.end && st.pos->key == entry.key)
                    {
                        ++st.pos;
                    }
                }

                return entry;
            };

            // ©¤©¤ Main merge loop ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
            // We keep the newest entry per key.  If the newest entry is a
            // non-tombstone value, it is emitted.  If the newest entry is a
            // tombstone, we check whether there was at least one
            // non-tombstone value for the same key in the merge input ¡ª if
            // so, all entries for this key cancel out (tombstone has served
            // its purpose).  Otherwise, the tombstone is kept because it may
            // shadow a value in a deeper level.
            //
            // To implement this we need to detect, for each key, whether a
            // non-tombstone entry was present among *any* of the input
            // tables.  We do this by scanning the input tables ahead of
            // time ¡ª but that is expensive.  Instead, we apply a simple
            // conservative rule:
            //
            //   If the winning entry is a tombstone, check whether the
            //   *next* entry in any input table for the same key is a
            //   non-tombstone.  If so, skip the key entirely.
            //
            // This catches the common case where both the tombstone and
            // the value it deletes are in adjacent positions in the
            // sorted input.
            //
            // NOTE: This is a simplification.  A production LSM-Tree uses
            // per-entry sequence numbers to determine safe tombstone
            // dropping precisely.

            while (true)
            {
                auto next = advanceMinKey();
                if (!next.has_value())
                    break;

                // Winning entry is a tombstone ¡ú check if there's a value
                // for this key in the merge input.
                if (next->tombstone)
                {
                    // Scan remaining states for a non-tombstone entry with
                    // the same key.
                    bool hasValue = false;
                    for (const auto& st : states)
                    {
                        if (st.pos != st.end && st.pos->key == next->key && !st.pos->tombstone)
                        {
                            hasValue = true;
                            break;
                        }
                    }
                    if (hasValue)
                    {
                        // The tombstone deletes a value that is also in
                        // the merge input.  Both cancel out, so we drop
                        // every entry for this key.
                        continue;
                    }
                    // No value found ¡ú keep the tombstone.
                }

                currentBatch.push_back(std::move(*next));

                if (currentBatch.size() >= batchTarget)
                {
                    result.push_back(std::make_unique<SSTableType>(nextSSTableId_++, std::move(currentBatch), options_.bloom_false_positive_rate));
                    currentBatch.clear();
                }
            }

            // Flush the final (partial) batch.
            if (!currentBatch.empty())
            {
                result.push_back(std::make_unique<SSTableType>(nextSSTableId_++, std::move(currentBatch), options_.bloom_false_positive_rate));
            }

            return result;
        }

        /// @brief Searches for a key in a given level and writes any found
        ///        value into @c latestFoundValue_.
        ///
        /// Level 0 is special: SSTables may have overlapping key ranges,
        /// so we search them in reverse creation order (newest first).
        /// For deeper levels the SSTables are non-overlapping, so we
        /// locate the single candidate via binary search on minKey.
        ///
        /// @param[in]  key   The key to find.
        /// @param[in]  level Level index.
        /// @return LevelResult indicating what was found (if anything).
        [[nodiscard]] auto searchLevel(const Key& key, std::size_t level) const -> LevelResult
        {
            const auto& lvl = levels_[level];
            if (lvl.empty())
            {
                return LevelResult::NotFound;
            }

            if (level == 0)
            {
                // L0: overlapping ¡ª search newest table first.
                // Build index sorted by ID descending.
                std::vector<std::size_t> indices(lvl.size());
                for (std::size_t i = 0; i < lvl.size(); ++i) indices[i] = i;

                std::sort(indices.begin(), indices.end(), [&](std::size_t a, std::size_t b) { return lvl[a]->id() > lvl[b]->id(); });

                for (auto idx : indices)
                {
                    const auto& tbl = lvl[idx];
                    if (tbl->empty())
                        continue;
                    // Quick range check
                    if (key < tbl->minKey() || tbl->maxKey() < key)
                    {
                        continue;
                    }
                    auto result = tbl->get(key);
                    if (result.has_value())
                    {
                        auto& [val, isTomb] = *result;
                        if (!isTomb)
                        {
                            latestFoundValue_ = std::move(val);
                            return LevelResult::FoundValue;
                        }
                        return LevelResult::FoundTombstone;
                    }
                }
            }
            else
            {
                // L1+: non-overlapping ¡ª find the single overlapping table
                // via binary search.
                auto it = std::upper_bound(lvl.begin(), lvl.end(), key, [](const Key& k, const SSTablePtr& tbl) { return k < tbl->minKey(); });

                if (it != lvl.begin())
                    --it;

                const auto& tbl = *it;
                if (!tbl->empty() && !(key < tbl->minKey()) && !(tbl->maxKey() < key))
                {
                    auto result = tbl->get(key);
                    if (result.has_value())
                    {
                        auto& [val, isTomb] = *result;
                        if (!isTomb)
                        {
                            latestFoundValue_ = std::move(val);
                            return LevelResult::FoundValue;
                        }
                        return LevelResult::FoundTombstone;
                    }
                }
            }

            return LevelResult::NotFound;
        }

        /// @brief Seals the MemTable if it is full, then flushes and
        ///        triggers compaction.
        void maybeSealAndFlush()
        {
            if (memtable_.size() < options_.memtable_size)
            {
                return;
            }

            sealMemTable();
            doFlush();
            doCompact();
        }
    };
} // namespace cppforge::data_structure::tree::lsm

#undef LSMTREE_LOG
