/**
 * @file KruskalMSTTest.cc
 * @brief Unit tests for Kruskal's Minimum Spanning Tree algorithm
 * @details Tests cover MST computation on simple graphs, equal-weight
 *          edge handling, single-node edge cases, and empty graphs.
 */

#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/KruskalMST.hpp"

using namespace common::data_structure::graph;
using namespace common::data_structure::graph::algorithm;

/**
 * @brief Test fixture for KruskalMST tests
 */
class KruskalMSTTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test Kruskal's algorithm on a simple 4-node graph
 * @details Verifies that the MST selects the 3 cheapest edges,
 *          total weight = 6, excluding the expensive edge (0→3, weight 10)
 */
TEST_F(KruskalMSTTest, Compute_SimpleGraph)
{
    Graph g(4);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(1, 2, 2);
    g.addUndirectedEdge(2, 3, 3);
    g.addUndirectedEdge(0, 3, 10);

    const auto result = KruskalMST::compute(g);
    EXPECT_EQ(result.total_weight, 6);
    EXPECT_EQ(result.edges.size(), static_cast<size_t>(3));
}

/**
 * @brief Test Kruskal's algorithm on a single-node graph
 * @details Edge case: MST of a single node has zero weight and no edges
 */
TEST_F(KruskalMSTTest, Compute_SingleNode)
{
    Graph g(1);
    const auto result = KruskalMST::compute(g);
    EXPECT_EQ(result.total_weight, 0);
    EXPECT_TRUE(result.edges.empty());
}

/**
 * @brief Test Kruskal's algorithm on a graph with all equal-weight edges
 * @details Verifies that the MST picks exactly 3 edges with total weight 3
 *          when all edges have weight 1
 */
TEST_F(KruskalMSTTest, Compute_EqualWeights)
{
    Graph g(4);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(1, 2, 1);
    g.addUndirectedEdge(2, 3, 1);
    g.addUndirectedEdge(0, 3, 1);
    g.addUndirectedEdge(0, 2, 1);
    g.addUndirectedEdge(1, 3, 1);

    const auto result = KruskalMST::compute(g);
    EXPECT_EQ(result.edges.size(), static_cast<size_t>(3));
    EXPECT_EQ(result.total_weight, 3);
}

/**
 * @brief Test Kruskal's algorithm on an empty graph
 * @details Edge case: MST of an empty graph has zero weight and no edges
 */
TEST_F(KruskalMSTTest, Compute_EmptyGraph)
{
    const Graph g(0);
    const auto result = KruskalMST::compute(g);
    EXPECT_EQ(result.total_weight, 0);
    EXPECT_TRUE(result.edges.empty());
}
