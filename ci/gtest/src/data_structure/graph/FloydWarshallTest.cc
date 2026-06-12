#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/FloydWarshall.hpp"

using namespace common::data_structure::graph;
using namespace common::data_structure::graph::algorithm;

class FloydWarshallTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(FloydWarshallTest, Compute_SimpleGraph)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 2);
    g.addEdge(2, 3, 3);

    const auto result = FloydWarshall::compute(g);
    EXPECT_FALSE(result.has_negative_cycle);
    EXPECT_EQ(result.distances[0][0], 0);
    EXPECT_EQ(result.distances[0][3], 6);
    EXPECT_EQ(result.distances[2][3], 3);
}

TEST_F(FloydWarshallTest, Compute_Disconnected)
{
    Graph g(4);
    g.addEdge(0, 1, 1);

    const auto result = FloydWarshall::compute(g);
    EXPECT_EQ(result.distances[0][2], INT32_MAX / 2);
    EXPECT_EQ(result.distances[2][3], INT32_MAX / 2);
}

TEST_F(FloydWarshallTest, Compute_NegativeCycle)
{
    Graph g(2);
    g.addEdge(0, 1, -1);
    g.addEdge(1, 0, -1);

    const auto result = FloydWarshall::compute(g);
    EXPECT_TRUE(result.has_negative_cycle);
}

TEST_F(FloydWarshallTest, Compute_SingleNode)
{
    Graph g(1);
    const auto result = FloydWarshall::compute(g);
    EXPECT_FALSE(result.has_negative_cycle);
    EXPECT_EQ(result.distances[0][0], 0);
}

TEST_F(FloydWarshallTest, Compute_WithIntermediate)
{
    Graph g(3);
    g.addEdge(0, 1, 4);
    g.addEdge(0, 2, 10);
    g.addEdge(1, 2, 3);

    const auto result = FloydWarshall::compute(g);
    EXPECT_EQ(result.distances[0][1], 4);
    EXPECT_EQ(result.distances[0][2], 7);
    EXPECT_EQ(result.distances[1][2], 3);
}

TEST_F(FloydWarshallTest, ReconstructPath_Valid)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 2);
    g.addEdge(2, 3, 3);

    const auto result = FloydWarshall::compute(g);
    const auto path = result.reconstructPath(0, 3);
    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front(), 0);
    EXPECT_EQ(path.back(), 3);
}

TEST_F(FloydWarshallTest, ReconstructPath_NoPath)
{
    Graph g(4);
    g.addEdge(0, 1, 1);

    const auto result = FloydWarshall::compute(g);
    const auto path = result.reconstructPath(0, 3);
    EXPECT_TRUE(path.empty());
}
