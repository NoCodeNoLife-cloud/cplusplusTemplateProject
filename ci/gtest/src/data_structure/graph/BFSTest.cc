/**
 * @file BFSTest.cc
 * @brief Unit tests for the BFS (Breadth-First Search) algorithm
 * @details Tests cover BFS graph traversal, shortest path computation,
 *          disconnected graphs, single-node edge cases, and the visitor
 *          callback pattern.
 */

#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/BFS.hpp"

using namespace common::data_structure::graph;
using namespace common::data_structure::graph::algorithm;

/**
 * @brief Test fixture for BFS algorithm tests
 */
class BFSTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test BFS traversal on a simple linear graph
 * @details Verifies that BFS visits nodes in order 0→1→2→3 along a line graph
 */
TEST_F(BFSTest, Traverse_SimpleLine)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);

    const auto result = BFS::traverse(g, 0);
    ASSERT_EQ(result.order.size(), static_cast<size_t>(4));
    EXPECT_EQ(result.order[0], 0);
    EXPECT_EQ(result.order[1], 1);
    EXPECT_EQ(result.order[2], 2);
    EXPECT_EQ(result.order[3], 3);
}

/**
 * @brief Test BFS traversal on a star graph (center with multiple leaves)
 * @details Verifies that all reachable nodes from center are visited;
 *          leaf order may vary due to adjacency list iteration
 */
TEST_F(BFSTest, Traverse_StarFromCenter)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(0, 2, 1);
    g.addEdge(0, 3, 1);

    const auto result = BFS::traverse(g, 0);
    ASSERT_EQ(result.order.size(), static_cast<size_t>(4));
    EXPECT_EQ(result.order[0], 0);
    EXPECT_TRUE((result.order[1] == 1 && result.order[2] == 2 && result.order[3] == 3) ||
                (result.order[1] == 2 && result.order[2] == 3 && result.order[3] == 1) ||
                (result.order[1] == 3 && result.order[2] == 1 && result.order[3] == 2));
}

/**
 * @brief Test BFS traversal on a disconnected graph
 * @details Verifies that only the connected component reachable from the
 *          start node is visited
 */
TEST_F(BFSTest, Traverse_DisconnectedGraph)
{
    Graph g(4);
    g.addEdge(0, 1, 1);

    const auto result = BFS::traverse(g, 0);
    ASSERT_EQ(result.order.size(), static_cast<size_t>(2));
    EXPECT_EQ(result.order[0], 0);
    EXPECT_EQ(result.order[1], 1);
}

/**
 * @brief Test BFS traversal on a single-node graph
 * @details Edge case: verifies that a graph with one node returns only that node
 */
TEST_F(BFSTest, Traverse_SingleNode)
{
    Graph g(1);
    const auto result = BFS::traverse(g, 0);
    ASSERT_EQ(result.order.size(), static_cast<size_t>(1));
    EXPECT_EQ(result.order[0], 0);
}

/**
 * @brief Test BFS shortest path on a simple line graph
 * @details Verifies that the shortest path between node 0 and node 3
 *          contains all intermediate nodes in order
 */
TEST_F(BFSTest, ShortestPath_SimpleLine)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);

    const auto path = BFS::shortestPath(g, 0, 3);
    ASSERT_EQ(path.size(), static_cast<size_t>(4));
    EXPECT_EQ(path[0], 0);
    EXPECT_EQ(path[1], 1);
    EXPECT_EQ(path[2], 2);
    EXPECT_EQ(path[3], 3);
}

/**
 * @brief Test BFS shortest path when no path exists
 * @details Verifies that an empty path is returned for unreachable target nodes
 */
TEST_F(BFSTest, ShortestPath_NoPath)
{
    Graph g(4);
    g.addEdge(0, 1, 1);

    const auto path = BFS::shortestPath(g, 0, 3);
    EXPECT_TRUE(path.empty());
}

/**
 * @brief Test BFS shortest path from a node to itself
 * @details Edge case: verifies that the path from a node to itself
 *          contains only that single node
 */
TEST_F(BFSTest, ShortestPath_SameNode)
{
    Graph g(3);
    g.addEdge(0, 1, 1);

    const auto path = BFS::shortestPath(g, 0, 0);
    ASSERT_EQ(path.size(), static_cast<size_t>(1));
    EXPECT_EQ(path[0], 0);
}

/**
 * @brief Test BFS traversal with a custom visitor
 * @details Verifies that the visitor's onNodeVisit and onEdgeTraverse
 *          callbacks are invoked correctly during traversal
 */
TEST_F(BFSTest, Traverse_WithVisitor)
{
    Graph g(3);
    g.addEdge(0, 1, 5);
    g.addEdge(0, 2, 10);

    class TestVisitor : public IGraphVisitor
    {
    public:
        std::vector<int32_t> visited;
        std::vector<std::tuple<int32_t, int32_t, int32_t>> edges;

        void onNodeVisit(int32_t node) override
        {
            visited.push_back(node);
        }

        void onEdgeTraverse(int32_t from, int32_t to, int32_t weight) override
        {
            edges.emplace_back(from, to, weight);
        }
    };

    TestVisitor visitor;
    const auto result = BFS::traverse(g, 0, &visitor);

    ASSERT_EQ(visitor.visited.size(), static_cast<size_t>(3));
    EXPECT_EQ(visitor.visited[0], 0);
    EXPECT_EQ(visitor.edges.size(), static_cast<size_t>(2));
}
