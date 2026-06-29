/**
 * @file DFSTest.cc
 * @brief Unit tests for the DFS (Depth-First Search) algorithm
 * @details Tests cover DFS graph traversal, cycle handling, disconnected
 *          components, path finding, and the visitor callback pattern.
 */

#include <gtest/gtest.h>

#include <cppforge/data_structure/graph/algorithm/DFS.hpp>

using namespace cppforge::data_structure::graph;
using namespace cppforge::data_structure::graph::algorithm;

/**
 * @brief Test fixture for DFS algorithm tests
 */
class DFSTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test DFS traversal on a simple linear graph
 * @details Verifies that DFS visits nodes in depth-first order along a line
 *          graph: 0â†?â†?â†?
 */
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

/**
 * @brief Test DFS traversal on a graph with a cycle
 * @details Verifies that DFS handles cycles gracefully by not revisiting
 *          already-visited nodes; all 3 nodes are visited once
 */
TEST_F(DFSTest, Traverse_WithCycle)
{
    Graph g(3);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 0, 1);

    const auto result = DFS::traverse(g, 0);
    EXPECT_EQ(result.order.size(), static_cast<size_t>(3));
}

/**
 * @brief Test DFS traversal on a disconnected graph
 * @details Verifies that only the connected component reachable from the
 *          start node is visited
 */
TEST_F(DFSTest, Traverse_DisconnectedGraph)
{
    Graph g(4);
    g.addEdge(0, 1, 1);

    const auto result = DFS::traverse(g, 0);
    ASSERT_EQ(result.order.size(), static_cast<size_t>(2));
    EXPECT_EQ(result.order[0], 0);
    EXPECT_EQ(result.order[1], 1);
}

/**
 * @brief Test DFS traversal covering all components
 * @details Verifies that traverseAll visits all nodes across disconnected
 *          components of the graph
 */
TEST_F(DFSTest, TraverseAll_Disconnected)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(2, 3, 1);

    const auto result = DFS::traverseAll(g);
    EXPECT_EQ(result.order.size(), static_cast<size_t>(4));
}

/**
 * @brief Test DFS path finding on a simple line graph
 * @details Verifies that findPath returns a valid path from source to target
 */
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

/**
 * @brief Test DFS path finding when no path exists
 * @details Verifies that an empty path is returned for unreachable target nodes
 */
TEST_F(DFSTest, FindPath_NoPath)
{
    Graph g(4);
    g.addEdge(0, 1, 1);

    const auto path = DFS::findPath(g, 0, 3);
    EXPECT_TRUE(path.empty());
}

/**
 * @brief Test DFS traversal with a custom visitor
 * @details Verifies that the visitor's onNodeDiscover and onNodeFinish
 *          callbacks are invoked correctly during traversal
 */
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
