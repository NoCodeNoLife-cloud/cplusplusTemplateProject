/**
 * @file DijkstraTest.cc
 * @brief Unit tests for Dijkstra's shortest-path algorithm
 * @details Tests cover single-source shortest-path computation on graphs
 *          with non-negative weights, disconnected graphs, single-node
 *          edge cases, target-specific queries, and path reconstruction.
 */

#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/Dijkstra.hpp"

using namespace cppforge::data_structure::graph;
using namespace cppforge::data_structure::graph::algorithm;

/**
 * @brief Test fixture for Dijkstra algorithm tests
 */
class DijkstraTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test Dijkstra on a simple three-node line graph
 * @details Verifies shortest distances: 0â†? (2) + 1â†? (3) = 5
 */
TEST_F(DijkstraTest, ShortestPath_Simple)
{
    Graph g(3);
    g.addEdge(0, 1, 2);
    g.addEdge(1, 2, 3);

    const auto result = Dijkstra::shortestPath(g, 0);
    EXPECT_EQ(result.distance[static_cast<size_t>(2)], 5);
    EXPECT_TRUE(result.hasPath());
}

/**
 * @brief Test Dijkstra on a graph with multiple alternative paths
 * @details Verifies that Dijkstra finds the optimal path 0â†?â†?â†? (total 4)
 *          over the more direct but expensive 0â†?â†? path
 */
TEST_F(DijkstraTest, ShortestPath_Complex)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(0, 2, 4);
    g.addEdge(1, 2, 2);
    g.addEdge(1, 3, 6);
    g.addEdge(2, 3, 1);

    const auto result = Dijkstra::shortestPath(g, 0);
    EXPECT_EQ(result.distance[static_cast<size_t>(0)], 0);
    EXPECT_EQ(result.distance[static_cast<size_t>(1)], 1);
    EXPECT_EQ(result.distance[static_cast<size_t>(2)], 3);
    EXPECT_EQ(result.distance[static_cast<size_t>(3)], 4);
}

/**
 * @brief Test Dijkstra on a disconnected graph
 * @details Verifies that unreachable nodes have distance INT32_MAX
 *          and no valid predecessor (-1)
 */
TEST_F(DijkstraTest, ShortestPath_NoPath)
{
    Graph g(4);
    g.addEdge(0, 1, 1);

    const auto result = Dijkstra::shortestPath(g, 0);
    EXPECT_EQ(result.distance[static_cast<size_t>(3)], INT32_MAX);
    EXPECT_EQ(result.previous[static_cast<size_t>(3)], -1);
}

/**
 * @brief Test Dijkstra from a node to itself
 * @details Edge case: verifies that distance to source node is always 0
 */
TEST_F(DijkstraTest, ShortestPath_SameNode)
{
    Graph g(3);
    g.addEdge(0, 1, 5);

    const auto result = Dijkstra::shortestPath(g, 0);
    EXPECT_EQ(result.distance[static_cast<size_t>(0)], 0);
}

/**
 * @brief Test Dijkstra with target-specific early termination
 * @details Verifies that shortestPathToTarget computes correctly with
 *          early termination when the target node is settled
 */
TEST_F(DijkstraTest, ShortestPath_ToTarget)
{
    Graph g(3);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 2);

    const auto result = Dijkstra::shortestPathToTarget(g, 0, 2);
    EXPECT_EQ(result.target, 2);
    EXPECT_EQ(result.distance[static_cast<size_t>(2)], 3);
}

/**
 * @brief Test path reconstruction from Dijkstra result
 * @details Verifies that reconstructPath returns the correct node sequence
 *          from source (0) through intermediate nodes to target (3)
 */
TEST_F(DijkstraTest, ReconstructPath)
{
    Graph g(4);
    g.addEdge(0, 1, 2);
    g.addEdge(1, 2, 3);
    g.addEdge(2, 3, 1);

    const auto result = Dijkstra::shortestPath(g, 0);
    const auto path = result.reconstructPath(3);
    ASSERT_EQ(path.size(), static_cast<size_t>(4));
    EXPECT_EQ(path[0], 0);
    EXPECT_EQ(path[1], 1);
    EXPECT_EQ(path[2], 2);
    EXPECT_EQ(path[3], 3);
}
