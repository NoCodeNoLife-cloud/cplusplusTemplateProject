/**
 * @file ConsistentHashTest.cc
 * @brief Unit tests for the ConsistentHash class
 * @details Tests cover ring operations (add/remove/clear), key routing,
 *          replication, virtual node behaviour, minimal-remapping invariant,
 *          determinism, thread safety, edge cases, and large-scale usage.
 */

#include <gtest/gtest.h>

#include "data_structure/hash/ConsistentHash.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <thread>
#include <vector>

using namespace common::data_structure::hash;

// ══════════════════════════════════════════════════════════════════════════
//  Test fixture
// ══════════════════════════════════════════════════════════════════════════

class ConsistentHashTest : public testing::Test
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
//  1. Empty ring behaviours
// ══════════════════════════════════════════════════════════════════════════

TEST_F(ConsistentHashTest, EmptyRing_GetNode_Throws)
{
    const ConsistentHash ring;
    EXPECT_THROW(static_cast<void>(ring.getNode("any-key")), std::runtime_error);
}

TEST_F(ConsistentHashTest, EmptyRing_GetNodes_ReturnsEmpty)
{
    const ConsistentHash ring;
    const auto nodes = ring.getNodes("key", 3);
    EXPECT_TRUE(nodes.empty());
}

TEST_F(ConsistentHashTest, EmptyRing_RemoveNode_ReturnsFalse)
{
    ConsistentHash ring;
    EXPECT_FALSE(ring.removeNode("non-existent"));
}

TEST_F(ConsistentHashTest, EmptyRing_NodeCountIsZero)
{
    const ConsistentHash ring;
    EXPECT_EQ(ring.nodeCount(), 0);
    EXPECT_EQ(ring.virtualNodeCount(), 0);
}

TEST_F(ConsistentHashTest, EmptyRing_ContainsNode_ReturnsFalse)
{
    const ConsistentHash ring;
    EXPECT_FALSE(ring.containsNode("anything"));
}

// ══════════════════════════════════════════════════════════════════════════
//  2. Single node
// ══════════════════════════════════════════════════════════════════════════

TEST_F(ConsistentHashTest, SingleNode_AllKeysMapToIt)
{
    ConsistentHash ring;
    ring.addNode("node-A", 3);

    for (int i = 0; i < 1000; ++i)
    {
        const auto key = "key-" + std::to_string(i);
        EXPECT_EQ(ring.getNode(key), "node-A");
    }
}

TEST_F(ConsistentHashTest, SingleNode_GetNodes_ReturnsJustThatNode)
{
    ConsistentHash ring;
    ring.addNode("node-X", 3);

    const auto nodes = ring.getNodes("anything", 5);
    ASSERT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes[0], "node-X");
}

TEST_F(ConsistentHashTest, SingleNode_Counters)
{
    ConsistentHash ring;
    ring.addNode("sole", 10);
    EXPECT_EQ(ring.nodeCount(), 1);
    EXPECT_EQ(ring.virtualNodeCount(), 10);
}

// ══════════════════════════════════════════════════════════════════════════
//  3. Multiple nodes — basic correctness
// ══════════════════════════════════════════════════════════════════════════

TEST_F(ConsistentHashTest, MultipleNodes_KeysDistribution)
{
    ConsistentHash ring;
    constexpr int NODE_COUNT = 5;
    for (int i = 0; i < NODE_COUNT; ++i)
    {
        ring.addNode("node-" + std::to_string(i), 100);
    }

    EXPECT_EQ(ring.nodeCount(), NODE_COUNT);

    std::set<NodeName> usedNodes;
    for (int i = 0; i < 10000; ++i)
    {
        usedNodes.insert(ring.getNode("key-" + std::to_string(i)));
    }

    // All nodes should have at least some keys assigned
    EXPECT_EQ(usedNodes.size(), NODE_COUNT);
}

TEST_F(ConsistentHashTest, MultipleNodes_GetNodes_NoDuplicates)
{
    ConsistentHash ring;
    for (int i = 0; i < 5; ++i)
    {
        ring.addNode("node-" + std::to_string(i), 50);
    }

    const auto nodes = ring.getNodes("some-key", 3);
    EXPECT_EQ(nodes.size(), 3);

    const std::set<NodeName> unique(nodes.begin(), nodes.end());
    EXPECT_EQ(unique.size(), nodes.size());
}

TEST_F(ConsistentHashTest, GetNodes_FirstElementEqualsGetNode)
{
    ConsistentHash ring;
    for (int i = 0; i < 3; ++i)
    {
        ring.addNode("node-" + std::to_string(i), 50);
    }

    const auto nodes = ring.getNodes("test-key", 2);
    ASSERT_GE(nodes.size(), 1);
    EXPECT_EQ(nodes[0], ring.getNode("test-key"));
}

