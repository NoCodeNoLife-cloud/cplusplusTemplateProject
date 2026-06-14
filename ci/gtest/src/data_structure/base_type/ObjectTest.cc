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

/**
 * @brief Test getClass returns correct type_info
 * @details Verifies that Object::getClass returns the type_info for Object
 */
TEST_F(ObjectTest, GetClass_ReturnsCorrectType)
{
    const Object obj;
    EXPECT_EQ(obj.getClass(), typeid(Object));
}

/**
 * @brief Test equals returns true for the same object reference
 * @details Verifies that Object::equals returns true when comparing an
 *          object to itself
 */
TEST_F(ObjectTest, Equals_SameObject_ReturnsTrue)
{
    const Object obj;
    EXPECT_TRUE(obj.equals(obj));
}

/**
 * @brief Test equals returns false for different objects
 * @details Verifies that Object::equals returns false when comparing two
 *          distinct Object instances
 */
TEST_F(ObjectTest, Equals_DifferentObject_ReturnsFalse)
{
    const Object a;
    const Object b;
    EXPECT_FALSE(a.equals(b));
}

/**
 * @brief Test hashCode returns consistent values
 * @details Verifies that Object::hashCode returns the same hash value
 *          for the same object on multiple invocations
 */
TEST_F(ObjectTest, HashCode_Consistent)
{
    const Object obj;
    const size_t h1 = obj.hashCode();
    const size_t h2 = obj.hashCode();
    EXPECT_EQ(h1, h2);
}

/**
 * @brief Test hashCode differs for different objects
 * @details Verifies that distinct Object instances produce different
 *          hash codes
 */
TEST_F(ObjectTest, HashCode_DifferentObjects_Different)
{
    const Object a;
    const Object b;
    EXPECT_NE(a.hashCode(), b.hashCode());
}

/**
 * @brief Test toString contains type information
 * @details Verifies that Object::toString returns a non-empty string
 *          that includes the class name "Object"
 */
TEST_F(ObjectTest, ToString_ContainsTypeInfo)
{
    const Object obj;
    const std::string str = obj.toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("Object"), std::string::npos);
}

/**
 * @brief Test clone on base Object throws logic_error
 * @details Verifies that cloning a base Object instance raises
 *          std::logic_error since it is not copyable
 */
TEST_F(ObjectTest, Clone_BaseObject_Throws)
{
    const Object obj;
    EXPECT_THROW(obj.clone(), std::logic_error);
}

/**
 * @brief Test instanceOf returns true for same type
 * @details Verifies that Object::instanceOf<Object>() returns true when
 *          checking against its own type
 */
TEST_F(ObjectTest, InstanceOf_SameType_ReturnsTrue)
{
    const Object obj;
    EXPECT_TRUE(obj.instanceOf<Object>());
}

/**
 * @brief Test is returns true for the same reference
 * @details Verifies that Object::is returns true when comparing an object
 *          to itself by reference
 */
TEST_F(ObjectTest, Is_SameReference_ReturnsTrue)
{
    const Object obj;
    EXPECT_TRUE(obj.is(obj));
}

/**
 * @brief Test is returns false for different references
 * @details Verifies that Object::is returns false when comparing two
 *          distinct Object instances by reference
 */
TEST_F(ObjectTest, Is_DifferentReference_ReturnsFalse)
{
    const Object a;
    const Object b;
    EXPECT_FALSE(a.is(b));
}

/**
 * @brief Test getClassName returns a non-empty string
 * @details Verifies that Object::getClassName returns a valid class name
 */
TEST_F(ObjectTest, GetClassName_NonEmpty)
{
    const Object obj;
    const std::string name = obj.getClassName();
    EXPECT_FALSE(name.empty());
}
