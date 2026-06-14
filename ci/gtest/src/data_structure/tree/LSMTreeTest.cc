/**
 * @file LSMTreeTest.cc
 * @brief Unit tests for the LSMTree class
 * @details Tests cover LSM-Tree construction, put/get/remove operations,
 *          MemTable flush, compaction, tombstone propagation, and
 *          correctness under various workloads.
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

#include "data_structure/tree/LSMTree.hpp"
#include "data_structure/tree/LSMTreeOptions.hpp"

using namespace common::data_structure::tree;

// ── Test fixture ──────────────────────────────────────────────────────────

/**
 * @brief Test fixture for LSMTree tests.
 *
 * Uses a small MemTable size (16 entries) to force frequent flushes and
 * compactions even with modest data sets.
 */
class LSMTreeTest : public testing::Test
{
protected:
    void SetUp() override
    {
        opts_.memtable_size = 16;
        opts_.l0_compaction_threshold = 4;
        opts_.fan_out = 4;
        opts_.max_levels = 3;
    }

    void TearDown() override
    {
    }

    LSMTreeOptions opts_;
};

// ══════════════════════════════════════════════════════════════════════════
//  Construction & basic operations
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test constructor with default options
 * @details Verifies that an LSM tree constructed with default options is empty
 */
TEST_F(LSMTreeTest, Constructor_DefaultOptions)
{
    LSMTree<int, std::string> tree;
    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.memtableSize() == 0);
}

/**
 * @brief Test constructor with custom options
 * @details Verifies that an LSM tree constructed with custom options is empty
 */
TEST_F(LSMTreeTest, Constructor_CustomOptions)
{
    LSMTree<int, std::string> tree(opts_);
    EXPECT_EQ(tree.size(), 0);
}

/**
 * @brief Test basic put and get operations
 * @details Verifies that values stored via put can be retrieved via get
 */
TEST_F(LSMTreeTest, PutAndGet_Basic)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");
    tree.put(2, "two");
    tree.put(3, "three");

    EXPECT_EQ(tree.get(1).value_or(""), "one");
    EXPECT_EQ(tree.get(2).value_or(""), "two");
    EXPECT_EQ(tree.get(3).value_or(""), "three");
}

/**
 * @brief Test get with a non-existent key
 * @details Verifies that get returns nullopt for a key that was never inserted
 */
TEST_F(LSMTreeTest, Get_NonExistentKey)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");

    const auto result = tree.get(999);
    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Test put updates an existing key
 * @details Verifies that putting a new value for an existing key overwrites the old value
 */
TEST_F(LSMTreeTest, Put_UpdateExistingKey)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");
    tree.put(1, "ONE");

    EXPECT_EQ(tree.get(1).value_or(""), "ONE");
    EXPECT_EQ(tree.size(), 1);
}

/**
 * @brief Test contains with an existing key
 * @details Verifies that contains returns false initially and true after insertion
 */
TEST_F(LSMTreeTest, Contains_ExistingKey)
{
    LSMTree<int, std::string> tree(opts_);
    EXPECT_FALSE(tree.contains(1));

    tree.put(1, "one");
    EXPECT_TRUE(tree.contains(1));
}

/**
 * @brief Test contains after a key has been removed
 * @details Verifies that contains returns false for a key that was deleted
 */
TEST_F(LSMTreeTest, Contains_AfterRemove)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");
    tree.remove(1);
    EXPECT_FALSE(tree.contains(1));
}

// ══════════════════════════════════════════════════════════════════════════
//  Remove (tombstone)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test removing an existing key
 * @details Verifies that removing an existing key makes it not findable
 */
TEST_F(LSMTreeTest, Remove_ExistingKey)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(42, "hello");
    EXPECT_TRUE(tree.contains(42));

    tree.remove(42);
    EXPECT_FALSE(tree.contains(42));
    EXPECT_FALSE(tree.get(42).has_value());
}

/**
 * @brief Test removing a non-existent key
 * @details Verifies that removing a key that was never inserted is a no-op
 */
