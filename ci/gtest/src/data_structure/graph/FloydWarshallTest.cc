/**
 * @file FloydWarshallTest.cc
 * @brief Unit tests for the Floyd-Warshall all-pairs shortest-path algorithm
 * @details Tests cover shortest-path computation on simple graphs, disconnected
 *          components, negative cycle detection, single-node edge cases,
 *          intermediate path optimization, and path reconstruction.
 */

#include <gtest/gtest.h>

#include <cppforge/data_structure/graph/algorithm/FloydWarshall.hpp>

using namespace cppforge::data_structure::graph;
using namespace cppforge::data_structure::graph::algorithm;

/**
 * @brief Test fixture for FloydWarshall tests
 */
class FloydWarshallTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test Floyd-Warshall on a simple line graph
 * @details Verifies all-pairs shortest distances in a 4-node line graph:
 *          distance 0â†? = 6, 2â†? = 3, and no negative cycle
 */
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

/**
 * @brief Test Floyd-Warshall on a disconnected graph
 * @details Verifies that unreachable node pairs have distance INT32_MAX / 2
 *          (the sentinel value used in the algorithm)
 */
TEST_F(FloydWarshallTest, Compute_Disconnected)
{
    Graph g(4);
    g.addEdge(0, 1, 1);

    const auto result = FloydWarshall::compute(g);
    EXPECT_EQ(result.distances[0][2], INT32_MAX / 2);
    EXPECT_EQ(result.distances[2][3], INT32_MAX / 2);
}

/**
 * @brief Test Floyd-Warshall negative cycle detection
 * @details Creates a 2-node cycle with negative total weight (-2)
 *          and verifies has_negative_cycle is set
 */
TEST_F(FloydWarshallTest, Compute_NegativeCycle)
{
    Graph g(2);
    g.addEdge(0, 1, -1);
    g.addEdge(1, 0, -1);

    const auto result = FloydWarshall::compute(g);
    EXPECT_TRUE(result.has_negative_cycle);
}

/**
 * @brief Test Floyd-Warshall on a single-node graph
 * @details Edge case: distance from a node to itself is 0 and no negative cycle
 */
TEST_F(FloydWarshallTest, Compute_SingleNode)
{
    Graph g(1);
    const auto result = FloydWarshall::compute(g);
    EXPECT_FALSE(result.has_negative_cycle);
    EXPECT_EQ(result.distances[0][0], 0);
}

/**
 * @brief Test Floyd-Warshall with intermediate path optimization
 * @details Verifies that the algorithm finds shorter paths via intermediate nodes:
 *          0â†? = 7 (via 1) instead of direct 10
 */
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

/**
 * @brief Test path reconstruction from Floyd-Warshall result
 * @details Verifies that reconstructPath returns the correct node sequence
 *          from source to target
 */
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

/**
 * @brief Test path reconstruction when no path exists
 * @details Verifies that an empty path is returned for unreachable node pairs
 */
TEST_F(FloydWarshallTest, ReconstructPath_NoPath)
{
    Graph g(4);
    g.addEdge(0, 1, 1);

    const auto result = FloydWarshall::compute(g);
    const auto path = result.reconstructPath(0, 3);
    EXPECT_TRUE(path.empty());
}
