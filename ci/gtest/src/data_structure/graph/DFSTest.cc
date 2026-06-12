#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/DFS.hpp"

using namespace common::data_structure::graph;
using namespace common::data_structure::graph::algorithm;

class DFSTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(DFSTest, Traverse_SimpleLine)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);

    const auto result = DFS::traverse(g, 0);
    ASSERT_EQ(result.order.size(), static_cast<size_t>(4));
    EXPECT_EQ(result.order[0], 0);
    EXPECT_EQ(result.order[1], 1);
    EXPECT_EQ(result.order[2], 2);
    EXPECT_EQ(result.order[3], 3);
}

TEST_F(DFSTest, Traverse_WithCycle)
{
    Graph g(3);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 0, 1);

    const auto result = DFS::traverse(g, 0);
    EXPECT_EQ(result.order.size(), static_cast<size_t>(3));
}

TEST_F(DFSTest, Traverse_DisconnectedGraph)
{
    Graph g(4);
    g.addEdge(0, 1, 1);

    const auto result = DFS::traverse(g, 0);
    ASSERT_EQ(result.order.size(), static_cast<size_t>(2));
    EXPECT_EQ(result.order[0], 0);
    EXPECT_EQ(result.order[1], 1);
}

TEST_F(DFSTest, TraverseAll_Disconnected)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(2, 3, 1);

    const auto result = DFS::traverseAll(g);
    EXPECT_EQ(result.order.size(), static_cast<size_t>(4));
}

TEST_F(DFSTest, FindPath_SimpleLine)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);

    const auto path = DFS::findPath(g, 0, 3);
    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front(), 0);
    EXPECT_EQ(path.back(), 3);
}

TEST_F(DFSTest, FindPath_NoPath)
{
    Graph g(4);
    g.addEdge(0, 1, 1);

    const auto path = DFS::findPath(g, 0, 3);
    EXPECT_TRUE(path.empty());
}

TEST_F(DFSTest, Traverse_WithVisitor)
{
    Graph g(3);
    g.addEdge(0, 1, 5);
    g.addEdge(1, 2, 10);

    class TestVisitor : public IGraphVisitor
    {
    public:
        std::vector<int32_t> discovered;
        std::vector<int32_t> finished;

        void onNodeDiscover(int32_t node) override
        {
            discovered.push_back(node);
        }

        void onNodeFinish(int32_t node) override
        {
            finished.push_back(node);
        }
    };

    TestVisitor visitor;
    DFS::traverse(g, 0, &visitor);

    EXPECT_FALSE(visitor.discovered.empty());
    EXPECT_EQ(visitor.discovered.size(), visitor.finished.size());
}