TEST_F(LSMTreeTest, Remove_NonExistentKey)
{
    LSMTree<int, std::string> tree(opts_);
    // Removing a key that was never inserted should be a no-op.
    tree.remove(999);
    EXPECT_FALSE(tree.contains(999));
}

/**
 * @brief Test re-inserting a key after deletion
 * @details Verifies that a key can be re-inserted after removal with a new value
 */
TEST_F(LSMTreeTest, Remove_ReinsertAfterDelete)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");
    tree.remove(1);
    EXPECT_FALSE(tree.contains(1));

    // Re-insert the same key.
    tree.put(1, "ONE");
    EXPECT_TRUE(tree.contains(1));
    EXPECT_EQ(tree.get(1).value_or(""), "ONE");
}

/**
 * @brief Test removing multiple keys
 * @details Verifies that removing even keys leaves odd keys findable
 */
TEST_F(LSMTreeTest, Remove_MultipleKeys)
{
    LSMTree<int, std::string> tree(opts_);
    for (int i = 0; i < 10; ++i)
    {
        tree.put(i, "val" + std::to_string(i));
    }

    // Delete even keys.
    for (int i = 0; i < 10; i += 2)
    {
        tree.remove(i);
    }

    for (int i = 0; i < 10; ++i)
    {
        if (i % 2 == 0)
        {
            EXPECT_FALSE(tree.contains(i)) << "Key " << i << " should be deleted";
        }
        else
        {
            EXPECT_TRUE(tree.contains(i)) << "Key " << i << " should exist";
        }
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  Flush & compaction
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test flush forces SSTable creation
 * @details Verifies that when the MemTable exceeds its size threshold,
 *          it is automatically flushed to an L0 SSTable
 */
TEST_F(LSMTreeTest, Flush_ForcesSSTableCreation)
{
    // Use a tiny memtable so that inserts trigger automatic flushes.
    // Set memtable_size = 5 so that the first 4 inserts fit without
    // triggering a flush (the check is >=).
    LSMTreeOptions tinyOpts = opts_;
    tinyOpts.memtable_size = 5;
    tinyOpts.l0_compaction_threshold = 10; // prevent compaction

    LSMTree<int, std::string> tree(tinyOpts);
    EXPECT_EQ(tree.sstableCount(0), 0);

    // Insert 4 entries — should not yet trigger a flush.
    for (int i = 0; i < 4; ++i)
    {
        tree.put(i, "v" + std::to_string(i));
    }
    EXPECT_EQ(tree.sstableCount(0), 0) << "MemTable not yet full";

    tree.put(4, "trigger"); // 5th entry triggers flush
    EXPECT_GE(tree.sstableCount(0), 1) << "MemTable should have been flushed";
}

/**
 * @brief Test data persists in SSTable after flush
 * @details Verifies that flushed data remains readable after MemTable is persisted
 */
TEST_F(LSMTreeTest, Flush_DataPersistsInSSTable)
{
    LSMTreeOptions tinyOpts = opts_;
    tinyOpts.memtable_size = 4;
    tinyOpts.l0_compaction_threshold = 10;

    LSMTree<int, std::string> tree(tinyOpts);

    for (int i = 0; i < 10; ++i)
    {
        tree.put(i, "val" + std::to_string(i));
    }

    // All keys should still be readable even after flushing.
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_TRUE(tree.contains(i)) << "Key " << i;
        EXPECT_EQ(tree.get(i).value_or(""), "val" + std::to_string(i));
    }
}

/**
 * @brief Test explicit flush operation
 * @details Verifies that calling flush() explicitly creates an SSTable
 *          and data survives the flush
 */
TEST_F(LSMTreeTest, ExplicitFlush)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");
    tree.put(2, "two");

    EXPECT_EQ(tree.sstableCount(0), 0);

    tree.flush();

    EXPECT_GE(tree.sstableCount(0), 1);

    // Data should survive the flush.
    EXPECT_EQ(tree.get(1).value_or(""), "one");
    EXPECT_EQ(tree.get(2).value_or(""), "two");
}

