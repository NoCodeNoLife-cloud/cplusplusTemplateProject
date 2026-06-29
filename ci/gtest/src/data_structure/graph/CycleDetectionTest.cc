/**
 * @file CycleDetectionTest.cc
 * @brief Unit tests for cycle detection algorithms in directed and undirected graphs
 * @details Tests cover directed cycle detection (DFS-based), undirected cycle
 *          detection (union-find / DFS-based), self-loop handling, and edge cases
 *          such as empty and single-node graphs.
 */

#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/CycleDetection.hpp"

using namespace cppforge::data_structure::graph;
using namespace cppforge::data_structure::graph::algorithm;

/**
 * @brief Test fixture for CycleDetection tests
 */
class CycleDetectionTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test directed cycle detection on an acyclic graph
 * @details Verifies that a linear DAG (0â†?â†?â†?) has no cycles
 */
TEST_F(CycleDetectionTest, DetectDirected_NoCycle)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);

    const auto result = CycleDetection::detectDirected(g);
    EXPECT_FALSE(result.has_cycle);
    EXPECT_TRUE(result.cycle.empty());
}

/**
 * @brief Test directed cycle detection on a graph with a cycle
 * @details Verifies that a 3-node directed cycle (0â†?â†?â†?) is correctly detected
 */
TEST_F(CycleDetectionTest, DetectDirected_HasCycle)
{
    Graph g(3);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 0, 1);

    const auto result = CycleDetection::detectDirected(g);
    EXPECT_TRUE(result.has_cycle);
    EXPECT_FALSE(result.cycle.empty());
}

/**
 * @brief Test directed cycle detection with a self-loop
 * @details Verifies that a self-loop edge (0â†?) is detected as a cycle
 */
TEST_F(CycleDetectionTest, DetectDirected_SelfLoop)
{
    Graph g(2);
    g.addEdge(0, 0, 1);

    const auto result = CycleDetection::detectDirected(g);
    EXPECT_TRUE(result.has_cycle);
    EXPECT_FALSE(result.cycle.empty());
}

/**
 * @brief Test undirected cycle detection on an acyclic graph
 * @details Verifies that a simple undirected path has no cycles
 */
TEST_F(CycleDetectionTest, DetectUndirected_NoCycle)
{
    Graph g(4);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(1, 2, 1);
    g.addUndirectedEdge(2, 3, 1);

    const auto result = CycleDetection::detectUndirected(g);
    EXPECT_FALSE(result.has_cycle);
    EXPECT_TRUE(result.cycle.empty());
}

/**
 * @brief Test undirected cycle detection on a graph with a cycle
 * @details Verifies that a 3-node undirected triangle has a cycle
 */
TEST_F(CycleDetectionTest, DetectUndirected_HasCycle)
{
    Graph g(3);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(1, 2, 1);
    g.addUndirectedEdge(2, 0, 1);

    const auto result = CycleDetection::detectUndirected(g);
    EXPECT_TRUE(result.has_cycle);
    EXPECT_FALSE(result.cycle.empty());
}

/**
 * @brief Test directed cycle detection on an empty graph
 * @details Edge case: verifies that a graph with zero nodes has no cycles
 */
TEST_F(CycleDetectionTest, DetectDirected_EmptyGraph)
{
    const Graph g(0);
    const auto result = CycleDetection::detectDirected(g);
    EXPECT_FALSE(result.has_cycle);
}

/**
 * @brief Test directed cycle detection on a single-node graph
 * @details Edge case: verifies that a graph with one node and no edges has no cycles
 */
TEST_F(CycleDetectionTest, DetectDirected_SingleNode)
{
    Graph g(1);
    const auto result = CycleDetection::detectDirected(g);
    EXPECT_FALSE(result.has_cycle);
}
