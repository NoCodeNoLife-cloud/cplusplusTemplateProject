#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/BipartiteCheck.hpp"

using namespace common::data_structure::graph;
using namespace common::data_structure::graph::algorithm;

class BipartiteCheckTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

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

TEST_F(BipartiteCheckTest, Check_NonBipartiteOddCycle)
{
    Graph g(3);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(1, 2, 1);
    g.addUndirectedEdge(2, 0, 1);

    const auto result = BipartiteCheck::check(g);
    EXPECT_FALSE(result.is_bipartite);
}

TEST_F(BipartiteCheckTest, Check_Tree)
{
    Graph g(4);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(1, 2, 1);
    g.addUndirectedEdge(1, 3, 1);

    const auto result = BipartiteCheck::check(g);
    EXPECT_TRUE(result.is_bipartite);
}

TEST_F(BipartiteCheckTest, Check_DisconnectedBipartite)
{
    Graph g(6);
    g.addUndirectedEdge(0, 1, 1);
    g.addUndirectedEdge(2, 3, 1);
    g.addUndirectedEdge(4, 5, 1);

    const auto result = BipartiteCheck::check(g);
    EXPECT_TRUE(result.is_bipartite);
}

TEST_F(BipartiteCheckTest, Check_SingleNode)
{
    Graph g(1);
    const auto result = BipartiteCheck::check(g);
    EXPECT_TRUE(result.is_bipartite);
}

TEST_F(BipartiteCheckTest, Check_EmptyGraph)
{
    const Graph g(0);
    const auto result = BipartiteCheck::check(g);
    EXPECT_TRUE(result.is_bipartite);
}
