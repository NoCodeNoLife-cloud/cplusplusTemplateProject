/**
 * @file GraphTest.cc
 * @brief Unit tests for the Graph class
 * @details Tests cover construction, edge addition, adjacency list retrieval,
 *          node count, empty state, and exception handling.
 */

#include <gtest/gtest.h>

#include "graphics/Graph.hpp"

using namespace common::graphics;

/**
 * @brief Test fixture for Graph tests
 */
class GraphTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Test Graph construction with valid node count
 * @details Verifies a graph with positive node count is created correctly
 */
TEST_F(GraphTest, Construction_ValidNodeCount)
{
    const Graph g(5);
    EXPECT_EQ(g.getNodeCount(), 5);
    EXPECT_FALSE(g.isEmpty());
}

/**
 * @brief Test Graph construction with zero nodes
 * @details Verifies a graph with zero nodes is empty
 */
TEST_F(GraphTest, Construction_ZeroNodes)
{
    const Graph g(0);
    EXPECT_EQ(g.getNodeCount(), 0);
    EXPECT_TRUE(g.isEmpty());
}

/**
 * @brief Test Graph construction with negative node count
 * @details Verifies negative node count throws std::invalid_argument
 */
TEST_F(GraphTest, Construction_NegativeNodes_ThrowsException)
{
    EXPECT_THROW(Graph(-1), std::invalid_argument);
}

/**
 * @brief Test addEdge with valid indices
 * @details Verifies adding an edge between valid nodes succeeds
 */
TEST_F(GraphTest, AddEdge_ValidIndices)
{
    Graph g(3);
    EXPECT_NO_THROW(g.addEdge(0, 1, 10));
}

/**
 * @brief Test getAdjList after adding edges
 * @details Verifies adjacency list contains the correct edges
 */
TEST_F(GraphTest, GetAdjList_ContainsAddedEdges)
{
    Graph g(3);
    g.addEdge(0, 1, 10);
    g.addEdge(0, 2, 20);

    const auto& list = g.getAdjList(0);
    ASSERT_EQ(list.size(), static_cast<size_t>(2));
    EXPECT_EQ(list[0].to(), 1);
    EXPECT_EQ(list[0].weight(), 10);
    EXPECT_EQ(list[1].to(), 2);
    EXPECT_EQ(list[1].weight(), 20);
}

/**
 * @brief Test getAdjList for node with no edges
 * @details Verifies adjacency list is empty for isolated nodes
 */
TEST_F(GraphTest, GetAdjList_IsolatedNode)
{
    const Graph g(3);
    const auto& list = g.getAdjList(1);
    EXPECT_TRUE(list.empty());
}

/**
 * @brief Test addEdge with negative from index
 * @details Verifies negative from throws std::out_of_range
 */
TEST_F(GraphTest, AddEdge_NegativeFrom_ThrowsException)
{
    Graph g(3);
    EXPECT_THROW(g.addEdge(-1, 0, 5), std::out_of_range);
}

/**
 * @brief Test addEdge with out-of-range from index
 * @details Verifies from >= nodeCount throws std::out_of_range
 */
TEST_F(GraphTest, AddEdge_FromOutOfRange_ThrowsException)
{
    Graph g(3);
    EXPECT_THROW(g.addEdge(3, 0, 5), std::out_of_range);
}

/**
 * @brief Test addEdge with negative to index
 * @details Verifies negative to throws std::out_of_range
 */
TEST_F(GraphTest, AddEdge_NegativeTo_ThrowsException)
{
    Graph g(3);
    EXPECT_THROW(g.addEdge(0, -1, 5), std::out_of_range);
}

/**
 * @brief Test addEdge with out-of-range to index
 * @details Verifies to >= nodeCount throws std::out_of_range
 */
TEST_F(GraphTest, AddEdge_ToOutOfRange_ThrowsException)
{
    Graph g(3);
    EXPECT_THROW(g.addEdge(0, 3, 5), std::out_of_range);
}

/**
 * @brief Test getAdjList with negative node index
 * @details Verifies negative node throws std::out_of_range
 */
TEST_F(GraphTest, GetAdjList_NegativeNode_ThrowsException)
{
    const Graph g(3);
    EXPECT_THROW(g.getAdjList(-1), std::out_of_range);
}

/**
 * @brief Test getAdjList with out-of-range node index
 * @details Verifies node >= nodeCount throws std::out_of_range
 */
TEST_F(GraphTest, GetAdjList_NodeOutOfRange_ThrowsException)
{
    const Graph g(3);
    EXPECT_THROW(g.getAdjList(3), std::out_of_range);
}

/**
 * @brief Test multiple edges from same source
 * @details Verifies a node can have multiple outgoing edges
 */
TEST_F(GraphTest, MultipleEdges_SameSource)
{
    Graph g(2);
    g.addEdge(0, 0, 1);
    g.addEdge(0, 1, 2);
    g.addEdge(0, 1, 3);

    const auto& list = g.getAdjList(0);
    ASSERT_EQ(list.size(), static_cast<size_t>(3));
    EXPECT_EQ(list[0].weight(), 1);
    EXPECT_EQ(list[1].weight(), 2);
    EXPECT_EQ(list[2].weight(), 3);
}

/**
 * @brief Test graph with large node count
 * @details Verifies graph handles larger node counts
 */
TEST_F(GraphTest, LargeNodeCount)
{
    constexpr int32_t kLargeCount = 10000;
    Graph g(kLargeCount);
    EXPECT_EQ(g.getNodeCount(), kLargeCount);
    EXPECT_NO_THROW(g.addEdge(0, kLargeCount - 1, 999));
    const auto& list = g.getAdjList(0);
    ASSERT_EQ(list.size(), static_cast<size_t>(1));
    EXPECT_EQ(list[0].to(), kLargeCount - 1);
}
