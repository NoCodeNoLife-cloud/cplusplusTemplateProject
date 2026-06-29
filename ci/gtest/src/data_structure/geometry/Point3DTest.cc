/**
 * @file Point3DTest.cc
 * @brief Unit tests for the Point3D class
 * @details Tests cover construction, accessors, mutators, arithmetic operators,
 *          comparison operators, distance calculations, and stream output.
 */

#include <sstream>
#include <gtest/gtest.h>

#include "data_structure/geometry/Point3D.hpp"

using namespace cppforge::data_structure::geometry;

/**
 * @brief Test fixture for Point3D tests
 */
class Point3DTest : public testing::Test
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
 * @details Verifies default Point3D is at (0, 0, 0)
 */
TEST_F(Point3DTest, DefaultConstructor)
{
    const Point3D p;
    EXPECT_DOUBLE_EQ(p.getX(), 0.0);
    EXPECT_DOUBLE_EQ(p.getY(), 0.0);
    EXPECT_DOUBLE_EQ(p.getZ(), 0.0);
}

/**
 * @brief Test parameterized construction
 * @details Verifies constructor sets x, y, z correctly
 */
TEST_F(Point3DTest, ParameterizedConstructor)
{
    const Point3D p(1.5, -2.5, 3.0);
    EXPECT_DOUBLE_EQ(p.getX(), 1.5);
    EXPECT_DOUBLE_EQ(p.getY(), -2.5);
    EXPECT_DOUBLE_EQ(p.getZ(), 3.0);
}

/**
 * @brief Test setX updates the x coordinate
 * @details Verifies setX modifies the internal x_ value
 */
TEST_F(Point3DTest, SetX_UpdatesValue)
{
    Point3D p;
    p.setX(7.77);
    EXPECT_DOUBLE_EQ(p.getX(), 7.77);
}

/**
 * @brief Test setY updates the y coordinate
 * @details Verifies setY modifies the internal y_ value
 */
TEST_F(Point3DTest, SetY_UpdatesValue)
{
    Point3D p;
    p.setY(-3.14);
    EXPECT_DOUBLE_EQ(p.getY(), -3.14);
}

/**
 * @brief Test setZ updates the z coordinate
 * @details Verifies setZ modifies the internal z_ value
 */
TEST_F(Point3DTest, SetZ_UpdatesValue)
{
    Point3D p;
    p.setZ(2.71);
    EXPECT_DOUBLE_EQ(p.getZ(), 2.71);
}

/**
 * @brief Test compound addition operator
 * @details Verifies operator+= adds correctly and returns *this
 */
TEST_F(Point3DTest, PlusEqualOperator)
{
    Point3D a(1.0, 2.0, 3.0);
    const Point3D b(4.0, 5.0, 6.0);
    const Point3D& ref = a += b;
    EXPECT_DOUBLE_EQ(a.getX(), 5.0);
    EXPECT_DOUBLE_EQ(a.getY(), 7.0);
    EXPECT_DOUBLE_EQ(a.getZ(), 9.0);
    EXPECT_EQ(&ref, &a);
}

/**
 * @brief Test compound subtraction operator
 * @details Verifies operator-= subtracts correctly and returns *this
 */
TEST_F(Point3DTest, MinusEqualOperator)
{
    Point3D a(5.0, 7.0, 9.0);
    const Point3D b(2.0, 3.0, 4.0);
    const Point3D& ref = a -= b;
    EXPECT_DOUBLE_EQ(a.getX(), 3.0);
    EXPECT_DOUBLE_EQ(a.getY(), 4.0);
    EXPECT_DOUBLE_EQ(a.getZ(), 5.0);
    EXPECT_EQ(&ref, &a);
}

/**
 * @brief Test negation operator
 * @details Verifies unary operator- negates all coordinates
 */
TEST_F(Point3DTest, NegationOperator)
{
    const Point3D p(2.5, -3.5, 1.0);
    const Point3D neg = -p;
    EXPECT_DOUBLE_EQ(neg.getX(), -2.5);
    EXPECT_DOUBLE_EQ(neg.getY(), 3.5);
    EXPECT_DOUBLE_EQ(neg.getZ(), -1.0);
}

