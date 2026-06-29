/**
 * @file BridgesArticulationTest.cc
 * @brief Unit tests for bridges and articulation points detection in undirected graphs
 * @details Tests cover bridge detection in star and line graphs, graphs with
 *          and without bridges, articulation point identification, and empty graphs.
 */

#include <gtest/gtest.h>

#include <cppforge/data_structure/graph/algorithm/BridgesArticulation.hpp>

using namespace cppforge::data_structure::graph;
using namespace cppforge::data_structure::graph::algorithm;

/**
 * @brief Test fixture for BridgesArticulation tests
 */
class BridgesArticulationTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test bridge detection on a star graph
 * @details A 3-node star (center 0 connected to 1 and 2) has 2 bridges
 */
TEST_F(BridgesArticulationTest, Compute_SimpleBridge)
{
    Graph g(3);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(0, 2, 1);

    const auto result = BridgesArticulation::compute(g);
    EXPECT_EQ(result.bridges.size(), static_cast<size_t>(2));
}

/**
 * @brief Test bridge detection on a triangle graph
 * @details A 3-node triangle has no bridges (every edge lies on a cycle)
 */
TEST_F(BridgesArticulationTest, Compute_NoBridges)
{
    Graph g(3);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(1, 2, 1);
    g.addUndirectedEdge(2, 0, 1);

    const auto result = BridgesArticulation::compute(g);
    EXPECT_EQ(result.bridges.size(), static_cast<size_t>(0));
}

/**
 * @brief Test articulation point detection
 * @details In a star-like graph (0-1-2, 1-3), node 1 is an articulation point
 *          because removing it disconnects the graph
 */
TEST_F(BridgesArticulationTest, Compute_ArticulationPoint)
{
    Graph g(4);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(1, 2, 1);
    g.addUndirectedEdge(1, 3, 1);

    const auto result = BridgesArticulation::compute(g);
    EXPECT_FALSE(result.articulation_points.empty());
    bool found_one = false;
    for (const auto p : result.articulation_points)
    {
        if (p == 1) { found_one = true; break; }
    }
    EXPECT_TRUE(found_one);
}

/**
 * @brief Test bridge detection on a line graph
 * @details A 4-node line graph has 3 bridges (every edge is a bridge)
 */
TEST_F(BridgesArticulationTest, Compute_LineGraph)
{
    Graph g(4);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(1, 2, 1);
    g.addUndirectedEdge(2, 3, 1);

    const auto result = BridgesArticulation::compute(g);
    EXPECT_EQ(result.bridges.size(), static_cast<size_t>(3));
}

/**
 * @brief Test bridges and articulation points on an empty graph
 * @details Edge case: empty graph has no bridges and no articulation points
 */
TEST_F(BridgesArticulationTest, Compute_EmptyGraph)
{
    const Graph g(0);
    const auto result = BridgesArticulation::compute(g);
    EXPECT_TRUE(result.bridges.empty());
    EXPECT_TRUE(result.articulation_points.empty());
}