TEST_F(ConsistentHashTest, GetNodes_MoreThanAvailable)
{
    ConsistentHash ring;
    ring.addNode("only-one", 50);

    const auto nodes = ring.getNodes("key", 100);
    EXPECT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes[0], "only-one");
}

// ══════════════════════════════════════════════════════════════════════════
//  4. Determinism
// ══════════════════════════════════════════════════════════════════════════

TEST_F(ConsistentHashTest, Determinism_SameKeySameNode)
{
    ConsistentHash ring;
    for (int i = 0; i < 10; ++i)
    {
        ring.addNode("node-" + std::to_string(i), 100);
    }

    const std::vector<std::string> keys = {"apple", "banana", "cherry",
                                           "42", "3.14", "user:10086"};

    for (const auto& key : keys)
    {
        const auto first = ring.getNode(key);
        // Call many times — must always return the same result
        for (int trial = 0; trial < 10; ++trial)
        {
            EXPECT_EQ(ring.getNode(key), first);
        }
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  5. Node addition & removal — minimal remapping invariant
// ══════════════════════════════════════════════════════════════════════════

TEST_F(ConsistentHashTest, AddNode_MinimalRemapping)
{
    constexpr uint64_t KEY_COUNT = 20000;
    constexpr int INITIAL_NODES = 10;
    constexpr int VNODES = 150;  // high vnode count for uniform distribution

    ConsistentHash ring;

    // Initial cluster
    for (int i = 0; i < INITIAL_NODES; ++i)
    {
        ring.addNode("node-" + std::to_string(i), VNODES);
    }

    // Record original mapping
    std::map<std::string, NodeName> originalMapping;
    for (uint64_t i = 0; i < KEY_COUNT; ++i)
    {
        const auto key = "key-" + std::to_string(i);
        originalMapping[key] = ring.getNode(key);
    }

    // Add one new node
    ring.addNode("node-new", VNODES);

    // Count how many keys changed nodes
    uint64_t moved = 0;
    for (const auto& [key, originalNode] : originalMapping)
    {
        if (ring.getNode(key) != originalNode)
        {
            ++moved;
        }
    }

    // Expected remapping ≈ 1 / (INITIAL_NODES + 1) ≈ 9.1%
    // Allow generous bounds: [5%, 20%]
    const double ratio = static_cast<double>(moved) / static_cast<double>(KEY_COUNT);
    EXPECT_GT(ratio, 0.05);
    EXPECT_LT(ratio, 0.20);
}

TEST_F(ConsistentHashTest, RemoveNode_OnlyAffectedKeysMove)
{
    constexpr uint64_t KEY_COUNT = 30000;
    constexpr int VNODES = 150;

    ConsistentHash ring;
    ring.addNode("node-A", VNODES);
    ring.addNode("node-B", VNODES);
    ring.addNode("node-C", VNODES);

    // Record original mapping
    std::map<std::string, NodeName> originalMapping;
    std::set<std::string> keysOnB;

    for (uint64_t i = 0; i < KEY_COUNT; ++i)
    {
        const auto key = "key-" + std::to_string(i);
        const auto node = ring.getNode(key);
        originalMapping[key] = node;
        if (node == "node-B")
        {
            keysOnB.insert(key);
        }
    }

    // Remove node-B
    ASSERT_TRUE(ring.removeNode("node-B"));

    // Verify:
    //   1. No key maps to node-B anymore
    //   2. Keys NOT on B have NOT moved
    uint64_t keysOnBAfterRemoval = 0;
    uint64_t unmovedNonB = 0;
    uint64_t movedNonB = 0;

    for (const auto& [key, originalNode] : originalMapping)
    {
        const auto newNode = ring.getNode(key);

        if (newNode == "node-B")
        {
            ++keysOnBAfterRemoval;
        }

        if (originalNode == "node-B")
        {
            // Was on B: must have moved to A or C
            EXPECT_NE(newNode, "node-B");
        }
        else
        {
            // Was NOT on B: should not have moved
            if (newNode == originalNode)
            {
                ++unmovedNonB;
            }
            else
            {
                ++movedNonB;
            }
        }
    }

    // No key should map to the removed node
    EXPECT_EQ(keysOnBAfterRemoval, 0);

    // Ideally zero non-B keys should move; allow tiny tolerance for
    // virtual-node-edge effects
    EXPECT_LE(movedNonB, static_cast<uint64_t>(KEY_COUNT * 0.005));
}

// ══════════════════════════════════════════════════════════════════════════
//  6. Node re-addition (duplicate)
// ══════════════════════════════════════════════════════════════════════════

TEST_F(ConsistentHashTest, ReAddNode_UpdatesVirtualCount)
{
    ConsistentHash ring;
    ring.addNode("node-X", 10);
    EXPECT_EQ(ring.virtualNodeCount(), 10);

    ring.addNode("node-X", 50);  // re-add with different count
    EXPECT_EQ(ring.virtualNodeCount(), 50);
    EXPECT_EQ(ring.nodeCount(), 1);
}

// ══════════════════════════════════════════════════════════════════════════
//  7. Clear
// ══════════════════════════════════════════════════════════════════════════

TEST_F(ConsistentHashTest, Clear_EmptiesRing)
{
    ConsistentHash ring;
    ring.addNode("node-A", 10);
    ring.addNode("node-B", 10);
    ASSERT_EQ(ring.nodeCount(), 2);
    ASSERT_GT(ring.virtualNodeCount(), 0);

    ring.clear();

    EXPECT_EQ(ring.nodeCount(), 0);
    EXPECT_EQ(ring.virtualNodeCount(), 0);
    EXPECT_TRUE(ring.getNodes("key", 1).empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  8. ContainsNode
// ══════════════════════════════════════════════════════════════════════════

TEST_F(ConsistentHashTest, ContainsNode_AfterAddAndRemove)
{
    ConsistentHash ring;

    EXPECT_FALSE(ring.containsNode("node-Z"));
    ring.addNode("node-Z", 5);
    EXPECT_TRUE(ring.containsNode("node-Z"));
    (void)ring.removeNode("node-Z");
    EXPECT_FALSE(ring.containsNode("node-Z"));
}

// ══════════════════════════════════════════════════════════════════════════
//  9. Statistical distribution balance (chi-squared style)
// ══════════════════════════════════════════════════════════════════════════

TEST_F(ConsistentHashTest, StatisticalDistribution_FairlyUniform)
{
    constexpr int NODE_COUNT = 10;
    constexpr int VNODES = 150;
    constexpr int KEY_COUNT = 50000;
    constexpr double EXPECTED_PER_NODE =
        static_cast<double>(KEY_COUNT) / static_cast<double>(NODE_COUNT);

    ConsistentHash ring;
    for (int i = 0; i < NODE_COUNT; ++i)
    {
        ring.addNode("n-" + std::to_string(i), VNODES);
    }

    std::map<NodeName, uint64_t> counts;
    for (int i = 0; i < KEY_COUNT; ++i)
    {
        ++counts[ring.getNode("key-" + std::to_string(i))];
    }

    // Each node should get roughly KEY_COUNT / NODE_COUNT keys.
    // Allow a generous band of [60%, 140%] — perfect uniformity is not
    // guaranteed with a finite vnode count.
    for (const auto& [node, count] : counts)
    {
        const double ratio = static_cast<double>(count) / EXPECTED_PER_NODE;
        EXPECT_GT(ratio, 0.60)
            << "Node " << node << " has too few keys: " << count;
        EXPECT_LT(ratio, 1.40)
            << "Node " << node << " has too many keys: " << count;
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  10. Edge cases
// ══════════════════════════════════════════════════════════════════════════

TEST_F(ConsistentHashTest, EmptyKey_DoesNotCrash)
{
    ConsistentHash ring;
    ring.addNode("node-A", 10);
    EXPECT_NO_THROW(static_cast<void>(ring.getNode("")));
    EXPECT_NO_THROW(static_cast<void>(ring.getNode(std::string_view{})));
}

TEST_F(ConsistentHashTest, ZeroVirtualNodes_FallsBackToDefault)
{
    ConsistentHash ring;
    ring.addNode("node-A", 0);  // should use DEFAULT_VNODE_COUNT

    EXPECT_EQ(ring.nodeCount(), 1);
    EXPECT_EQ(ring.virtualNodeCount(), ConsistentHash::DEFAULT_VNODE_COUNT);
}

TEST_F(ConsistentHashTest, ManyNodes_ManyKeys_NoError)
{
    ConsistentHash ring;

    // 300 physical nodes, each with 50 virtual replicas
    for (int i = 0; i < 300; ++i)
    {
        ring.addNode("srv-" + std::to_string(i), 50);
    }
    ASSERT_EQ(ring.nodeCount(), 300);
    ASSERT_EQ(ring.virtualNodeCount(), 300 * 50);

    // Route 100k keys
    for (int i = 0; i < 100000; ++i)
    {
        const auto node = ring.getNode("data-" + std::to_string(i));
        ASSERT_FALSE(node.empty());
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  11. Thread safety
// ══════════════════════════════════════════════════════════════════════════

TEST_F(ConsistentHashTest, ConcurrentReads_NoCrash)
{
    ConsistentHash ring;
    for (int i = 0; i < 10; ++i)
    {
        ring.addNode("node-" + std::to_string(i), 100);
    }

    std::atomic<uint64_t> successCount{0};
    constexpr int THREADS = 8;
    constexpr int OPS_PER_THREAD = 5000;

    auto worker = [&]()
    {
        for (int i = 0; i < OPS_PER_THREAD; ++i)
        {
            const auto key = "concurrent-key-" + std::to_string(i);
            const auto node = ring.getNode(key);
            // Just ensure it returns something non-empty
            if (!node.empty())
            {
                successCount.fetch_add(1, std::memory_order_relaxed);
            }
        }
    };

    std::vector<std::thread> threads;
    for (int t = 0; t < THREADS; ++t)
    {
        threads.emplace_back(worker);
    }
    for (auto& t : threads)
    {
        t.join();
    }

    EXPECT_EQ(successCount.load(), THREADS * OPS_PER_THREAD);
}

TEST_F(ConsistentHashTest, ConcurrentReadWrite_NoCrash)
{
    ConsistentHash ring;

    // Pre-populate with some nodes
    for (int i = 0; i < 5; ++i)
    {
        ring.addNode("pre-" + std::to_string(i), 50);
    }

    std::atomic<bool> stop{false};
    std::atomic<uint64_t> readOk{0};

    // Reader threads
    auto reader = [&]()
    {
        while (!stop.load(std::memory_order_acquire))
        {
            for (int i = 0; i < 100; ++i)
            {
                const auto key = "key-" + std::to_string(i);
                try
                {
                    const auto n = ring.getNode(key);
                    if (!n.empty()) readOk.fetch_add(1);
                }
                catch (...)
                {
                    // Acceptable during concurrent modification
                }
            }
        }
    };

    // Writer thread
    auto writer = [&]()
    {
        for (int i = 0; i < 20; ++i)
        {
            ring.addNode("dyn-" + std::to_string(i), 30);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            (void)ring.removeNode("dyn-" + std::to_string(i));
        }
        stop.store(true, std::memory_order_release);
    };

    std::vector<std::thread> readers;
    for (int t = 0; t < 4; ++t)
    {
        readers.emplace_back(reader);
    }

    std::thread writerThread(writer);

    for (auto& t : readers)
    {
        t.join();
    }
    writerThread.join();

    // We expect at least some reads to have completed without crash
    EXPECT_GT(readOk.load(), 0);
}

// ══════════════════════════════════════════════════════════════════════════
//  12. Interface compliance
// ══════════════════════════════════════════════════════════════════════════

TEST_F(ConsistentHashTest, Interface_AddGetRemove)
{
    // Verify the full lifecycle through the interface
    std::unique_ptr<IHashRing> ring = std::make_unique<ConsistentHash>();

    ring->addNode("srv-A", 100);
    ring->addNode("srv-B", 100);
    EXPECT_EQ(ring->nodeCount(), 2);

    const auto node = ring->getNode("some-key");
    EXPECT_TRUE(node == "srv-A" || node == "srv-B");

    const auto replicas = ring->getNodes("some-key", 2);
    EXPECT_EQ(replicas.size(), 2);
    EXPECT_NE(replicas[0], replicas[1]);

    (void)ring->removeNode("srv-A");
    EXPECT_EQ(ring->nodeCount(), 1);

    ring->clear();
    EXPECT_EQ(ring->nodeCount(), 0);
}

// ══════════════════════════════════════════════════════════════════════════
//  13. Move semantics
// ══════════════════════════════════════════════════════════════════════════

TEST_F(ConsistentHashTest, MoveConstructor_TransfersState)
{
    ConsistentHash original;
    original.addNode("node-A", 10);
    original.addNode("node-B", 20);
    ASSERT_EQ(original.nodeCount(), 2);

    ConsistentHash moved(std::move(original));

    EXPECT_EQ(moved.nodeCount(), 2);
    EXPECT_EQ(moved.virtualNodeCount(), 30);
    EXPECT_EQ(moved.getNode("test-key"), moved.getNode("test-key"));

    // Original should be empty
    EXPECT_EQ(original.nodeCount(), 0);
}

TEST_F(ConsistentHashTest, MoveAssignment_TransfersState)
{
    ConsistentHash first;
    first.addNode("first-node", 10);

    ConsistentHash second;
    second.addNode("second-node", 20);

    second = std::move(first);

    EXPECT_EQ(second.nodeCount(), 1);
    EXPECT_EQ(second.virtualNodeCount(), 10);

    EXPECT_EQ(first.nodeCount(), 0);
}

// ══════════════════════════════════════════════════════════════════════════
//  14. Multiple calls to removeNode (idempotency)
// ══════════════════════════════════════════════════════════════════════════

TEST_F(ConsistentHashTest, RemoveNode_Idempotent)
{
    ConsistentHash ring;
    ring.addNode("node-X", 5);

    EXPECT_TRUE(ring.removeNode("node-X"));
    EXPECT_FALSE(ring.removeNode("node-X"));  // second removal should fail
    EXPECT_EQ(ring.nodeCount(), 0);
}
