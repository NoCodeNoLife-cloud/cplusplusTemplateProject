#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/TopologicalSort.hpp"

using namespace common::data_structure::graph;
using namespace common::data_structure::graph::algorithm;

class TopologicalSortTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

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

TEST_F(TopologicalSortTest, Sort_SingleNode)
{
    Graph g(1);
    const auto result = TopologicalSort::sort(g);
    EXPECT_FALSE(result.has_cycle);
    ASSERT_EQ(result.order.size(), static_cast<size_t>(1));
    EXPECT_EQ(result.order[0], 0);
}

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