/**
 * @brief Test compaction merges SSTables
 * @details Verifies that compaction merges multiple SSTables into one
 *          and all data remains readable
 */
TEST_F(LSMTreeTest, Compaction_MergesSSTables)
{
    LSMTreeOptions tinyOpts = opts_;
    tinyOpts.memtable_size = 4;
    tinyOpts.l0_compaction_threshold = 2; // compact L0 after just 2 SSTables
    tinyOpts.fan_out = 4;
    tinyOpts.max_levels = 3;

    LSMTree<int, std::string> tree(tinyOpts);

    // Insert enough data to create several SSTables and trigger compaction.
    for (int i = 0; i < 32; ++i)
    {
        tree.put(i, "v" + std::to_string(i));
    }

    // Force compaction.
    tree.compact();

    // All data must still be readable.
    for (int i = 0; i < 32; ++i)
    {
        EXPECT_TRUE(tree.contains(i)) << "Key " << i << " lost after compaction";
        EXPECT_EQ(tree.get(i).value_or(""), "v" + std::to_string(i));
    }
}

/**
 * @brief Test tombstone survives flush
 * @details Verifies that a tombstone (delete marker) persists after flushing
 *          and shadows the older value
 */
TEST_F(LSMTreeTest, Tombstone_SurvivesFlush)
{
    LSMTreeOptions tinyOpts = opts_;
    tinyOpts.memtable_size = 4;
    tinyOpts.l0_compaction_threshold = 10;

    LSMTree<int, std::string> tree(tinyOpts);

    tree.put(1, "one");
    tree.flush(); // flush key 1 to L0

    tree.remove(1); // delete — tombstone goes into memtable
    tree.flush(); // flush tombstone to L0

    // Tombstone should shadow the older value.
    EXPECT_FALSE(tree.contains(1));
    EXPECT_FALSE(tree.get(1).has_value());
}

/**
 * @brief Test tombstone survives compaction
 * @details Verifies that a tombstone persists across compaction cycles
 *          and correctly deletes values from earlier SSTables
 */
TEST_F(LSMTreeTest, Tombstone_SurvivesCompaction)
{
    LSMTreeOptions tinyOpts = opts_;
    tinyOpts.memtable_size = 4;
    tinyOpts.l0_compaction_threshold = 2;
    tinyOpts.fan_out = 4;
    tinyOpts.max_levels = 3;

    LSMTree<int, std::string> tree(tinyOpts);

    // Insert values, flush, then delete and flush again.
    tree.put(1, "one");
    tree.put(2, "two");
    tree.flush(); // L0 SSTable 1: {1: "one", 2: "two"}

    tree.remove(1);
    tree.flush(); // L0 SSTable 2: {1: tombstone}

    // Force compaction of L0 → L1.
    tree.compact();

    // After compaction, key 1 should be deleted, key 2 should survive.
    EXPECT_FALSE(tree.contains(1));
    EXPECT_TRUE(tree.contains(2));
    EXPECT_EQ(tree.get(2).value_or(""), "two");
}

/**
 * @brief Test tombstone cancels value in compaction
 * @details Verifies that when a tombstone and the value it deletes are in the
 *          same compaction input, both are dropped from the output
 */
TEST_F(LSMTreeTest, Tombstone_CancelsValueInCompaction)
{
    // When a tombstone and the value it deletes are in the same compaction
    // input, both should be dropped (not propagated to the output).
    LSMTreeOptions tinyOpts = opts_;
    tinyOpts.memtable_size = 4;
    tinyOpts.l0_compaction_threshold = 2;
    tinyOpts.fan_out = 4;
    tinyOpts.max_levels = 3;

    LSMTree<int, std::string> tree(tinyOpts);

    tree.put(1, "one");
    tree.flush(); // L0 SSTable A: {1: "one"}

    tree.remove(1);
    tree.flush(); // L0 SSTable B: {1: tombstone}

    // Tree now has 2 SSTables in L0 → compaction threshold reached.
    // The next put() will trigger compaction via maybeSealAndFlush.
    tree.put(99, "keep"); // triggers compaction

    // Key 1 should be gone; key 99 should exist.
    EXPECT_FALSE(tree.contains(1));
    EXPECT_TRUE(tree.contains(99));
}

