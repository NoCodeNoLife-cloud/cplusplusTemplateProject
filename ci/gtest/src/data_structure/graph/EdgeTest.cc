/**
 * @file EdgeTest.cc
 * @brief Unit tests for the Edge class
 * @details Tests cover construction, accessors, mutators, and comparison operators.
 */

#include <gtest/gtest.h>

#include "data_structure/graph/Edge.hpp"

using namespace common::data_structure::graph;

/**
 * @brief Test fixture for Edge tests
 */
class EdgeTest : public testing::Test
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
 * @brief Test Edge construction with valid parameters
 * @details Verifies that the constructor correctly initializes to() and weight()
 */
TEST_F(EdgeTest, Construction_ValidParams)
{
    const Edge e(3, 10);
    EXPECT_EQ(e.to(), 3);
    EXPECT_EQ(e.weight(), 10);
}

/**
 * @brief Test Edge default member initialization
 * @details Verifies that default-constructed Edge has correct default values
 */
TEST_F(EdgeTest, DefaultValues)
{
    const Edge e(0, 0);
    EXPECT_EQ(e.to(), 0);
    EXPECT_EQ(e.weight(), 0);
}

/**
 * @brief Test setTo updates the target vertex
 * @details Verifies setTo modifies the internal to_ value
 */
TEST_F(EdgeTest, SetTo_UpdatesValue)
{
    Edge e(1, 5);
    e.setTo(7);
    EXPECT_EQ(e.to(), 7);
}

/**
 * @brief Test setWeight updates the weight
 * @details Verifies setWeight modifies the internal weight_ value
 */
TEST_F(EdgeTest, SetWeight_UpdatesValue)
{
    Edge e(1, 5);
    e.setWeight(99);
    EXPECT_EQ(e.weight(), 99);
}

/**
 * @brief Test equality operator for equal edges
 * @details Verifies operator== returns true when both to and weight match
 */
TEST_F(EdgeTest, EqualityOperator_EqualEdges)
{
    const Edge a(2, 8);
    const Edge b(2, 8);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

/**
 * @brief Test equality operator for edges with different to
 * @details Verifies operator== returns false when to differs
 */
TEST_F(EdgeTest, EqualityOperator_DifferentTo)
{
    const Edge a(2, 8);
    const Edge b(5, 8);
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

/**
 * @brief Test equality operator for edges with different weight
 * @details Verifies operator== returns false when weight differs
 */
TEST_F(EdgeTest, EqualityOperator_DifferentWeight)
{
    const Edge a(2, 8);
    const Edge b(2, 3);
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

/**
 * @brief Test Edge with large values
 * @details Verifies Edge handles large int32_t values correctly
 */
TEST_F(EdgeTest, LargeValues)
{
    const Edge e(INT32_MAX, INT32_MIN);
    EXPECT_EQ(e.to(), INT32_MAX);
    EXPECT_EQ(e.weight(), INT32_MIN);
}

/**
 * @brief Test negative target vertex
 * @details Verifies Edge accepts negative to values
 */
TEST_F(EdgeTest, NegativeTo)
{
    const Edge e(-1, 5);
    EXPECT_EQ(e.to(), -1);
    EXPECT_EQ(e.weight(), 5);
}

/**
 * @brief Test negative weight
 * @details Verifies Edge accepts negative weight values
 */
TEST_F(EdgeTest, NegativeWeight)
{
    const Edge e(0, -10);
    EXPECT_EQ(e.to(), 0);
    EXPECT_EQ(e.weight(), -10);
}

/**
 * @brief Test chained modifications
 * @details Verifies multiple set calls work correctly
 */
TEST_F(EdgeTest, ChainedModifications)
{
    Edge e(1, 1);
    e.setTo(2);
    e.setWeight(2);
    e.setTo(3);
    e.setWeight(3);
    EXPECT_EQ(e.to(), 3);
    EXPECT_EQ(e.weight(), 3);
}
