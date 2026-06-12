#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/PrimMST.hpp"

using namespace common::data_structure::graph;
using namespace common::data_structure::graph::algorithm;

class PrimMSTTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

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

TEST_F(PrimMSTTest, Compute_SingleNode)
{
    Graph g(1);
    const auto result = PrimMST::compute(g);
    EXPECT_EQ(result.total_weight, 0);
    EXPECT_TRUE(result.edges.empty());
}

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

TEST_F(PrimMSTTest, Compute_EmptyGraph)
{
    const Graph g(0);
    const auto result = PrimMST::compute(g);
    EXPECT_EQ(result.total_weight, 0);
    EXPECT_TRUE(result.edges.empty());
}
