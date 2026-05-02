/**
 * @file ObjectTest.cc
 * @brief Unit tests for the Object class
 * @details Tests cover type information, equality, hashing, string representation,
 * and object utilities functionality.
 */

#include <gtest/gtest.h>
#include "base_type/Object.hpp"
#include <memory>
#include <string>
#include <typeinfo>

using namespace common::base_type;

/**
 * @brief Test class that inherits from Object for testing purposes
 * @details Provides a concrete implementation to test Object's virtual methods
 */
class TestDerivedObject : public Object {
public:
    explicit TestDerivedObject(int value = 0) : m_value(value) {}

    /**
     * @brief Override equals to compare by value
     * @param[in] other The object to compare with
     * @return true if values are equal, false otherwise
     */
    [[nodiscard]] auto equals(const Object &other) const -> bool override {
        const auto *derived = dynamic_cast<const TestDerivedObject *>(&other);
        return derived != nullptr && m_value == derived->m_value;
    }

    /**
     * @brief Override hashCode to hash based on value
     * @return size_t Hash code based on m_value
     */
    [[nodiscard]] auto hashCode() const noexcept -> size_t override {
        return std::hash<int>{}(m_value);
    }

    /**
     * @brief Override toString to provide meaningful representation
     * @return std::string String representation including value
     */
    [[nodiscard]] auto toString() const -> std::string override {
        return "TestDerivedObject(" + std::to_string(m_value) + ")";
    }

    /**
     * @brief Override clone to create a copy
     * @return std::unique_ptr<Object> A new instance with same value
     */
    [[nodiscard]] auto clone() const -> std::unique_ptr<Object> override {
        return std::make_unique<TestDerivedObject>(m_value);
    }

    [[nodiscard]] auto getValue() const -> int { return m_value; }

private:
    int m_value;
};

/**
 * @brief Test getClass returns correct type information
 * @details Verifies that getClass returns typeid of the actual object
 */
TEST(ObjectTest, GetClass_ReturnsCorrectTypeInfo) {
    TestDerivedObject obj{42};
    
    const auto &type_info = obj.getClass();
    
    EXPECT_EQ(type_info, typeid(TestDerivedObject));
}

/**
 * @brief Test equals with same reference (default behavior)
 * @details Verifies that an object is equal to itself by default
 */
TEST(ObjectTest, Equals_SameReference_ReturnsTrue) {
    TestDerivedObject obj{10};
    
    EXPECT_TRUE(obj.equals(obj));
}

/**
 * @brief Test equals with different objects having same value
 * @details Verifies custom equals implementation compares values correctly
 */
TEST(ObjectTest, Equals_DifferentObjectsSameValue_ReturnsTrue) {
    TestDerivedObject obj1{42};
    TestDerivedObject obj2{42};
    
    EXPECT_TRUE(obj1.equals(obj2));
}

/**
 * @brief Test equals with different objects having different values
 * @details Verifies custom equals implementation detects value differences
 */
TEST(ObjectTest, Equals_DifferentObjectsDifferentValues_ReturnsFalse) {
    TestDerivedObject obj1{42};
    TestDerivedObject obj2{43};
    
    EXPECT_FALSE(obj1.equals(obj2));
}

/**
 * @brief Test equals with incompatible types
 * @details Verifies equals returns false when comparing with different types
 */
TEST(ObjectTest, Equals_IncompatibleTypes_ReturnsFalse) {
    TestDerivedObject obj1{42};
    TestDerivedObject obj2{42};
    const Object &base_ref = obj2;
    
    // Create another derived object to test type checking
    class AnotherDerived : public Object {
    public:
        explicit AnotherDerived(int val) : m_val(val) {}
        [[nodiscard]] auto equals(const Object &) const -> bool override { return false; }
    private:
        int m_val;
    };
    
    AnotherDerived another{42};
    
    EXPECT_FALSE(obj1.equals(another));
}

/**
 * @brief Test hashCode consistency
 * @details Verifies that hashCode returns consistent values for same object state
 */
TEST(ObjectTest, HashCode_ConsistentForSameState) {
    TestDerivedObject obj{100};
    
    const auto hash1 = obj.hashCode();
    const auto hash2 = obj.hashCode();
    
    EXPECT_EQ(hash1, hash2);
}

/**
 * @brief Test hashCode for objects with equal values
 * @details Verifies that equal objects have equal hash codes
 */
TEST(ObjectTest, HashCode_EqualObjectsHaveEqualHashCodes) {
    TestDerivedObject obj1{50};
    TestDerivedObject obj2{50};
    
    EXPECT_EQ(obj1.hashCode(), obj2.hashCode());
}

/**
 * @brief Test hashCode for objects with different values
 * @details Verifies that different values typically produce different hash codes
 */
TEST(ObjectTest, HashCode_DifferentValuesProduceDifferentHashCodes) {
    TestDerivedObject obj1{1};
    TestDerivedObject obj2{2};
    
    EXPECT_NE(obj1.hashCode(), obj2.hashCode());
}

/**
 * @brief Test toString provides meaningful representation
 * @details Verifies toString includes class name and value
 */
TEST(ObjectTest, ToString_ProvidesMeaningfulRepresentation) {
    TestDerivedObject obj{123};
    
    const auto str = obj.toString();
    
    EXPECT_NE(str.find("TestDerivedObject"), std::string::npos);
    EXPECT_NE(str.find("123"), std::string::npos);
}

/**
 * @brief Test toString for different objects produces different strings
 * @details Verifies toString reflects object state differences
 */
