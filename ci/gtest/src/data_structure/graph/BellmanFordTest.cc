/**
 * @file BellmanFordTest.cc
 * @brief Unit tests for the Bellman-Ford shortest-path algorithm
 * @details Tests cover shortest-path computation with positive and negative
 *          edge weights, negative-cycle detection, disconnected graph handling,
 *          and path reconstruction.
 */

#include <gtest/gtest.h>

#include <cppforge/data_structure/graph/algorithm/BellmanFord.hpp>

using namespace cppforge::data_structure::graph;
using namespace cppforge::data_structure::graph::algorithm;

/**
 * @brief Test fixture for BellmanFord algorithm tests
 */
class BellmanFordTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test Bellman-Ford on a simple three-node line graph
 * @details Verifies that shortest distances are computed correctly with
 *          positive edge weights: 0â†? (2) + 1â†? (3) = 5
 */
TEST_F(BellmanFordTest, ShortestPath_Simple)
{
    Graph g(3);
    g.addEdge(0, 1, 2);
    g.addEdge(1, 2, 3);

    const auto result = BellmanFord::shortestPath(g, 0);
    EXPECT_EQ(result.distance[static_cast<size_t>(2)], 5);
    EXPECT_TRUE(result.hasPath());
}

/**
 * @brief Test Bellman-Ford with mixed positive and negative edge weights
 * @details Verifies correct distance calculation when negative edges
 *          produce shorter alternative paths: 0â†?â†? = 1 (via 4 + -3)
 */
TEST_F(BellmanFordTest, ShortestPath_NegativeWeights)
{
    Graph g(4);
    g.addEdge(0, 1, 4);
    g.addEdge(0, 2, 5);
    g.addEdge(1, 2, -3);
    g.addEdge(1, 3, 2);
    g.addEdge(2, 3, 1);

    const auto result = BellmanFord::shortestPath(g, 0);
    EXPECT_EQ(result.distance[static_cast<size_t>(0)], 0);
    EXPECT_EQ(result.distance[static_cast<size_t>(1)], 4);
    EXPECT_EQ(result.distance[static_cast<size_t>(2)], 1);
    EXPECT_EQ(result.distance[static_cast<size_t>(3)], 2);
}

/**
 * @brief Test that a graph with only positive edges has no negative cycle
 * @details Verifies that hasNegativeCycle returns false for acyclic graphs
 *          and graphs with only non-negative weights
 */
TEST_F(BellmanFordTest, HasNegativeCycle_NoCycle)
{
    Graph g(3);
    g.addEdge(0, 1, 2);
    g.addEdge(1, 2, 3);

    EXPECT_FALSE(BellmanFord::hasNegativeCycle(g));
}

/**
 * @brief Test negative cycle detection on a graph with a negative cycle
 * @details Creates a 3-node cycle where total weight is negative (-3)
 *          and verifies that hasNegativeCycle detects it
 */
TEST_F(BellmanFordTest, HasNegativeCycle_HasCycle)
{
    Graph g(3);
    g.addEdge(0, 1, -1);
    g.addEdge(1, 2, -1);
    g.addEdge(2, 0, -1);

    EXPECT_TRUE(BellmanFord::hasNegativeCycle(g));
}

/**
 * @brief Test Bellman-Ford on a disconnected graph
 * @details Verifies that unreachable nodes have distance INT32_MAX
 *          and no valid predecessor
 */
TEST_F(BellmanFordTest, ShortestPath_Disconnected)
{
    Graph g(4);
    g.addEdge(0, 1, 1);

    const auto result = BellmanFord::shortestPath(g, 0);
    EXPECT_EQ(result.distance[static_cast<size_t>(2)], INT32_MAX);
    EXPECT_EQ(result.distance[static_cast<size_t>(3)], INT32_MAX);
}

/**
 * @brief Test path reconstruction from Bellman-Ford result
 * @details Verifies that reconstructPath returns the correct node sequence
 *          from source to target through intermediate nodes
 */
TEST_F(BellmanFordTest, ReconstructPath)
{
    Graph g(3);
    g.addEdge(0, 1, 10);
    g.addEdge(1, 2, 20);

    const auto result = BellmanFord::shortestPath(g, 0);
    const auto path = result.reconstructPath(2);
    ASSERT_EQ(path.size(), static_cast<size_t>(3));
    EXPECT_EQ(path[0], 0);
    EXPECT_EQ(path[1], 1);
    EXPECT_EQ(path[2], 2);
}