// ══════════════════════════════════════════════════════════════════════════
//  Size tracking
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test size on an empty tree
 * @details Verifies that a newly constructed LSM tree has size zero
 */
TEST_F(LSMTreeTest, Size_Empty)
{
    LSMTree<int, std::string> tree(opts_);
    EXPECT_EQ(tree.size(), 0);
}

/**
 * @brief Test size increases after insert
 * @details Verifies that putting new keys increases the size count
 */
TEST_F(LSMTreeTest, Size_AfterInsert)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");
    EXPECT_EQ(tree.size(), 1);

    tree.put(2, "two");
    EXPECT_EQ(tree.size(), 2);
}

/**
 * @brief Test size does not change on update
 * @details Verifies that updating an existing key does not change the size
 */
TEST_F(LSMTreeTest, Size_UpdateDoesNotChange)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");
    EXPECT_EQ(tree.size(), 1);

    tree.put(1, "ONE"); // update, not insert
    EXPECT_EQ(tree.size(), 1);
}

/**
 * @brief Test size decreases after remove
 * @details Verifies that removing a key decreases the approximate size count
 */
TEST_F(LSMTreeTest, Size_AfterRemove)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");
    tree.put(2, "two");
    EXPECT_EQ(tree.size(), 2);

    tree.remove(1);
    // Size may not immediately reflect the deletion (tombstone counts
    // approximately); it is an approximate counter.
    EXPECT_EQ(tree.size(), 1);
}

/**
 * @brief Test size becomes zero after clear
 * @details Verifies that clearing the tree resets size to zero
 */
TEST_F(LSMTreeTest, Size_AfterClear)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");
    tree.put(2, "two");
    tree.clear();
    EXPECT_EQ(tree.size(), 0);
}

// ══════════════════════════════════════════════════════════════════════════
//  Clear
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test clear on an empty tree
 * @details Verifies that clearing an empty tree does not throw
 */
TEST_F(LSMTreeTest, Clear_Empty)
{
    LSMTree<int, std::string> tree(opts_);
    EXPECT_NO_THROW(tree.clear());
}

/**
 * @brief Test clear removes all entries
 * @details Verifies that clearing a tree with 20 entries removes all of them
 */
TEST_F(LSMTreeTest, Clear_RemovesAllEntries)
{
    LSMTree<int, std::string> tree(opts_);
    for (int i = 0; i < 20; ++i)
    {
        tree.put(i, "v" + std::to_string(i));
    }
    EXPECT_GT(tree.size(), 0);

    tree.clear();

    EXPECT_EQ(tree.size(), 0);
    for (int i = 0; i < 20; ++i)
    {
        EXPECT_FALSE(tree.contains(i)) << "Key " << i << " should be gone";
    }
}

/**
 * @brief Test reusing tree after clear
 * @details Verifies that the tree can be reused after clearing
 */
TEST_F(LSMTreeTest, ClearThenReuse)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");
    tree.clear();

    tree.put(2, "two");
    EXPECT_TRUE(tree.contains(2));
    EXPECT_EQ(tree.size(), 1);
}

// ══════════════════════════════════════════════════════════════════════════
//  Large-scale / stress
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test large-scale put and get operations
 * @details Verifies that 500 entries can be inserted and retrieved correctly
 */
TEST_F(LSMTreeTest, LargeScale_PutAndGet)
{
    LSMTree<int, std::string> tree(opts_);

    constexpr int N = 500;
    for (int i = 0; i < N; ++i)
    {
        tree.put(i, "val" + std::to_string(i));
    }

    EXPECT_EQ(tree.size(), static_cast<std::size_t>(N));

    for (int i = 0; i < N; ++i)
    {
        auto val = tree.get(i);
        ASSERT_TRUE(val.has_value()) << "Missing key " << i;
        EXPECT_EQ(val.value(), "val" + std::to_string(i));
    }
}