TEST(ObjectTest, ToString_DifferentObjectsProduceDifferentStrings) {
    TestDerivedObject obj1{10};
    TestDerivedObject obj2{20};
    
    const auto str1 = obj1.toString();
    const auto str2 = obj2.toString();
    
    EXPECT_NE(str1, str2);
}

/**
 * @brief Test clone creates independent copy
 * @details Verifies cloned object has same value but is a separate instance
 */
TEST(ObjectTest, Clone_CreatesIndependentCopy) {
    TestDerivedObject original{99};
    
    auto cloned = original.clone();
    
    ASSERT_NE(cloned, nullptr);
    EXPECT_TRUE(original.equals(*cloned));
    EXPECT_FALSE(original.is(*cloned));
}

/**
 * @brief Test clone preserves object state
 * @details Verifies cloned object maintains the same value as original
 */
TEST(ObjectTest, Clone_PreservesObjectState) {
    TestDerivedObject original{77};
    
    auto cloned = original.clone();
    const auto *cloned_derived = dynamic_cast<TestDerivedObject *>(cloned.get());
    
    ASSERT_NE(cloned_derived, nullptr);
    EXPECT_EQ(cloned_derived->getValue(), original.getValue());
}

/**
 * @brief Test base Object clone throws exception
 * @details Verifies that calling clone on base Object throws logic_error
 */
TEST(ObjectTest, Clone_BaseObjectThrowsException) {
    class SimpleObject : public Object {
    public:
        // Does not override clone, so should use base implementation
    };
    
    SimpleObject obj;
    
    EXPECT_THROW(obj.clone(), std::logic_error);
}

/**
 * @brief Test instanceOf with matching type
 * @details Verifies instanceOf returns true for correct type
 */
TEST(ObjectTest, InstanceOf_MatchingType_ReturnsTrue) {
    TestDerivedObject obj{42};
    
    EXPECT_TRUE(obj.instanceOf<TestDerivedObject>());
}

/**
 * @brief Test instanceOf with base type
 * @details Verifies instanceOf returns true for base class type
 */
TEST(ObjectTest, InstanceOf_BaseType_ReturnsTrue) {
    TestDerivedObject obj{42};
    
    EXPECT_TRUE(obj.instanceOf<Object>());
}

/**
 * @brief Test instanceOf with unrelated type
 * @details Verifies instanceOf returns false for incompatible types
 */
TEST(ObjectTest, InstanceOf_UnrelatedType_ReturnsFalse) {
    TestDerivedObject obj{42};
    
    class UnrelatedType : public Object {
    public:
        [[nodiscard]] auto equals(const Object &) const -> bool override { return false; }
    };
    
    EXPECT_FALSE(obj.instanceOf<UnrelatedType>());
}

/**
 * @brief Test isInstance with matching type
 * @details Verifies isInstance returns true for correct type_info
 */
TEST(ObjectTest, IsInstance_MatchingType_ReturnsTrue) {
    TestDerivedObject obj{42};
    
    EXPECT_TRUE(obj.isInstance(typeid(TestDerivedObject)));
}

/**
 * @brief Test isInstance with non-matching type
 * @details Verifies isInstance returns false for incorrect type_info
 */
TEST(ObjectTest, IsInstance_NonMatchingType_ReturnsFalse) {
    TestDerivedObject obj{42};
    
    EXPECT_FALSE(obj.isInstance(typeid(int)));
}

/**
 * @brief Test getClassName returns class name
 * @details Verifies getClassName returns the actual class name
 */
TEST(ObjectTest, GetClassName_ReturnsClassName) {
    TestDerivedObject obj{42};
    
    const auto class_name = obj.getClassName();
    
    EXPECT_NE(class_name.find("TestDerivedObject"), std::string::npos);
}

/**
 * @brief Test is with same object reference
 * @details Verifies is returns true when comparing object with itself
 */
TEST(ObjectTest, Is_SameReference_ReturnsTrue) {
    TestDerivedObject obj{42};
    
    EXPECT_TRUE(obj.is(obj));
}

/**
 * @brief Test is with different objects
 * @details Verifies is returns false for different object instances
 */
TEST(ObjectTest, Is_DifferentObjects_ReturnsFalse) {
    TestDerivedObject obj1{42};
    TestDerivedObject obj2{42};
    
    EXPECT_FALSE(obj1.is(obj2));
}

/**
 * @brief Test is with references to same object
 * @details Verifies is correctly identifies same object through different references
 */
TEST(ObjectTest, Is_DifferentReferencesSameObject_ReturnsTrue) {
    TestDerivedObject obj{42};
    const Object &ref1 = obj;
    const Object &ref2 = obj;
    
    EXPECT_TRUE(ref1.is(ref2));
}

/**
 * @brief Test polymorphic behavior through base pointer
 * @details Verifies virtual methods work correctly through base class pointers
 */
TEST(ObjectTest, PolymorphicBehavior_ThroughBasePointer) {
    auto obj = std::make_unique<TestDerivedObject>(55);
    Object *base_ptr = obj.get();
    
    EXPECT_EQ(base_ptr->getClass(), typeid(TestDerivedObject));
    EXPECT_EQ(base_ptr->hashCode(), std::hash<int>{}(55));
    EXPECT_NE(base_ptr->toString().find("55"), std::string::npos);
}

/**
 * @brief Test multiple clones are independent
 * @details Verifies that multiple clones don't share state
 */
TEST(ObjectTest, MultipleClones_AreIndependent) {
    TestDerivedObject original{100};
    
    auto clone1 = original.clone();
    auto clone2 = original.clone();
    
    EXPECT_TRUE(clone1->equals(*clone2));
    EXPECT_FALSE(clone1->is(*clone2));
}
