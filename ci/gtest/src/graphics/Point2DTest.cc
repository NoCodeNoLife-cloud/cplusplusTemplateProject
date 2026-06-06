/**
 * @file Point2DTest.cc
 * @brief Unit tests for the Point2D class
 * @details Tests cover construction, accessors, mutators, arithmetic operators,
 *          comparison operators, distance calculations, and stream output.
 */

#include <sstream>
#include <gtest/gtest.h>

#include "graphics/Point2D.hpp"

using namespace common::graphics;

/**
 * @brief Test fixture for Point2D tests
 */
class Point2DTest : public testing::Test
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
 * @brief Test default construction
 * @details Verifies default Point2D is at (0, 0)
 */
TEST_F(Point2DTest, DefaultConstructor)
{
    const Point2D p;
    EXPECT_DOUBLE_EQ(p.getX(), 0.0);
    EXPECT_DOUBLE_EQ(p.getY(), 0.0);
}

/**
 * @brief Test parameterized construction
 * @details Verifies constructor sets x and y correctly
 */
TEST_F(Point2DTest, ParameterizedConstructor)
{
    const Point2D p(3.5, -2.1);
    EXPECT_DOUBLE_EQ(p.getX(), 3.5);
    EXPECT_DOUBLE_EQ(p.getY(), -2.1);
}

/**
 * @brief Test setX updates the x coordinate
 * @details Verifies setX modifies the internal x_ value
 */
TEST_F(Point2DTest, SetX_UpdatesValue)
{
    Point2D p;
    p.setX(7.77);
    EXPECT_DOUBLE_EQ(p.getX(), 7.77);
}

/**
 * @brief Test setY updates the y coordinate
 * @details Verifies setY modifies the internal y_ value
 */
TEST_F(Point2DTest, SetY_UpdatesValue)
{
    Point2D p;
    p.setY(-3.14);
    EXPECT_DOUBLE_EQ(p.getY(), -3.14);
}

/**
 * @brief Test compound addition operator
 * @details Verifies operator+= adds correctly and returns *this
 */
TEST_F(Point2DTest, PlusEqualOperator)
{
    Point2D a(1.0, 2.0);
    const Point2D b(3.0, 4.0);
    const Point2D& ref = a += b;
    EXPECT_DOUBLE_EQ(a.getX(), 4.0);
    EXPECT_DOUBLE_EQ(a.getY(), 6.0);
    EXPECT_EQ(&ref, &a);
}

/**
 * @brief Test compound subtraction operator
 * @details Verifies operator-= subtracts correctly and returns *this
 */
TEST_F(Point2DTest, MinusEqualOperator)
{
    Point2D a(5.0, 7.0);
    const Point2D b(2.0, 3.0);
    const Point2D& ref = a -= b;
    EXPECT_DOUBLE_EQ(a.getX(), 3.0);
    EXPECT_DOUBLE_EQ(a.getY(), 4.0);
    EXPECT_EQ(&ref, &a);
}

/**
 * @brief Test negation operator
 * @details Verifies unary operator- negates both coordinates
 */
TEST_F(Point2DTest, NegationOperator)
{
    const Point2D p(2.5, -3.5);
    const Point2D neg = -p;
    EXPECT_DOUBLE_EQ(neg.getX(), -2.5);
    EXPECT_DOUBLE_EQ(neg.getY(), 3.5);
}

/**
 * @brief Test addition operator
 * @details Verifies operator+ returns correct sum
 */
TEST_F(Point2DTest, AdditionOperator)
{
    const Point2D a(1.0, 2.0);
    const Point2D b(3.0, 4.0);
    const Point2D c = a + b;
    EXPECT_DOUBLE_EQ(c.getX(), 4.0);
    EXPECT_DOUBLE_EQ(c.getY(), 6.0);
}

/**
 * @brief Test subtraction operator
 * @details Verifies operator- returns correct difference
 */
TEST_F(Point2DTest, SubtractionOperator)
{
    const Point2D a(5.0, 7.0);
    const Point2D b(2.0, 3.0);
    const Point2D c = a - b;
    EXPECT_DOUBLE_EQ(c.getX(), 3.0);
    EXPECT_DOUBLE_EQ(c.getY(), 4.0);
}

/**
 * @brief Test equality operator
 * @details Verifies operator== with exact and epsilon-close values
 */
TEST_F(Point2DTest, EqualityOperator)
{
    const Point2D a(1.0, 2.0);
    const Point2D b(1.0, 2.0);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

/**
 * @brief Test equality operator with epsilon tolerance
 * @details Verifies operator== handles floating-point tolerance
 */
TEST_F(Point2DTest, EqualityOperator_EpsilonTolerance)
{
    const Point2D a(1.0, 2.0);
    const Point2D b(1.0 + 1e-10, 2.0 + 1e-10);
    EXPECT_TRUE(a == b);
}

/**
 * @brief Test inequality operator
 * @details Verifies operator!= returns true for different points
 */
TEST_F(Point2DTest, InequalityOperator)
{
    const Point2D a(1.0, 2.0);
    const Point2D b(1.0, 3.0);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
}

/**
 * @brief Test distanceTo
 * @details Verifies Euclidean distance calculation
 */
TEST_F(Point2DTest, DistanceTo)
{
    const Point2D a(0.0, 0.0);
    const Point2D b(3.0, 4.0);
    EXPECT_DOUBLE_EQ(a.distanceTo(b), 5.0);
}

/**
 * @brief Test distanceTo with negative coordinates
 * @details Verifies distance with negative values
 */
TEST_F(Point2DTest, DistanceTo_NegativeCoordinates)
{
    const Point2D a(-1.0, -1.0);
    const Point2D b(2.0, 3.0);
    EXPECT_DOUBLE_EQ(a.distanceTo(b), 5.0);
}

/**
 * @brief Test distanceTo zero
 * @details Verifies distance from a point to itself is zero
 */
TEST_F(Point2DTest, DistanceTo_Zero)
{
    const Point2D a(3.0, 4.0);
    EXPECT_DOUBLE_EQ(a.distanceTo(a), 0.0);
}

/**
 * @brief Test distanceSquaredTo
 * @details Verifies squared distance calculation
 */
TEST_F(Point2DTest, DistanceSquaredTo)
{
    const Point2D a(0.0, 0.0);
    const Point2D b(3.0, 4.0);
    EXPECT_DOUBLE_EQ(a.distanceSquaredTo(b), 25.0);
}

/**
 * @brief Test stream output
 * @details Verifies operator<< produces "(x, y)" format
 */
TEST_F(Point2DTest, StreamOutput)
{
    const Point2D p(1.5, -2.5);
    std::ostringstream oss;
    oss << p;
    EXPECT_EQ(oss.str(), "(1.5, -2.5)");
}

/**
 * @brief Test commutative addition
 * @details Verifies a + b == b + a
 */
TEST_F(Point2DTest, AdditionCommutative)
{
    const Point2D a(1.0, 2.0);
    const Point2D b(3.0, 4.0);
    EXPECT_EQ(a + b, b + a);
}
