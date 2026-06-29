/**
 * @file PrimMSTTest.cc
 * @brief Unit tests for Prim's Minimum Spanning Tree algorithm
 * @details Tests cover MST computation on simple graphs, complete graphs,
 *          single-node edge cases, and empty graphs.
 */

#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/PrimMST.hpp"

using namespace cppforge::data_structure::graph;
using namespace cppforge::data_structure::graph::algorithm;

/**
 * @brief Test fixture for PrimMST tests
 */
class PrimMSTTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test Prim's algorithm on a simple 4-node graph
 * @details Verifies that the MST includes the 3 cheapest edges and
 *          excludes the expensive edge (0â†?, weight 10). Total weight = 6.
 */
TEST_F(PrimMSTTest, Compute_SimpleGraph)
{
    Graph g(4);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(1, 2, 2);
    g.addUndirectedEdge(2, 3, 3);
    g.addUndirectedEdge(0, 3, 10);

    const auto result = PrimMST::compute(g);
    EXPECT_EQ(result.total_weight, 6);
    EXPECT_EQ(result.edges.size(), static_cast<size_t>(3));
}

/**
 * @brief Test Prim's algorithm on a single-node graph
 * @details Edge case: MST of a single node has zero weight and no edges
 */
TEST_F(PrimMSTTest, Compute_SingleNode)
{
    Graph g(1);
    const auto result = PrimMST::compute(g);
    EXPECT_EQ(result.total_weight, 0);
    EXPECT_TRUE(result.edges.empty());
}

/**
 * @brief Test Prim's algorithm on a complete graph with 4 nodes
 * @details Verifies that the MST contains exactly (n-1) = 3 edges
 */
TEST_F(PrimMSTTest, Compute_CompleteGraph)
{
    Graph g(4);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(0, 2, 2);
    g.addUndirectedEdge(0, 3, 3);
    g.addUndirectedEdge(1, 2, 4);
    g.addUndirectedEdge(1, 3, 5);
    g.addUndirectedEdge(2, 3, 6);

    const auto result = PrimMST::compute(g);
    EXPECT_EQ(result.edges.size(), static_cast<size_t>(3));
    EXPECT_TRUE(result.total_weight >= 6);
}

/**
 * @brief Test Prim's algorithm on an empty graph
 * @details Edge case: MST of an empty graph has zero weight and no edges
 */
TEST_F(PrimMSTTest, Compute_EmptyGraph)
{
    const Graph g(0);
    const auto result = PrimMST::compute(g);
    EXPECT_EQ(result.total_weight, 0);
    EXPECT_TRUE(result.edges.empty());
}
