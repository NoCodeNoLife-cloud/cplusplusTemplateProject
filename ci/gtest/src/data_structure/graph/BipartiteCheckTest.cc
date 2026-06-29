/**
 * @file BipartiteCheckTest.cc
 * @brief Unit tests for the bipartite graph checking algorithm
 * @details Tests cover even-cycle (bipartite), odd-cycle (non-bipartite),
 *          tree, disconnected, single-node, and empty graph scenarios.
 */

#include <gtest/gtest.h>

#include <cppforge/data_structure/graph/algorithm/BipartiteCheck.hpp>

using namespace cppforge::data_structure::graph;
using namespace cppforge::data_structure::graph::algorithm;

/**
 * @brief Test fixture for BipartiteCheck tests
 */
class BipartiteCheckTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test bipartite check on a graph with an even-length cycle
 * @details An even cycle (4 nodes) should be bipartite
 */
TEST_F(BipartiteCheckTest, Check_BipartiteEvenCycle)
{
    Graph g(4);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(1, 2, 1);
    g.addUndirectedEdge(2, 3, 1);
    g.addUndirectedEdge(3, 0, 1);

    const auto result = BipartiteCheck::check(g);
    EXPECT_TRUE(result.is_bipartite);
}

/**
 * @brief Test bipartite check on a graph with an odd-length cycle
 * @details An odd cycle (3-node triangle) should NOT be bipartite
 */
TEST_F(BipartiteCheckTest, Check_NonBipartiteOddCycle)
{
    Graph g(3);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(1, 2, 1);
    g.addUndirectedEdge(2, 0, 1);

    const auto result = BipartiteCheck::check(g);
    EXPECT_FALSE(result.is_bipartite);
}

/**
 * @brief Test bipartite check on a tree
 * @details All trees (acyclic graphs) are bipartite
 */
TEST_F(BipartiteCheckTest, Check_Tree)
{
    Graph g(4);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(1, 2, 1);
    g.addUndirectedEdge(1, 3, 1);

    const auto result = BipartiteCheck::check(g);
    EXPECT_TRUE(result.is_bipartite);
}

/**
 * @brief Test bipartite check on disconnected bipartite components
 * @details Multiple disconnected bipartite components should still pass
 */
TEST_F(BipartiteCheckTest, Check_DisconnectedBipartite)
{
    Graph g(6);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(2, 3, 1);
    g.addUndirectedEdge(4, 5, 1);

    const auto result = BipartiteCheck::check(g);
    EXPECT_TRUE(result.is_bipartite);
}

/**
 * @brief Test bipartite check on a single-node graph
 * @details Edge case: a graph with one node is trivially bipartite
 */
TEST_F(BipartiteCheckTest, Check_SingleNode)
{
    Graph g(1);
    const auto result = BipartiteCheck::check(g);
    EXPECT_TRUE(result.is_bipartite);
}

/**
 * @brief Test bipartite check on an empty graph
 * @details Edge case: a graph with zero nodes is trivially bipartite
 */
TEST_F(BipartiteCheckTest, Check_EmptyGraph)
{
    const Graph g(0);
    const auto result = BipartiteCheck::check(g);
    EXPECT_TRUE(result.is_bipartite);
}
