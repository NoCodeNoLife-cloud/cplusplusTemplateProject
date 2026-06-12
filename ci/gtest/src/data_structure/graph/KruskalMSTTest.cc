#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/KruskalMST.hpp"

using namespace common::data_structure::graph;
using namespace common::data_structure::graph::algorithm;

class KruskalMSTTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

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

TEST_F(KruskalMSTTest, Compute_SingleNode)
{
    Graph g(1);
    const auto result = KruskalMST::compute(g);
    EXPECT_EQ(result.total_weight, 0);
    EXPECT_TRUE(result.edges.empty());
}

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

TEST_F(KruskalMSTTest, Compute_EmptyGraph)
{
    const Graph g(0);
    const auto result = KruskalMST::compute(g);
    EXPECT_EQ(result.total_weight, 0);
    EXPECT_TRUE(result.edges.empty());
}
