/**
 * @file UnionSetWithRollback.hpp
 * @brief Rollback-able Union-Find (Disjoint Set Union) with union by rank
 * @details A Union-Find data structure that supports rollback to arbitrary
 *          previous states via a history stack.  Uses array-based storage
 *          (std::vector) with integer indexing and union by rank (no path
 *          compression), ensuring that find() is a purely read-only operation
 *          and the full history of mutations is preserved for rollback.
 *          Each successful union records 1-2 ChangeRecord entries on an
 *          internal history stack; rollback() replays them in reverse.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for all concurrent access patterns.
 *
 * @par Complexity
 * - find:        O(α(n)) amortised (union by rank without path compression)
 * - unionSets:   O(α(n)) amortised
 * - connected:   O(α(n)) amortised
 * - snapshot:    O(1)
 * - rollback:    O(k) where k = |history| - id
 * - commit:      O(1)
 *
 * @par Usage Example
 * @code
 * UnionSetWithRollback dsu(10);
 * const auto s1 = dsu.snapshot();
 * dsu.unionSets(0, 1);
 * dsu.unionSets(2, 3);
 * dsu.rollback(s1);
 * assert(!dsu.connected(0, 1));  // disconnected after rollback
 * @endcode
 */

#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <limits>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <vector>

namespace cppforge::data_structure::union_find
{

/// @brief A Union-Find (Disjoint Set Union) with rollback support.
///
/// Uses integer-indexed array storage with union by rank.  No path compression
/// is performed, keeping find() a pure read-only operation and enabling a full
/// mutation history for rollback.
///
/// @tparam Index  Integral type used for element indices (default std::size_t).
///
/// @par Memory
/// Storage is allocated up-front in the constructor: O(n) for parent + rank
/// vectors.  The history stack grows as unions are performed; use commit()
/// to reclaim history memory when rollback is no longer required.
///
/// @par Rollback Semantics
/// Every successful union pushes 1-2 ChangeRecord entries onto the history
/// stack.  rollback(id) pops entries until the stack size reaches @p id,
/// restoring each individual change in reverse order.  The set count is
/// adjusted accordingly.  Snapshot IDs are simply history stack sizes at
/// the time snapshot() was called.
template <std::integral Index = std::size_t>
class UnionSetWithRollback final
{
public:
    // ── Construction ──────────────────────────────────────────────

    /// @brief Constructs a UnionSetWithRollback with @p n isolated elements.
    /// @param n  Number of elements (must be > 0).
    /// @throws std::invalid_argument if n == 0.
    explicit UnionSetWithRollback(std::size_t n);

    // ── Queries ───────────────────────────────────────────────────

    /// @brief Finds the root (representative) of the set containing element x.
    /// @param x  Element index to find.
    /// @return The root index of the set containing x.
    /// @par Complexity
    /// O(α(n)) amortised �?no path compression, only rank-based depth bound.
    [[nodiscard]] auto find(Index x) const noexcept -> Index;

    /// @brief Checks if elements x and y belong to the same set.
    /// @param x  First element.
    /// @param y  Second element.
    /// @return true if x and y are connected (same root).
    [[nodiscard]] auto connected(Index x, Index y) const noexcept -> bool;

    // ── Mutations ─────────────────────────────────────────────────

    /// @brief Unites the sets containing elements x and y.
    ///
    /// Uses union by rank to keep trees balanced.  On success, pushes 1-2
    /// ChangeRecord entries onto the internal history stack so the union
    /// can be undone by rollback().
    /// @param x  First element.
    /// @param y  Second element.
    /// @return true if the sets were united, false if they were already in
    ///         the same set (no history recorded).
    /// @throws std::bad_alloc if memory allocation for the history record fails.
    [[nodiscard]] auto unionSets(Index x, Index y) -> bool;

    // ── Rollback ──────────────────────────────────────────────────

    /// @brief Captures the current state as a snapshot ID.
    /// @return An opaque ID that can be passed to rollback() to return
    ///         to the current state.
    [[nodiscard]] auto snapshot() const noexcept -> std::size_t;

    /// @brief Rolls back the data structure to the state identified by @p id.
    ///
    /// Pops ChangeRecord entries from the history stack until its size
    /// reaches @p id.  Each popped entry restores the parent (and
    /// optionally the rank) of the recorded node, and increments the
    /// set count to reflect the undone union.
    /// @param id  Snapshot ID previously returned by snapshot(), or any
    ///            value in [0, history_.size()].
    /// @pre  id <= history_.size()
    auto rollback(std::size_t id) noexcept -> void;

    /// @brief Discards all history, making all prior snapshot IDs invalid.
    ///
    /// This frees the memory used by the history stack.  After calling
    /// commit(), rollback() can still be used with snapshot IDs obtained
    /// after the commit.
    auto commit() noexcept -> void;

    // ── State queries ─────────────────────────────────────────────

    /// @brief Returns the total number of elements.
    /// @return n as passed to the constructor (unchanged over the lifetime).
    [[nodiscard]] auto size() const noexcept -> std::size_t;

