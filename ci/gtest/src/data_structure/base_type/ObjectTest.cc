/**
 * @file ObjectTest.cc
 * @brief Unit tests for the Object base class
 * @details Tests cover type information, equality, hashing, string representation,
 *          cloning, and instance type checking.
 */

#include <gtest/gtest.h>

#include "data_structure/base_type/Object.hpp"

using namespace common::data_structure::base_type;

/**
 * @brief Test fixture for Object tests
 */
class ObjectTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(ObjectTest, GetClass_ReturnsCorrectType)
{
    const Object obj;
    EXPECT_EQ(obj.getClass(), typeid(Object));
}

TEST_F(ObjectTest, Equals_SameObject_ReturnsTrue)
{
    const Object obj;
    EXPECT_TRUE(obj.equals(obj));
}

TEST_F(ObjectTest, Equals_DifferentObject_ReturnsFalse)
{
    const Object a;
    const Object b;
    EXPECT_FALSE(a.equals(b));
}

TEST_F(ObjectTest, HashCode_Consistent)
{
    const Object obj;
    const size_t h1 = obj.hashCode();
    const size_t h2 = obj.hashCode();
    EXPECT_EQ(h1, h2);
}

TEST_F(ObjectTest, HashCode_DifferentObjects_Different)
{
    const Object a;
    const Object b;
    EXPECT_NE(a.hashCode(), b.hashCode());
}

TEST_F(ObjectTest, ToString_ContainsTypeInfo)
{
    const Object obj;
    const std::string str = obj.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("Object"), std::string::npos);
}

TEST_F(ObjectTest, Clone_BaseObject_Throws)
{
    const Object obj;
    EXPECT_THROW(obj.clone(), std::logic_error);
}

TEST_F(ObjectTest, InstanceOf_SameType_ReturnsTrue)
{
    const Object obj;
    EXPECT_TRUE(obj.instanceOf<Object>());
}

TEST_F(ObjectTest, Is_SameReference_ReturnsTrue)
{
    const Object obj;
    EXPECT_TRUE(obj.is(obj));
}

TEST_F(ObjectTest, Is_DifferentReference_ReturnsFalse)
{
    const Object a;
    const Object b;
    EXPECT_FALSE(a.is(b));
}

TEST_F(ObjectTest, GetClassName_NonEmpty)
{
    const Object obj;
    const std::string name = obj.getClassName();
    EXPECT_FALSE(name.empty());
}
