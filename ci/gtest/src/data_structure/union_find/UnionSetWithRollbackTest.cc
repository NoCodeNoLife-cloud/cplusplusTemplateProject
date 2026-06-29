/**
 * @file UnionSetWithRollbackTest.cc
 * @brief Unit tests for the UnionSetWithRollback class
 * @details Tests cover basic operations, snapshot/rollback semantics, commit,
 *          setCount tracking, clear behavior, and edge cases with large N
 *          and alternative index types.
 */

#include <cppforge/data_structure/union_find/UnionSetWithRollback.hpp>

#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

using namespace cppforge::data_structure::union_find;

// ══════════════════════════════════════════════════════════════════════════
//  Test Fixture
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test fixture for UnionSetWithRollback tests
 */
class UnionSetWithRollbackTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ══════════════════════════════════════════════════════════════════════════
//  Basic Operations
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Verify that each element is its own root after construction
 * @details After constructing a DSU with N elements, every element's find
 *          should return itself, setCount should equal size, and no two
 *          distinct elements should be connected.
 */
TEST_F(UnionSetWithRollbackTest, Constructor_InitialState_AllIsolated)
{
    constexpr std::size_t kN = 10;
    const UnionSetWithRollback<> dsu(kN);

    EXPECT_EQ(dsu.size(), kN);
    EXPECT_EQ(dsu.setCount(), kN);

    for (std::size_t i = 0; i < kN; ++i)
    {
        EXPECT_EQ(dsu.find(i), i);
    }

    for (std::size_t i = 0; i < kN; ++i)
    {
        for (std::size_t j = i + 1; j < kN; ++j)
        {
            EXPECT_FALSE(dsu.connected(i, j));
        }
    }
}

/**
 * @brief Test that n==0 throws invalid_argument
 * @details The constructor requires n > 0; passing zero should throw.
 */
TEST_F(UnionSetWithRollbackTest, Constructor_ZeroSize_Throws)
{
    EXPECT_THROW(UnionSetWithRollback<>(0), std::invalid_argument);
}

/**
 * @brief Test a single union connects two elements
 * @details After unionSets(0, 1), the two elements should be connected
 *          and share the same root.
 */
TEST_F(UnionSetWithRollbackTest, UnionSets_SingleMerge_Connected)
{
    UnionSetWithRollback<> dsu(5);

    const bool merged = dsu.unionSets(0, 1);

    EXPECT_TRUE(merged);
    EXPECT_TRUE(dsu.connected(0, 1));
    EXPECT_EQ(dsu.find(0), dsu.find(1));
}

/**
 * @brief Test transitive connectivity after multiple unions
 * @details A chain of unions (0-1, 1-2, 2-3) should make all elements
 *          in the chain connected transitively.
 */
TEST_F(UnionSetWithRollbackTest, UnionSets_MultipleUnions_TransitiveConnectivity)
{
    UnionSetWithRollback<> dsu(10);

    EXPECT_TRUE(dsu.unionSets(0, 1));
    EXPECT_TRUE(dsu.unionSets(1, 2));
    EXPECT_TRUE(dsu.unionSets(2, 3));

    EXPECT_TRUE(dsu.connected(0, 3));
    EXPECT_TRUE(dsu.connected(1, 3));
    EXPECT_TRUE(dsu.connected(0, 2));

    // Element 4 remains isolated
    EXPECT_FALSE(dsu.connected(0, 4));
}

/**
 * @brief Test that unionSets returns false when elements are already in same set
 * @details Merging already-connected elements should be a no-op and return false.
 */
TEST_F(UnionSetWithRollbackTest, UnionSets_AlreadyConnected_ReturnsFalse)
{
    UnionSetWithRollback<> dsu(5);

    EXPECT_TRUE(dsu.unionSets(0, 1));
    EXPECT_FALSE(dsu.unionSets(0, 1));
    EXPECT_FALSE(dsu.unionSets(1, 0));

    // Still connected
    EXPECT_TRUE(dsu.connected(0, 1));
}

// ══════════════════════════════════════════════════════════════════════════
//  Snapshot & Rollback  (core feature)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test single-level rollback reverts a union
 * @details Capture a snapshot, perform one union, then rollback.
 *          After rollback the two elements should no longer be connected
 *          and setCount should be restored.
 */
