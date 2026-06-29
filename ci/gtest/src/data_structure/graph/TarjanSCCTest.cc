/**
 * @file TarjanSCCTest.cc
 * @brief Unit tests for Tarjan's Strongly Connected Components algorithm
 * @details Tests cover SCC detection on single nodes, simple cycles,
 *          complex graphs with mixed SCCs, linear (acyclic) graphs,
 *          empty graphs, and component ID assignment.
 */

#include <gtest/gtest.h>

#include <cppforge/data_structure/graph/algorithm/TarjanSCC.hpp>

using namespace cppforge::data_structure::graph;
using namespace cppforge::data_structure::graph::algorithm;

/**
 * @brief Test fixture for TarjanSCC tests
 */
class TarjanSCCTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test SCC detection on a graph where every node is its own SCC
 * @details In a graph with edges 0â†? and 2â†? (no cycles), each node
 *          forms its own SCC, yielding 4 components
 */
TEST_F(TarjanSCCTest, Compute_SingleNodes)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(2, 3, 1);

    const auto result = TarjanSCC::compute(g);
    EXPECT_EQ(result.components.size(), static_cast<size_t>(4));
}

/**
 * @brief Test SCC detection on a simple 3-node directed cycle
 * @details All 3 nodes in a directed cycle belong to a single SCC
 */
TEST_F(TarjanSCCTest, Compute_SimpleCycle)
{
    Graph g(3);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 0, 1);

    const auto result = TarjanSCC::compute(g);
    EXPECT_EQ(result.components.size(), static_cast<size_t>(1));
    EXPECT_EQ(result.components[0].size(), static_cast<size_t>(3));
}

/**
 * @brief Test SCC detection on a graph with mixed components
 * @details Contains a 3-node cycle (0â†?â†?â†?) plus a chain (1â†?â†?)
 *          yielding at least 2 SCCs
 */
TEST_F(TarjanSCCTest, Compute_ComplexGraph)
{
    Graph g(5);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 0, 1);
    g.addEdge(1, 3, 1);
    g.addEdge(3, 4, 1);

    const auto result = TarjanSCC::compute(g);
    EXPECT_GE(result.components.size(), static_cast<size_t>(2));

    bool found_three_cycle = false;
    for (const auto& comp : result.components)
    {
        if (comp.size() == 3)
        {
            found_three_cycle = true;
            break;
        }
    }
    EXPECT_TRUE(found_three_cycle);
}

/**
 * @brief Test SCC detection on a linear (acyclic) directed graph
 * @details In a line 0â†?â†?â†? each node forms its own SCC, yielding 4 components
 */
TEST_F(TarjanSCCTest, Compute_LinearGraph)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);

    const auto result = TarjanSCC::compute(g);
    EXPECT_EQ(result.components.size(), static_cast<size_t>(4));
}

/**
 * @brief Test SCC detection on an empty graph
 * @details Edge case: empty graph has no SCCs
 */
TEST_F(TarjanSCCTest, Compute_EmptyGraph)
{
    const Graph g(0);
    const auto result = TarjanSCC::compute(g);
    EXPECT_TRUE(result.components.empty());
}

/**
 * @brief Test that nodes in the same SCC share the same component ID
 * @details Verifies that all nodes in a 3-node cycle are assigned
 *          the same component_id value
 */
TEST_F(TarjanSCCTest, Compute_ComponentId)
{
    Graph g(3);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 0, 1);

    const auto result = TarjanSCC::compute(g);
    EXPECT_EQ(result.component_id[0], 0);
    EXPECT_EQ(result.component_id[1], 0);
    EXPECT_EQ(result.component_id[2], 0);
}