/**
 * @brief Test large-scale update of all values
 * @details Verifies that all 200 entries can be updated and the new values are retrieved
 */
TEST_F(LSMTreeTest, LargeScale_Update)
{
    LSMTree<int, std::string> tree(opts_);

    constexpr int N = 200;
    for (int i = 0; i < N; ++i)
    {
        tree.put(i, "original");
    }

    // Update all values.
    for (int i = 0; i < N; ++i)
    {
        tree.put(i, "updated");
    }

    for (int i = 0; i < N; ++i)
    {
        EXPECT_EQ(tree.get(i).value_or(""), "updated");
    }
}

/**
 * @brief Test deleting all entries in a large dataset
 * @details Verifies that all 200 entries can be removed and none remain findable
 */
TEST_F(LSMTreeTest, LargeScale_DeleteAll)
{
    LSMTree<int, std::string> tree(opts_);

    constexpr int N = 200;
    for (int i = 0; i < N; ++i)
    {
        tree.put(i, "val");
    }

    for (int i = 0; i < N; ++i)
    {
        tree.remove(i);
    }

    for (int i = 0; i < N; ++i)
    {
        EXPECT_FALSE(tree.contains(i)) << "Key " << i << " should be deleted";
    }
}

/**
 * @brief Test interleaved put and remove operations
 * @details Verifies that alternating put and remove on even keys works correctly,
 *          with re-insertion using different values
 */
TEST_F(LSMTreeTest, LargeScale_InterleavedPutRemove)
{
    LSMTree<int, std::string> tree(opts_);

    constexpr int N = 100;
    for (int i = 0; i < N; ++i)
    {
        tree.put(i, "v" + std::to_string(i));
    }

    // Delete every other key, then re-insert a different value.
    for (int i = 0; i < N; i += 2)
    {
        tree.remove(i);
    }
    for (int i = 0; i < N; i += 2)
    {
        tree.put(i, "new_v" + std::to_string(i));
    }

    for (int i = 0; i < N; ++i)
    {
        EXPECT_TRUE(tree.contains(i)) << "Key " << i;
        if (i % 2 == 0)
        {
            EXPECT_EQ(tree.get(i).value_or(""), "new_v" + std::to_string(i));
        }
        else
        {
            EXPECT_EQ(tree.get(i).value_or(""), "v" + std::to_string(i));
        }
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  Move semantics
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test move constructor
 * @details Verifies that the move constructor transfers all data to the new tree
 */
TEST_F(LSMTreeTest, MoveConstruct)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");
    tree.put(2, "two");

    LSMTree<int, std::string> moved(std::move(tree));
    EXPECT_EQ(moved.get(1).value_or(""), "one");
    EXPECT_EQ(moved.get(2).value_or(""), "two");
}

/**
 * @brief Test move assignment
 * @details Verifies that move assignment transfers all data to the destination
 */
TEST_F(LSMTreeTest, MoveAssign)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");

    LSMTree<int, std::string> other(opts_);
    other = std::move(tree);
    EXPECT_EQ(other.get(1).value_or(""), "one");
}

// ══════════════════════════════════════════════════════════════════════════
//  Edge cases
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test operations on an empty tree
 * @details Verifies that size, contains, get, and levelCount behave correctly on empty tree
 */
TEST_F(LSMTreeTest, EmptyTree)
{
    LSMTree<int, std::string> tree(opts_);
    EXPECT_EQ(tree.size(), 0);
    EXPECT_FALSE(tree.contains(42));
    EXPECT_FALSE(tree.get(42).has_value());
    EXPECT_EQ(tree.levelCount(), 0);
}

/**
 * @brief Test LSM tree with string keys
 * @details Verifies that string keys work correctly with put and get
 */