/**
 * @brief Test addition operator
 * @details Verifies operator+ returns correct sum
 */
TEST_F(Point3DTest, AdditionOperator)
{
    const Point3D a(1.0, 2.0, 3.0);
    const Point3D b(4.0, 5.0, 6.0);
    const Point3D c = a + b;
    EXPECT_DOUBLE_EQ(c.getX(), 5.0);
    EXPECT_DOUBLE_EQ(c.getY(), 7.0);
    EXPECT_DOUBLE_EQ(c.getZ(), 9.0);
}

/**
 * @brief Test subtraction operator
 * @details Verifies operator- returns correct difference
 */
TEST_F(Point3DTest, SubtractionOperator)
{
    const Point3D a(5.0, 7.0, 9.0);
    const Point3D b(2.0, 3.0, 4.0);
    const Point3D c = a - b;
    EXPECT_DOUBLE_EQ(c.getX(), 3.0);
    EXPECT_DOUBLE_EQ(c.getY(), 4.0);
    EXPECT_DOUBLE_EQ(c.getZ(), 5.0);
}

/**
 * @brief Test equality operator
 * @details Verifies operator== with exact values
 */
TEST_F(Point3DTest, EqualityOperator)
{
    const Point3D a(1.0, 2.0, 3.0);
    const Point3D b(1.0, 2.0, 3.0);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

/**
 * @brief Test equality operator with epsilon tolerance
 * @details Verifies operator== handles floating-point tolerance
 */
TEST_F(Point3DTest, EqualityOperator_EpsilonTolerance)
{
    const Point3D a(1.0, 2.0, 3.0);
    const Point3D b(1.0 + 1e-10, 2.0 + 1e-10, 3.0 + 1e-10);
    EXPECT_TRUE(a == b);
}

/**
 * @brief Test inequality operator
 * @details Verifies operator!= returns true for different points
 */
TEST_F(Point3DTest, InequalityOperator)
{
    const Point3D a(1.0, 2.0, 3.0);
    const Point3D b(1.0, 2.0, 4.0);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
}

/**
 * @brief Test distanceTo
 * @details Verifies Euclidean distance calculation in 3D
 */
TEST_F(Point3DTest, DistanceTo)
{
    const Point3D a(0.0, 0.0, 0.0);
    const Point3D b(1.0, 2.0, 2.0);
    EXPECT_DOUBLE_EQ(a.distanceTo(b), 3.0);
}

/**
 * @brief Test distanceTo with negative coordinates
 * @details Verifies distance with negative values
 */
TEST_F(Point3DTest, DistanceTo_NegativeCoordinates)
{
    const Point3D a(-1.0, -1.0, -1.0);
    const Point3D b(2.0, 3.0, 4.0);
    EXPECT_DOUBLE_EQ(a.distanceTo(b), std::sqrt(9.0 + 16.0 + 25.0));
}

/**
 * @brief Test distanceTo zero
 * @details Verifies distance from a point to itself is zero
 */
TEST_F(Point3DTest, DistanceTo_Zero)
{
    const Point3D a(3.0, 4.0, 5.0);
    EXPECT_DOUBLE_EQ(a.distanceTo(a), 0.0);
}

/**
 * @brief Test distanceSquaredTo
 * @details Verifies squared distance calculation in 3D
 */
TEST_F(Point3DTest, DistanceSquaredTo)
{
    const Point3D a(0.0, 0.0, 0.0);
    const Point3D b(1.0, 2.0, 2.0);
    EXPECT_DOUBLE_EQ(a.distanceSquaredTo(b), 9.0);
}

/**
 * @brief Test stream output
 * @details Verifies operator<< produces "(x, y, z)" format
 */
TEST_F(Point3DTest, StreamOutput)
{
    const Point3D p(1.5, -2.5, 3.0);
    std::ostringstream oss;
    oss << p;
    EXPECT_EQ(oss.str(), "(1.5, -2.5, 3.0)");
}

/**
 * @brief Test commutative addition
 * @details Verifies a + b == b + a
 */
TEST_F(Point3DTest, AdditionCommutative)
{
    const Point3D a(1.0, 2.0, 3.0);
    const Point3D b(4.0, 5.0, 6.0);
    EXPECT_EQ(a + b, b + a);
}