TEST_F(UnionSetWithRollbackTest, Rollback_SingleUndo_RevertsMerge)
{
    UnionSetWithRollback<> dsu(5);
    const auto initial_count = dsu.setCount();

    const auto s0 = dsu.snapshot();
    EXPECT_TRUE(dsu.unionSets(0, 1));
    EXPECT_EQ(dsu.setCount(), initial_count - 1);

    dsu.rollback(s0);
    EXPECT_FALSE(dsu.connected(0, 1));
    EXPECT_EQ(dsu.setCount(), initial_count);
}

/**
 * @brief Test rollback does not affect other connectivity
 * @details Merge A-B, take snapshot, merge C-D, rollback.  The A-B
 *          relation should be preserved while C-D is undone.
 */
TEST_F(UnionSetWithRollbackTest, Rollback_AfterMerge_OtherRelationsPreserved)
{
    UnionSetWithRollback<> dsu(10);

    EXPECT_TRUE(dsu.unionSets(0, 1));
    EXPECT_TRUE(dsu.unionSets(2, 3));

    const auto s1 = dsu.snapshot();

    EXPECT_TRUE(dsu.unionSets(4, 5));
    EXPECT_TRUE(dsu.unionSets(6, 7));

    dsu.rollback(s1);

    // Previously merged sets should still be connected
    EXPECT_TRUE(dsu.connected(0, 1));
    EXPECT_TRUE(dsu.connected(2, 3));

    // Post-snapshot unions should be undone
    EXPECT_FALSE(dsu.connected(4, 5));
    EXPECT_FALSE(dsu.connected(6, 7));
}

/**
 * @brief Test rollback to initial state makes all elements isolated
 * @details Capture snapshot at t=0, perform several unions, then rollback
 *          to initial state.  All elements should be isolated again.
 */
TEST_F(UnionSetWithRollbackTest, Rollback_ToInitialState_AllIsolated)
{
    constexpr std::size_t kN = 20;
    UnionSetWithRollback<> dsu(kN);

    const auto s0 = dsu.snapshot();

    // Perform several unions �?all should succeed on distinct pairs
    for (std::size_t i = 0; i < kN; i += 2)
    {
        EXPECT_TRUE(dsu.unionSets(i, i + 1));
    }

    // Rollback everything
    dsu.rollback(s0);

    EXPECT_EQ(dsu.setCount(), kN);
    for (std::size_t i = 0; i < kN; ++i)
    {
        EXPECT_EQ(dsu.find(i), i);
        for (std::size_t j = i + 1; j < kN; ++j)
        {
            EXPECT_FALSE(dsu.connected(i, j));
        }
    }
}

/**
 * @brief Test selective rollback with multiple snapshots
 * @details snapshot A, union A-B, snapshot B, union C-D, rollback to B.
 *          After rollback, A-B should still be connected but C-D should
 *          not be connected.
 */
TEST_F(UnionSetWithRollbackTest, Rollback_MultipleSnapshots_SelectiveUndo)
{
    UnionSetWithRollback<> dsu(10);

    const auto s0 = dsu.snapshot();

    EXPECT_TRUE(dsu.unionSets(0, 1));
    const auto s1 = dsu.snapshot();

    EXPECT_TRUE(dsu.unionSets(2, 3));

    // Rollback to s1 �?undoes only union(2,3)
    dsu.rollback(s1);
    EXPECT_TRUE(dsu.connected(0, 1));
    EXPECT_FALSE(dsu.connected(2, 3));

    // Rollback to s0 �?undoes everything
    dsu.rollback(s0);
    EXPECT_FALSE(dsu.connected(0, 1));
}

/**
 * @brief Test nested snapshot rollback (intermediate state then full undo)
 * @details s0 �?union(0,1) �?s1 �?union(2,3) �?s2 �?union(4,5)
 *          rollback s1 �?only (0,1) connected, (2,3) and (4,5) disconnected
 *          rollback s0 �?everything disconnected
 */
