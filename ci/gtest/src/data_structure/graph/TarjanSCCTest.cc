#include <gtest/gtest.h>

#include "data_structure/graph/algorithm/TarjanSCC.hpp"

using namespace common::data_structure::graph;
using namespace common::data_structure::graph::algorithm;

class TarjanSCCTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TarjanSCCTest, Compute_SingleNodes)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(2, 3, 1);

    const auto result = TarjanSCC::compute(g);
    EXPECT_EQ(result.components.size(), static_cast<size_t>(4));
}

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

TEST_F(TarjanSCCTest, Compute_LinearGraph)
{
    Graph g(4);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(2, 3, 1);

    const auto result = TarjanSCC::compute(g);
    EXPECT_EQ(result.components.size(), static_cast<size_t>(4));
}

TEST_F(TarjanSCCTest, Compute_EmptyGraph)
{
    const Graph g(0);
    const auto result = TarjanSCC::compute(g);
    EXPECT_TRUE(result.components.empty());
}

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
