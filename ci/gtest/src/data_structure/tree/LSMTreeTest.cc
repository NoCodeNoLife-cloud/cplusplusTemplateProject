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

TEST_F(LSMTreeTest, Constructor_DefaultOptions)
{
    LSMTree<int, std::string> tree;
    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.memtableSize() == 0);
}

TEST_F(LSMTreeTest, Constructor_CustomOptions)
{
    LSMTree<int, std::string> tree(opts_);
    EXPECT_EQ(tree.size(), 0);
}

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

TEST_F(LSMTreeTest, Get_NonExistentKey)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");

    const auto result = tree.get(999);
    EXPECT_FALSE(result.has_value());
}

TEST_F(LSMTreeTest, Put_UpdateExistingKey)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");
    tree.put(1, "ONE");

    EXPECT_EQ(tree.get(1).value_or(""), "ONE");
    EXPECT_EQ(tree.size(), 1);
}

TEST_F(LSMTreeTest, Contains_ExistingKey)
{
    LSMTree<int, std::string> tree(opts_);
    EXPECT_FALSE(tree.contains(1));

    tree.put(1, "one");
    EXPECT_TRUE(tree.contains(1));
}

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

TEST_F(LSMTreeTest, Remove_ExistingKey)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(42, "hello");
    EXPECT_TRUE(tree.contains(42));

    tree.remove(42);
    EXPECT_FALSE(tree.contains(42));
    EXPECT_FALSE(tree.get(42).has_value());
}

TEST_F(LSMTreeTest, Remove_NonExistentKey)
{
    LSMTree<int, std::string> tree(opts_);
    // Removing a key that was never inserted should be a no-op.
    tree.remove(999);
    EXPECT_FALSE(tree.contains(999));
}

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

TEST_F(LSMTreeTest, Size_Empty)
{
    LSMTree<int, std::string> tree(opts_);
    EXPECT_EQ(tree.size(), 0);
}

TEST_F(LSMTreeTest, Size_AfterInsert)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");
    EXPECT_EQ(tree.size(), 1);

    tree.put(2, "two");
    EXPECT_EQ(tree.size(), 2);
}

TEST_F(LSMTreeTest, Size_UpdateDoesNotChange)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");
    EXPECT_EQ(tree.size(), 1);

    tree.put(1, "ONE"); // update, not insert
    EXPECT_EQ(tree.size(), 1);
}

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

TEST_F(LSMTreeTest, Clear_Empty)
{
    LSMTree<int, std::string> tree(opts_);
    EXPECT_NO_THROW(tree.clear());
}

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

TEST_F(LSMTreeTest, MoveConstruct)
{
    LSMTree<int, std::string> tree(opts_);
    tree.put(1, "one");
    tree.put(2, "two");

    LSMTree<int, std::string> moved(std::move(tree));
    EXPECT_EQ(moved.get(1).value_or(""), "one");
    EXPECT_EQ(moved.get(2).value_or(""), "two");
}

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

TEST_F(LSMTreeTest, EmptyTree)
{
    LSMTree<int, std::string> tree(opts_);
    EXPECT_EQ(tree.size(), 0);
    EXPECT_FALSE(tree.contains(42));
    EXPECT_FALSE(tree.get(42).has_value());
    EXPECT_EQ(tree.levelCount(), 0);
}

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

TEST_F(LSMTreeTest, MoveValue)
{
    LSMTree<int, std::string> tree(opts_);
    std::string value = "hello";
    tree.put(1, std::move(value));
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move): moved-from is unspecified
    EXPECT_EQ(tree.get(1).value_or(""), "hello");
}

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