TEST_F(UnionSetWithRollbackTest, Rollback_NestedSnapshots_IntermediateAndFullUndo)
{
    UnionSetWithRollback<> dsu(10);

    const auto s0 = dsu.snapshot();
    EXPECT_TRUE(dsu.unionSets(0, 1));

    const auto s1 = dsu.snapshot();
    EXPECT_TRUE(dsu.unionSets(2, 3));

    const auto s2 = dsu.snapshot();
    EXPECT_TRUE(dsu.unionSets(4, 5));

    // Rollback to intermediate state s1 �?only union(0,1) remains
    dsu.rollback(s1);
    EXPECT_TRUE(dsu.connected(0, 1));
    EXPECT_FALSE(dsu.connected(2, 3));
    EXPECT_FALSE(dsu.connected(4, 5));

    // Rollback to initial state s0 �?all isolated
    dsu.rollback(s0);
    EXPECT_FALSE(dsu.connected(0, 1));
    EXPECT_FALSE(dsu.connected(2, 3));
    EXPECT_FALSE(dsu.connected(4, 5));
}

/**
 * @brief Test rollback when no mutations occurred between snapshot and rollback
 * @details Taking a snapshot and then immediately rolling back to it should
 *          be a no-op �?all connectivity should be unchanged.
 */
TEST_F(UnionSetWithRollbackTest, Rollback_NoChanges_NoEffect)
{
    UnionSetWithRollback<> dsu(5);

    EXPECT_TRUE(dsu.unionSets(0, 1));
    EXPECT_TRUE(dsu.unionSets(2, 3));

    const auto s = dsu.snapshot();
    // No mutations between snapshot and rollback
    dsu.rollback(s);

    // State should be unchanged
    EXPECT_TRUE(dsu.connected(0, 1));
    EXPECT_TRUE(dsu.connected(2, 3));
    EXPECT_FALSE(dsu.connected(0, 2));
}

/**
 * @brief Test rolling back all unions at once after many operations
 * @details Perform a series of unions, then rollback all of them with one call.
 */
TEST_F(UnionSetWithRollbackTest, Rollback_AllUnions_RevertsAll)
{
    constexpr std::size_t kN = 20;
    UnionSetWithRollback<> dsu(kN);

    const auto s0 = dsu.snapshot();

    // Perform unions forming 5 pairs
    for (std::size_t i = 0; i < kN; i += 2)
    {
        EXPECT_TRUE(dsu.unionSets(i, i + 1));
    }
    EXPECT_EQ(dsu.setCount(), kN - kN / 2);

    // Also connect pairs to form one big component
    EXPECT_TRUE(dsu.unionSets(0, 2));
    EXPECT_TRUE(dsu.unionSets(4, 6));

    // One rollback to undo everything
    dsu.rollback(s0);

    EXPECT_EQ(dsu.setCount(), kN);
    for (std::size_t i = 0; i < kN; ++i)
    {
        EXPECT_EQ(dsu.find(i), i);
    }
}

/**
 * @brief Test rollback specifically for rank-equal merges (2-record case)
 * @details When two roots have equal rank, unionSets pushes 2 ChangeRecords
 *          (parent change + rank increment).  Rollback must correctly restore
 *          both the parent pointer and the rank.  We build equal-rank trees
 *          by performing unions in a specific order.
 */
