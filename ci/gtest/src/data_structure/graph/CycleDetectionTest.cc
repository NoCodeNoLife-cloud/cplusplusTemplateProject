#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/CycleDetection.hpp"

using namespace common::data_structure::graph;
using namespace common::data_structure::graph::algorithm;

class CycleDetectionTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

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

TEST_F(CycleDetectionTest, DetectDirected_SelfLoop)
{
    Graph g(2);
    g.addEdge(0, 0, 1);

    const auto result = CycleDetection::detectDirected(g);
    EXPECT_TRUE(result.has_cycle);
    EXPECT_FALSE(result.cycle.empty());
}

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

TEST_F(CycleDetectionTest, DetectDirected_EmptyGraph)
{
    const Graph g(0);
    const auto result = CycleDetection::detectDirected(g);
    EXPECT_FALSE(result.has_cycle);
}

TEST_F(CycleDetectionTest, DetectDirected_SingleNode)
{
    Graph g(1);
    const auto result = CycleDetection::detectDirected(g);
    EXPECT_FALSE(result.has_cycle);
}
