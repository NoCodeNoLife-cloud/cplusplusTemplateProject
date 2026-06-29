/**
 * @file TopologicalSortTest.cc
 * @brief Unit tests for topological sorting algorithms
 * @details Tests cover both DFS-based and Kahn's algorithm (BFS-based)
 *          topological sort implementations, including cycle detection,
 *          single-node edge cases, and complex DAG ordering invariants.
 */

#include <gtest/gtest.h>

#include <cppforge/data_structure/graph/algorithm/TopologicalSort.hpp>

using namespace cppforge::data_structure::graph;
using namespace cppforge::data_structure::graph::algorithm;

/**
 * @brief Test fixture for TopologicalSort tests
 */
class TopologicalSortTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test DFS-based topological sort on a simple DAG
 * @details Verifies that a linear DAG (0â†?â†?â†?) produces a valid topological order
 */
TEST_F(TopologicalSortTest, Sort_DAG)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);

    const auto result = TopologicalSort::sort(g);
    EXPECT_FALSE(result.has_cycle);
    ASSERT_EQ(result.order.size(), static_cast<size_t>(4));
    EXPECT_EQ(result.order[0], 0);
    EXPECT_EQ(result.order[3], 3);
}

/**
 * @brief Test DFS-based sort on a graph with a cycle
 * @details Verifies that has_cycle is set to true and order is empty
 *          when the graph contains a directed cycle
 */
TEST_F(TopologicalSortTest, Sort_HasCycle)
{
    Graph g(3);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 0, 1);

    const auto result = TopologicalSort::sort(g);
    EXPECT_TRUE(result.has_cycle);
    EXPECT_TRUE(result.order.empty());
}

/**
 * @brief Test Kahn's (BFS-based) topological sort on a simple DAG
 * @details Verifies that Kahn's algorithm produces a valid topological order
 *          for a linear DAG (0â†?â†?â†?)
 */
TEST_F(TopologicalSortTest, SortKahn_DAG)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);

    const auto result = TopologicalSort::sortKahn(g);
    EXPECT_FALSE(result.has_cycle);
    ASSERT_EQ(result.order.size(), static_cast<size_t>(4));
    EXPECT_EQ(result.order[0], 0);
    EXPECT_EQ(result.order[3], 3);
}

/**
 * @brief Test Kahn's sort on a graph with a cycle
 * @details Verifies that Kahn's algorithm detects cycles and returns
 *          an empty order when the graph contains a directed cycle
 */
TEST_F(TopologicalSortTest, SortKahn_HasCycle)
{
    Graph g(3);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 0, 1);

    const auto result1 = TopologicalSort::sortKahn(g);
    EXPECT_TRUE(result1.has_cycle);
    EXPECT_TRUE(result1.order.empty());
}

/**
 * @brief Test topological sort on a single-node graph
 * @details Edge case: verifies that a graph with one node produces
 *          a valid single-element topological order
 */
TEST_F(TopologicalSortTest, Sort_SingleNode)
{
    Graph g(1);
    const auto result = TopologicalSort::sort(g);
    EXPECT_FALSE(result.has_cycle);
    ASSERT_EQ(result.order.size(), static_cast<size_t>(1));
    EXPECT_EQ(result.order[0], 0);
}

/**
 * @brief Test topological sort on a complex DAG with multiple partial orders
 * @details Verifies that the output respects all edge dependencies:
 *          each edge (uâ†’v) implies u appears before v in the order
 */
TEST_F(TopologicalSortTest, Sort_ComplexDAG)
{
    Graph g(6);
    g.addEdge(5, 2, 1);
    g.addEdge(5, 0, 1);
    g.addEdge(4, 0, 1);
    g.addEdge(4, 1, 1);
    g.addEdge(2, 3, 1);
    g.addEdge(3, 1, 1);

    const auto result = TopologicalSort::sort(g);
    EXPECT_FALSE(result.has_cycle);
    EXPECT_EQ(result.order.size(), static_cast<size_t>(6));

    std::vector<int32_t> pos(6, -1);
    for (size_t i = 0; i < result.order.size(); ++i)
    {
        pos[static_cast<size_t>(result.order[i])] = static_cast<int32_t>(i);
    }

    EXPECT_LT(pos[5], pos[2]);
    EXPECT_LT(pos[5], pos[0]);
    EXPECT_LT(pos[4], pos[0]);
    EXPECT_LT(pos[4], pos[1]);
    EXPECT_LT(pos[2], pos[3]);
    EXPECT_LT(pos[3], pos[1]);
}