TEST_F(LSMTreeTest, StringKeys)
{
    LSMTree<std::string, int> tree(opts_);
    tree.put("alpha", 1);
    tree.put("beta", 2);
    tree.put("gamma", 3);

    EXPECT_EQ(tree.get("alpha").value_or(-1), 1);
    EXPECT_EQ(tree.get("beta").value_or(-1), 2);
    EXPECT_EQ(tree.get("gamma").value_or(-1), 3);
    EXPECT_FALSE(tree.get("delta").has_value());
}

/**
 * @brief Test moving a value into the tree
 * @details Verifies that a value can be moved into a put operation
 */
TEST_F(LSMTreeTest, MoveValue)
{
    LSMTree<int, std::string> tree(opts_);
    std::string value = "hello";
    tree.put(1, std::move(value));
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move): moved-from is unspecified
    EXPECT_EQ(tree.get(1).value_or(""), "hello");
}

/**
 * @brief Test level count tracking
 * @details Verifies that levelCount is zero initially and increases after flush
 */
TEST_F(LSMTreeTest, LevelCount)
{
    LSMTree<int, std::string> tree(opts_);
    EXPECT_EQ(tree.levelCount(), 0);

    tree.put(1, "one");
    // Data is still in MemTable — no levels used yet.
    EXPECT_EQ(tree.levelCount(), 0);

    tree.flush();
    EXPECT_GE(tree.levelCount(), 1);
}

// ══════════════════════════════════════════════════════════════════════════
//  Value persistence: correctness across flush & compaction cycles
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test values survive multiple flushes
 * @details Verifies that data remains readable after multiple MemTable flushes
 */
TEST_F(LSMTreeTest, ValuesSurviveMultipleFlushes)
{
    LSMTreeOptions tinyOpts = opts_;
    tinyOpts.memtable_size = 4;
    tinyOpts.l0_compaction_threshold = 10;

    LSMTree<int, std::string> tree(tinyOpts);

    // Insert 20 entries (5 flushes with memtable_size=4).
    for (int i = 0; i < 20; ++i)
    {
        tree.put(i, "v" + std::to_string(i));
    }

    // Read back all values.
    for (int i = 0; i < 20; ++i)
    {
        ASSERT_TRUE(tree.contains(i)) << "Key " << i;
        EXPECT_EQ(tree.get(i).value_or(""), "v" + std::to_string(i));
    }
}

/**
 * @brief Test values survive compaction cycles
 * @details Verifies that 400 entries remain readable after multiple compaction rounds
 */
TEST_F(LSMTreeTest, ValuesSurviveCompactionCycles)
{
    LSMTreeOptions tinyOpts = opts_;
    tinyOpts.memtable_size = 4;
    tinyOpts.l0_compaction_threshold = 2;
    tinyOpts.fan_out = 4;
    tinyOpts.max_levels = 4;

    LSMTree<int, std::string> tree(tinyOpts);

    // Insert enough data to trigger multiple compaction rounds.
    constexpr int N = 400;
    for (int i = 0; i < N; ++i)
    {
        tree.put(i, "persist_" + std::to_string(i));
    }

    // Multiple compaction rounds.
    for (int round = 0; round < 5; ++round)
    {
        tree.compact();
    }

    // All data must survive.
    for (int i = 0; i < N; ++i)
    {
        ASSERT_TRUE(tree.contains(i)) << "Key " << i << " after compaction";
        EXPECT_EQ(tree.get(i).value_or(""), "persist_" + std::to_string(i));
    }
}

/**
 * @brief Test tombstone vs newer value across compaction
 * @details Verifies that a new value written after a tombstone correctly
 *          shadows the tombstone across compaction boundaries
 */