    /// @brief Returns the current number of disjoint sets.
    /// @return Number of roots (connected components) in the DSU.
    [[nodiscard]] auto setCount() const noexcept -> std::size_t;

    /// @brief Resets all elements to isolated sets and clears history.
    ///
    /// After clear(), every element is its own root with rank 0, as if
    /// the object had just been constructed.
    auto clear() noexcept -> void;

private:
    // ── ChangeRecord ──────────────────────────────────────────────

    /// @brief A single atomic mutation recorded for rollback.
    struct ChangeRecord
    {
        Index node;                    ///< The element whose state was changed.
        Index old_parent;              ///< The parent value before this change.
        std::optional<int> old_rank;   ///< The rank before this change (std::nullopt if unchanged).
    };

    // ── Internal state ────────────────────────────────────────────

    std::vector<Index> parent_;      ///< parent_[i] == parent of element i.
    std::vector<int> rank_;          ///< rank_[i] == upper bound on tree height at root i.
    std::vector<ChangeRecord> history_;  ///< Mutation history (LIFO).
    std::size_t set_count_;          ///< Current number of disjoint sets.
};

// ─────────────────────────────────────────────────────────────────────────────
// Out-of-line method definitions
// ─────────────────────────────────────────────────────────────────────────────

template <std::integral Index>
UnionSetWithRollback<Index>::UnionSetWithRollback(const std::size_t n)
    : parent_(n)
    , rank_(n, 0)
    , set_count_(n)
{
    if (n == 0)
    {
        throw std::invalid_argument("UnionSetWithRollback: n must be > 0");
    }
    if (n > static_cast<std::size_t>(std::numeric_limits<Index>::max()))
    {
        throw std::overflow_error(
            "UnionSetWithRollback: n exceeds Index type range");
    }
    for (std::size_t i = 0; i < n; ++i)
    {
        parent_[i] = static_cast<Index>(i);
    }
}

template <std::integral Index>
auto UnionSetWithRollback<Index>::find(const Index x) const noexcept -> Index
{
    Index root = x;
    while (parent_[root] != root)
    {
        root = parent_[root];
    }
    return root;
}

template <std::integral Index>
auto UnionSetWithRollback<Index>::connected(const Index x,
                                            const Index y) const noexcept -> bool
{
    return find(x) == find(y);
}

template <std::integral Index>
auto UnionSetWithRollback<Index>::unionSets(const Index x,
                                            const Index y) -> bool
{
    const Index rx = find(x);
    const Index ry = find(y);

    if (rx == ry)
    {
        return false;
    }

    if (rank_[rx] < rank_[ry])
    {
        // Attach rx under ry �?1 parent change
        history_.push_back({rx, parent_[rx], std::nullopt});
        parent_[rx] = ry;
        --set_count_;
        return true;
    }

    if (rank_[rx] > rank_[ry])
    {
        // Attach ry under rx �?1 parent change
        history_.push_back({ry, parent_[ry], std::nullopt});
        parent_[ry] = rx;
        --set_count_;
        return true;
    }

    // Attach ry under rx, then increase rank of rx �?2 changes
    history_.push_back({ry, parent_[ry], std::nullopt});
    parent_[ry] = rx;

    history_.push_back({rx, parent_[rx], rank_[rx]});
    ++rank_[rx];
    --set_count_;
    return true;
}

template <std::integral Index>
auto UnionSetWithRollback<Index>::snapshot() const noexcept -> std::size_t
{
    return history_.size();
}

template <std::integral Index>
auto UnionSetWithRollback<Index>::rollback(const std::size_t id) noexcept -> void
{
    assert(id <= history_.size() && "rollback: id out of range");
    while (history_.size() > id)
    {
        const auto& rec = history_.back();
        parent_[rec.node] = rec.old_parent;
        if (rec.old_rank.has_value())
        {
            rank_[rec.node] = rec.old_rank.value();
        }
        history_.pop_back();
        // Each successful union decrements set_count_ exactly once.
        // For rank-equal unions, 2 records are pushed (parent + rank);
        // only the parent-change record (old_rank == nullopt) triggers
        // the set_count_ increment.  Rank-only records (old_rank.has_value)
        // are secondary changes and do not adjust set_count_.
        if (!rec.old_rank.has_value())
        {
            ++set_count_;
        }
    }
}

template <std::integral Index>
auto UnionSetWithRollback<Index>::commit() noexcept -> void
{
    history_.clear();
}

template <std::integral Index>
auto UnionSetWithRollback<Index>::size() const noexcept -> std::size_t
{
    return parent_.size();
}

template <std::integral Index>
auto UnionSetWithRollback<Index>::setCount() const noexcept -> std::size_t
{
    return set_count_;
}

template <std::integral Index>
auto UnionSetWithRollback<Index>::clear() noexcept -> void
{
    std::iota(parent_.begin(), parent_.end(), Index{0});
    std::fill(rank_.begin(), rank_.end(), 0);
    set_count_ = static_cast<std::size_t>(parent_.size());
    history_.clear();
}

} // namespace cppforge::data_structure::union_find