TEST_F(UnionSetWithRollbackTest, Rollback_RankEqualMerge_CorrectUndo)
{
    // Strategy:
    //   union(0,1) �?both rank 0 �?rank[0]=1  (root of {0,1})
    //   union(2,3) �?both rank 0 �?rank[2]=1  (root of {2,3})
    //   Now roots 0 and 2 both have rank 1 �?union(0,2) is rank-equal
    //   �?2 records pushed.  Rollback must restore both.
    UnionSetWithRollback<> dsu(10);
    const auto s0 = dsu.snapshot();

    EXPECT_TRUE(dsu.unionSets(0, 1));
    EXPECT_TRUE(dsu.unionSets(2, 3));
    // Create a third equal-rank pair to make it more thorough
    EXPECT_TRUE(dsu.unionSets(4, 5));

    // Root ranks of 0, 2, 4 are now 1.  Union two of them (rank-equal case).
    EXPECT_TRUE(dsu.unionSets(0, 2));

    // Verify connected
    EXPECT_TRUE(dsu.connected(0, 2));
    EXPECT_TRUE(dsu.connected(1, 3));

    // Also connect {4,5} into the same big component for good measure
    EXPECT_TRUE(dsu.unionSets(0, 4));
    EXPECT_TRUE(dsu.connected(0, 4));

    // Rollback everything
    dsu.rollback(s0);

    // All elements should be isolated, ranks restored to 0
    EXPECT_EQ(dsu.setCount(), 10);
    for (std::size_t i = 0; i < 10; ++i)
    {
        EXPECT_EQ(dsu.find(i), i);
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  Commit Semantics
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test commit clears the history stack
 * @details After commit(), snapshot() should return 0, indicating no
 *          pending history.
 */
TEST_F(UnionSetWithRollbackTest, Commit_AfterUnions_HistoryCleared)
{
    UnionSetWithRollback<> dsu(10);

    EXPECT_TRUE(dsu.unionSets(0, 1));
    EXPECT_TRUE(dsu.unionSets(2, 3));
    EXPECT_TRUE(dsu.unionSets(4, 5));

    EXPECT_GT(dsu.snapshot(), 0);

    dsu.commit();
    EXPECT_EQ(dsu.snapshot(), 0);
}

/**
 * @brief Test commit preserves the current connectivity state
 * @details After commit(), the data structure state should be unchanged
 *          even though the history has been discarded.
 */
TEST_F(UnionSetWithRollbackTest, Commit_AfterUnions_StatePreserved)
{
    UnionSetWithRollback<> dsu(10);

    EXPECT_TRUE(dsu.unionSets(0, 1));
    EXPECT_TRUE(dsu.unionSets(2, 3));

    dsu.commit();

    // State preserved
    EXPECT_TRUE(dsu.connected(0, 1));
    EXPECT_TRUE(dsu.connected(2, 3));
    EXPECT_FALSE(dsu.connected(0, 2));

    // New unions still work
    EXPECT_TRUE(dsu.unionSets(0, 2));
    EXPECT_TRUE(dsu.connected(0, 3));
}

// ══════════════════════════════════════════════════════════════════════════
//  setCount Tracking
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test initial setCount equals size
 * @details Right after construction, every element is its own set.
 */
TEST_F(UnionSetWithRollbackTest, SetCount_Initial_EqualsSize)
{
    constexpr std::size_t kN = 42;
    const UnionSetWithRollback<> dsu(kN);

    EXPECT_EQ(dsu.setCount(), kN);
    EXPECT_EQ(dsu.setCount(), dsu.size());
}

/**
 * @brief Test setCount tracks unions and rollbacks correctly
 * @details Each successful union decrements setCount; a selective rollback
 *          restores only the undone unions' contributions.  Verify that
 *          the intermediate setCount values are correct.
 */
TEST_F(UnionSetWithRollbackTest, SetCount_AfterUnionAndRollback_CorrectTracking)
{
    constexpr std::size_t kN = 10;
    UnionSetWithRollback<> dsu(kN);

    EXPECT_EQ(dsu.setCount(), kN);

    // Step 1: single union �?setCount-1
    EXPECT_TRUE(dsu.unionSets(0, 1));
    EXPECT_EQ(dsu.setCount(), kN - 1);

    // Step 2: snapshot, then two more unions
    const auto s1 = dsu.snapshot();
    EXPECT_TRUE(dsu.unionSets(2, 3));
    EXPECT_TRUE(dsu.unionSets(4, 5));
    EXPECT_EQ(dsu.setCount(), kN - 3);

    // Step 3: rollback s1 �?undoes (2,3) and (4,5) but keeps (0,1)
    dsu.rollback(s1);
    EXPECT_EQ(dsu.setCount(), kN - 1);
    EXPECT_TRUE(dsu.connected(0, 1));
    EXPECT_FALSE(dsu.connected(2, 3));
    EXPECT_FALSE(dsu.connected(4, 5));

    // Step 4: rollback to zero (initial state) �?undo (0,1) too
    dsu.rollback(0);
    EXPECT_EQ(dsu.setCount(), kN);
    for (std::size_t i = 0; i < kN; ++i)
    {
        EXPECT_EQ(dsu.find(i), i);
    }
}

/**
 * @brief Test setCount correctly tracks repeated union-rollback cycles
 * @details Verifying that a union followed by rollback can be repeated
 *          with identical results each cycle.
 */
TEST_F(UnionSetWithRollbackTest, SetCount_RepeatCycle_Consistent)
{
    constexpr std::size_t kN = 5;
    UnionSetWithRollback<> dsu(kN);

    EXPECT_EQ(dsu.setCount(), kN);

    const auto s0 = dsu.snapshot();

    // Union the first two elements
    EXPECT_TRUE(dsu.unionSets(0, 1));
    EXPECT_EQ(dsu.setCount(), kN - 1);

    // Rollback
    dsu.rollback(s0);
    EXPECT_EQ(dsu.setCount(), kN);

    // Union again �?should work identically
    EXPECT_TRUE(dsu.unionSets(0, 1));
    EXPECT_EQ(dsu.setCount(), kN - 1);

    // Rollback again
    dsu.rollback(s0);
    EXPECT_EQ(dsu.setCount(), kN);
}

// ══════════════════════════════════════════════════════════════════════════
//  Clear Semantics
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test clear resets all elements to isolated state
 * @details After performing unions, clear() should disconnect all elements,
 *          reset setCount to size, and reset all ranks to zero.
 */
TEST_F(UnionSetWithRollbackTest, Clear_AfterUnions_AllIsolated)
{
    constexpr std::size_t kN = 10;
    UnionSetWithRollback<> dsu(kN);

    EXPECT_TRUE(dsu.unionSets(0, 1));
    EXPECT_TRUE(dsu.unionSets(2, 3));
    EXPECT_TRUE(dsu.unionSets(0, 2));  // Connect 0-1-2-3

    // Verify current state
    EXPECT_TRUE(dsu.connected(0, 3));

    dsu.clear();

    EXPECT_EQ(dsu.setCount(), kN);
    EXPECT_EQ(dsu.size(), kN);

    for (std::size_t i = 0; i < kN; ++i)
    {
        EXPECT_EQ(dsu.find(i), i);
        for (std::size_t j = i + 1; j < kN; ++j)
        {
            EXPECT_FALSE(dsu.connected(i, j));
        }
    }
}

/**
 * @brief Test clear empties the history stack
 * @details After clear(), snapshot() returns 0 and rollback has no records
 *          to undo.
 */
TEST_F(UnionSetWithRollbackTest, Clear_AfterUnions_HistoryCleared)
{
    UnionSetWithRollback<> dsu(10);

    EXPECT_TRUE(dsu.unionSets(0, 1));
    EXPECT_TRUE(dsu.unionSets(2, 3));
    EXPECT_GT(dsu.snapshot(), 0);

    dsu.clear();
    EXPECT_EQ(dsu.snapshot(), 0);

    // After clear, new operations work and are independent
    EXPECT_TRUE(dsu.unionSets(5, 6));
    EXPECT_TRUE(dsu.connected(5, 6));
}

// ══════════════════════════════════════════════════════════════════════════
//  Edge Cases & Stress
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test DSU with the minimum possible size (n=1)
 * @details A single-element DSU should have setCount=1, its own root,
 *          and union should be a no-op.
 */
TEST_F(UnionSetWithRollbackTest, MinSize_SingleElement_Correct)
{
    UnionSetWithRollback<> dsu(1);

    EXPECT_EQ(dsu.size(), 1);
    EXPECT_EQ(dsu.setCount(), 1);
    EXPECT_EQ(dsu.find(0), 0);
    EXPECT_TRUE(dsu.connected(0, 0));

    // Union with itself is a no-op (same root, returns false)
    EXPECT_FALSE(dsu.unionSets(0, 0));
    EXPECT_EQ(dsu.setCount(), 1);
}

/**
 * @brief Test UnionSetWithRollback with a non-default Index type (int)
 * @details The template accepts any std::integral type.  Verify that
 *          using int as Index compiles and works correctly.
 */
TEST_F(UnionSetWithRollbackTest, CustomIndexType_Int_Works)
{
    UnionSetWithRollback<int> dsu(10);

    EXPECT_EQ(dsu.size(), 10);
    EXPECT_EQ(dsu.setCount(), 10);

    EXPECT_TRUE(dsu.unionSets(0, 1));
    EXPECT_TRUE(dsu.unionSets(2, 3));

    EXPECT_TRUE(dsu.connected(0, 1));
    EXPECT_FALSE(dsu.connected(0, 2));

    // Rollback with int Index
    const auto s = dsu.snapshot();
    EXPECT_TRUE(dsu.unionSets(5, 6));
    dsu.rollback(s);
    EXPECT_FALSE(dsu.connected(5, 6));

    // Clear
    dsu.clear();
    EXPECT_EQ(dsu.setCount(), 10);
}

/**
 * @brief Test UnionSetWithRollback with std::uint16_t as Index
 * @details Verify that a small unsigned integral type also works.
 */
TEST_F(UnionSetWithRollbackTest, CustomIndexType_Uint16_Works)
{
    UnionSetWithRollback<std::uint16_t> dsu(100);

    EXPECT_TRUE(dsu.unionSets(0, 1));
    EXPECT_TRUE(dsu.unionSets(50, 51));

    EXPECT_TRUE(dsu.connected(0, 1));
    EXPECT_FALSE(dsu.connected(0, 50));

    dsu.clear();
    EXPECT_FALSE(dsu.connected(0, 1));
}

/**
 * @brief Stress test with 1000 elements, many unions, and full rollback
 * @details Large-scale verification that unions, setCount tracking, and
 *          rollback all work correctly.  Uses a deterministic random
 *          sequence to generate varied merge patterns.
 */
TEST_F(UnionSetWithRollbackTest, LargeScale_ManyUnionsAndRollback_Correctness)
{
    constexpr std::size_t kN = 1000;
    UnionSetWithRollback<> dsu(kN);

    std::mt19937_64 rng(42);  // NOLINT: fixed seed for determinism
    std::uniform_int_distribution<std::size_t> dist(0, kN - 1);

    const auto s0 = dsu.snapshot();
    EXPECT_EQ(dsu.setCount(), kN);

    // Perform 200 random unions
    int successful_unions = 0;
    for (int i = 0; i < 200; ++i)
    {
        const auto x = dist(rng);
        const auto y = dist(rng);
        if (dsu.unionSets(x, y))
        {
            ++successful_unions;
        }
    }

    EXPECT_EQ(dsu.setCount(), kN - static_cast<std::size_t>(successful_unions));

    // Rollback everything
    dsu.rollback(s0);

    EXPECT_EQ(dsu.setCount(), kN);
    for (std::size_t i = 0; i < kN; ++i)
    {
        EXPECT_EQ(dsu.find(i), i);
    }
}

/**
 * @brief Stress test with many nested snapshot layers
 * @details Take periodic snapshots during a series of unions, then
 *          selectively roll back to each snapshot, verifying that
 *          each rollback produces the correct intermediate state.
 */
TEST_F(UnionSetWithRollbackTest, LargeScale_NestedSnapshots_SelectiveRollback)
{
    constexpr std::size_t kN = 500;
    UnionSetWithRollback<> dsu(kN);

    // Take snapshots at strategic points and store them
    std::vector<std::size_t> snapshots;
    snapshots.push_back(dsu.snapshot());  // snapshot at 0 unions

    // Perform unions in batches
    for (std::size_t batch = 0; batch < 5; ++batch)
    {
        for (std::size_t i = 0; i < 20; ++i)
        {
            const auto idx = batch * 20 + i;
            EXPECT_TRUE(dsu.unionSets(idx * 2, idx * 2 + 1));
        }
        snapshots.push_back(dsu.snapshot());
    }

    // Verify each snapshot: rollback, check state, restore
    // Go backwards so we can verify incremental state
    for (std::size_t lvl = snapshots.size(); lvl > 0; --lvl)
    {
        dsu.rollback(snapshots[lvl - 1]);

        // At this level, only unions with batch index < (lvl-1) should exist
        const auto expected_set_count = kN - (lvl - 1) * 20;
        EXPECT_EQ(dsu.setCount(), expected_set_count);
    }

    // Final state: all isolated
    EXPECT_EQ(dsu.setCount(), kN);
}
