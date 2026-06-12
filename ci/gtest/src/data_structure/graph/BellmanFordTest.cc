#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/BellmanFord.hpp"

using namespace common::data_structure::graph;
using namespace common::data_structure::graph::algorithm;

class BellmanFordTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(BellmanFordTest, ShortestPath_Simple)
{
    Graph g(3);
    g.addEdge(0, 1, 2);
    g.addEdge(1, 2, 3);

    const auto result = BellmanFord::shortestPath(g, 0);
    EXPECT_EQ(result.distance[static_cast<size_t>(2)], 5);
    EXPECT_TRUE(result.hasPath());
}

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

TEST_F(BellmanFordTest, HasNegativeCycle_NoCycle)
{
    Graph g(3);
    g.addEdge(0, 1, 2);
    g.addEdge(1, 2, 3);

    EXPECT_FALSE(BellmanFord::hasNegativeCycle(g));
}

TEST_F(BellmanFordTest, HasNegativeCycle_HasCycle)
{
    Graph g(3);
    g.addEdge(0, 1, -1);
    g.addEdge(1, 2, -1);
    g.addEdge(2, 0, -1);

    EXPECT_TRUE(BellmanFord::hasNegativeCycle(g));
}

TEST_F(BellmanFordTest, ShortestPath_Disconnected)
{
    Graph g(4);
    g.addEdge(0, 1, 1);

    const auto result = BellmanFord::shortestPath(g, 0);
    EXPECT_EQ(result.distance[static_cast<size_t>(2)], INT32_MAX);
    EXPECT_EQ(result.distance[static_cast<size_t>(3)], INT32_MAX);
}

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