TEST_F(LSMTreeTest, Tombstone_VsNewerValueAcrossCompaction)
{
    // Scenario:
    //   Level 0: SSTable A {1: "old"}
    //   Level 0: SSTable B {1: tombstone}  (deletion)
    //   Compaction A+B → Level 1: {1: tombstone kept}
    //   Level 0: SSTable C {1: "new"}      (re-insert)
    //   Compaction C + Level 1 tombstone → Level 1: {1: "new"}
    LSMTreeOptions tinyOpts = opts_;
    tinyOpts.memtable_size = 4;
    tinyOpts.l0_compaction_threshold = 2;
    tinyOpts.fan_out = 4;
    tinyOpts.max_levels = 3;

    LSMTree<int, std::string> tree(tinyOpts);

    tree.put(1, "old");
    tree.flush();
    tree.remove(1);
    tree.flush(); // L0: SSTable A ("old") + SSTable B (tombstone)

    tree.compact(); // tombstone should propagate to L1

    // Re-insert the key.
    tree.put(1, "new");
    tree.flush(); // SSTable C in L0

    // The new value should shadow the tombstone.
    EXPECT_TRUE(tree.contains(1));
    EXPECT_EQ(tree.get(1).value_or(""), "new");
}

// ══════════════════════════════════════════════════════════════════════════
//  Sorted order invariants (SSTable entries are always sorted by key)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test non-sequential key insertion
 * @details Verifies that keys inserted in random order are all findable
 */
TEST_F(LSMTreeTest, NonSequentialKeys)
{
    LSMTree<int, std::string> tree(opts_);

    // Insert in random-ish order.
    std::vector<int> keys = {42, 7, 99, 13, 55, 1, 88, 34, 21, 66};
    for (int k : keys)
    {
        tree.put(k, "v" + std::to_string(k));
    }

    for (int k : keys)
    {
        EXPECT_TRUE(tree.contains(k)) << "Key " << k;
        EXPECT_EQ(tree.get(k).value_or(""), "v" + std::to_string(k));
    }
}

/**
 * @brief Test descending key insertion order
 * @details Verifies that keys inserted in descending order are all findable
 */
TEST_F(LSMTreeTest, DescendingKeyOrder)
{
    LSMTree<int, std::string> tree(opts_);

    for (int i = 100; i >= 1; --i)
    {
        tree.put(i, "v" + std::to_string(i));
    }

    for (int i = 1; i <= 100; ++i)
    {
        EXPECT_TRUE(tree.contains(i)) << "Key " << i;
        EXPECT_EQ(tree.get(i).value_or(""), "v" + std::to_string(i));
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  Thread safety (basic concurrent access)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test concurrent reads
 * @details Verifies that multiple threads can read from the tree simultaneously
 *          without crashing or producing incorrect values
 */
TEST_F(LSMTreeTest, ConcurrentReads)
{
    LSMTree<int, std::string> tree(opts_);

    constexpr int N = 100;
    for (int i = 0; i < N; ++i)
    {
        tree.put(i, "v" + std::to_string(i));
    }

    // Concurrent reads should not crash or produce wrong values.
    std::vector<std::thread> readers;
    for (int t = 0; t < 4; ++t)
    {
        readers.emplace_back([&tree, t, N]()
        {
            for (int i = t; i < N; i += 4)
            {
                auto val = tree.get(i);
                ASSERT_TRUE(val.has_value()) << "Thread " << t << " missing key " << i;
                ASSERT_EQ(val.value(), "v" + std::to_string(i));
            }
        });
    }

    for (auto& th : readers)
    {
        th.join();
    }
}

/**
 * @brief Test concurrent read and write
 * @details Verifies that concurrent reads and writes do not crash,
 *          and all data is consistent after both threads complete
 */
TEST_F(LSMTreeTest, ConcurrentReadWrite)
{
    LSMTree<int, std::string> tree(opts_);

    std::thread writer([&tree]()
    {
        for (int i = 0; i < 200; ++i)
        {
            tree.put(i, "w" + std::to_string(i));
        }
    });

    std::thread reader([&tree]()
    {
        for (int i = 0; i < 200; ++i)
        {
            // Reading during concurrent writes should not crash; results
            // may be inconsistent (that is expected without external sync).
            (void)tree.contains(i);
        }
    });

    writer.join();
    reader.join();

    // After both threads finish, all data must be consistent.
    for (int i = 0; i < 200; ++i)
    {
        EXPECT_TRUE(tree.contains(i));
    }
}
