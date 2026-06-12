#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/Dijkstra.hpp"

using namespace common::data_structure::graph;
using namespace common::data_structure::graph::algorithm;

class DijkstraTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(DijkstraTest, ShortestPath_Simple)
{
    Graph g(3);
    g.addEdge(0, 1, 2);
    g.addEdge(1, 2, 3);

    const auto result = Dijkstra::shortestPath(g, 0);
    EXPECT_EQ(result.distance[static_cast<size_t>(2)], 5);
    EXPECT_TRUE(result.hasPath());
}

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

TEST_F(DijkstraTest, ShortestPath_NoPath)
{
    Graph g(4);
    g.addEdge(0, 1, 1);

    const auto result = Dijkstra::shortestPath(g, 0);
    EXPECT_EQ(result.distance[static_cast<size_t>(3)], INT32_MAX);
    EXPECT_EQ(result.previous[static_cast<size_t>(3)], -1);
}

TEST_F(DijkstraTest, ShortestPath_SameNode)
{
    Graph g(3);
    g.addEdge(0, 1, 5);

    const auto result = Dijkstra::shortestPath(g, 0);
    EXPECT_EQ(result.distance[static_cast<size_t>(0)], 0);
}

TEST_F(DijkstraTest, ShortestPath_ToTarget)
{
    Graph g(3);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 2);

    const auto result = Dijkstra::shortestPathToTarget(g, 0, 2);
    EXPECT_EQ(result.target, 2);
    EXPECT_EQ(result.distance[static_cast<size_t>(2)], 3);
}

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
