/**
 * @file LSMTreeOptions.hpp
 * @brief Configuration parameters for the LSM-Tree
 * @details Defines tunable parameters that control LSM-Tree behaviour:
 *          memtable size, level fan-out, compaction triggers, bloom filter
 *          false-positive rate, and whether optional persistence is enabled.
 *
 * @par Thread Safety
 * Options are read after construction and never modified — no synchronisation
 * is needed.  The LSMTree copies the struct at construction time.
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace common::data_structure::tree
{
    /// @brief Tunable parameters for an LSMTree instance.
    ///
    /// Defaults are chosen for a general-purpose in-memory / embedded workload.
    /// They mirror typical LevelDB / RocksDB conventions:
    ///   - MemTable flushes at 4 MB
    ///   - Each level is @p fanOut (×10) larger than the previous
    ///   - Level-0 compaction triggers when 4 or more SSTables accumulate
    ///   - Bloom filters target 1 % false-positive probability
    struct LSMTreeOptions final
    {
        // ── MemTable ────────────────────────────────────────────────────

        /// @brief Maximum number of entries in the active MemTable before it
        ///        is sealed and flushed to level 0.
        ///
        /// A higher value reduces write amplification but increases memory
        /// pressure and read‑amplification on the MemTable.
        std::size_t memtable_size = 4096;

        // ── Compaction ──────────────────────────────────────────────────

        /// @brief Number of SSTables at level 0 that trigger a compaction
        ///        into level 1.
        ///
        /// Level 0 SSTables may have overlapping key ranges because they are
        /// flushed from successive MemTables without prior merging.
        std::size_t l0_compaction_threshold = 4;

        /// @brief Maximum number of levels (excluding level 0).
        ///
        /// The deepest level is kept small; once @p max_levels is reached,
        /// further compaction merges into the deepest level without creating
        /// a new one.
        std::size_t max_levels = 4;

        /// @brief Size ratio between consecutive levels (fan‑out).
        ///
        /// Level (i+1) is allowed to hold up to @p fanOut × the target size
        /// of level i.  A larger fan‑out reduces write amplification but
        /// increases read amplification.
        std::size_t fan_out = 10;

        // ── Bloom filter ────────────────────────────────────────────────

        /// @brief Target false-positive probability for per-SSTable Bloom
        ///        filters.
        ///
        /// A lower value reduces useless I/O during point lookups at the
        /// cost of more memory for the filter.  0.01 (1 %) is a sensible
        /// default.
        double bloom_false_positive_rate = 0.01;

        // ── Debug / introspection ───────────────────────────────────────

        /// @brief If true, the tree records per-level statistics that can be
        ///        queried via @c stats().
        bool enable_stats = false;
    };
} // namespace common::data_structure::tree
