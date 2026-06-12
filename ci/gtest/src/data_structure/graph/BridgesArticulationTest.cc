#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/BridgesArticulation.hpp"

using namespace common::data_structure::graph;
using namespace common::data_structure::graph::algorithm;

class BridgesArticulationTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(BridgesArticulationTest, Compute_SimpleBridge)
{
    Graph g(3);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(0, 2, 1);

    const auto result = BridgesArticulation::compute(g);
    EXPECT_EQ(result.bridges.size(), static_cast<size_t>(2));
}

TEST_F(BridgesArticulationTest, Compute_NoBridges)
{
    Graph g(3);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(1, 2, 1);
    g.addUndirectedEdge(2, 0, 1);

    const auto result = BridgesArticulation::compute(g);
    EXPECT_EQ(result.bridges.size(), static_cast<size_t>(0));
}

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

TEST_F(BridgesArticulationTest, Compute_LineGraph)
{
    Graph g(4);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(1, 2, 1);
    g.addUndirectedEdge(2, 3, 1);

    const auto result = BridgesArticulation::compute(g);
    EXPECT_EQ(result.bridges.size(), static_cast<size_t>(3));
}

TEST_F(BridgesArticulationTest, Compute_EmptyGraph)
{
    const Graph g(0);
    const auto result = BridgesArticulation::compute(g);
    EXPECT_TRUE(result.bridges.empty());
    EXPECT_TRUE(result.articulation_points.empty());
}
