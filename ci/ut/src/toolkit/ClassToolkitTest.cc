/**
 * @file ClassToolkitTest.cc
 * @brief Unit tests for the ClassToolkit class
 * @details Tests cover type identification, reflection, and field extraction functionality.
 */

#include <gtest/gtest.h>
#include "toolkit/ClassToolkit.hpp"
#include <string>
#include <type_traits>

using namespace common::toolkit;

/**
 * @brief Simple test class for reflection tests
 */
struct TestPerson {
    std::string name;
    int age;
    double score;
};

/**
 * @brief ReflectTraits specialization for TestPerson
 * @details Defines metadata for reflection including field count and member pointers
 */
template<>
struct ReflectTraits<TestPerson> {
    static constexpr std::size_t field_count = 3;
    static constexpr auto fields = std::make_tuple(
        std::make_pair("name", &TestPerson::name),
        std::make_pair("age", &TestPerson::age),
        std::make_pair("score", &TestPerson::score)
    );
};

/**
 * @brief Test getTypeId with basic types
 * @details Verifies type name extraction for fundamental types
 */
TEST(ClassToolkitTest, GetTypeId_BasicTypes) {
    constexpr int int_val = 42;
    constexpr double double_val = 3.14;
    const std::string str_val = "test";

    auto int_type = ClassToolkit::getTypeId(int_val);
    auto double_type = ClassToolkit::getTypeId(double_val);
    const auto string_type = ClassToolkit::getTypeId(str_val);

    EXPECT_FALSE(int_type.empty());
    EXPECT_FALSE(double_type.empty());
    EXPECT_FALSE(string_type.empty());

    // Verify type names contain expected substrings
    EXPECT_NE(int_type.find("int"), std::string::npos);
    EXPECT_NE(double_type.find("double"), std::string::npos);
}

/**
 * @brief Test getTypeId with custom class
 * @details Verifies type name extraction includes class name
 */
TEST(ClassToolkitTest, GetTypeId_CustomClass) {
    const TestPerson person{"Alice", 25, 95.5};
    auto type_name = ClassToolkit::getTypeId(person);

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("TestPerson"), std::string::npos);
}

/**
 * @brief Test getTypeIdWithCvr with basic types
 * @details Verifies CVR (const/volatile/reference) qualifiers are handled correctly
 */
TEST(ClassToolkitTest, GetTypeIdWithCvr_BasicTypes) {
    constexpr int const_int = 100;
    // Note: getTypeIdWithCvr deduces T from parameter type
    // For const int&, T is deduced as int (not const int)
    auto type_with_cvr = ClassToolkit::getTypeIdWithCvr(const_int);

    EXPECT_FALSE(type_with_cvr.empty());
    // The type should be "int" since template deduction strips top-level const
    EXPECT_NE(type_with_cvr.find("int"), std::string::npos);
}

/**
 * @brief Test getTypeIdByClass without instance for int type
 * @details Verifies template-based type identification
 */
TEST(ClassToolkitTest, GetTypeIdByClass_IntType) {
    auto type_name = ClassToolkit::getTypeIdByClass<int>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("int"), std::string::npos);
}

/**
 * @brief Test getTypeIdByClass for double type
 * @details Verifies floating-point type identification
 */
TEST(ClassToolkitTest, GetTypeIdByClass_DoubleType) {
    auto type_name = ClassToolkit::getTypeIdByClass<double>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("double"), std::string::npos);
}

/**
 * @brief Test getTypeIdByClass for string type
 * @details Verifies standard library type identification
 */
TEST(ClassToolkitTest, GetTypeIdByClass_StringType) {
    auto type_name = ClassToolkit::getTypeIdByClass<std::string>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("string"), std::string::npos);
}

/**
 * @brief Test getTypeIdByClass for custom class
 * @details Verifies user-defined type identification
 */
TEST(ClassToolkitTest, GetTypeIdByClass_CustomClass) {
    auto type_name = ClassToolkit::getTypeIdByClass<TestPerson>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("TestPerson"), std::string::npos);
}

/**
 * @brief Test getTypeIdWithCvrByClass with const qualifier
 * @details Verifies const qualifier is preserved in type name
 */
TEST(ClassToolkitTest, GetTypeIdWithCvrByClass_ConstInt) {
    auto type_name = ClassToolkit::getTypeIdWithCvrByClass<const int>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("const"), std::string::npos);
    EXPECT_NE(type_name.find("int"), std::string::npos);
}

/**
 * @brief Test getTypeIdWithCvrByClass with volatile qualifier
 * @details Verifies volatile qualifier is preserved in type name
 */
TEST(ClassToolkitTest, GetTypeIdWithCvrByClass_VolatileDouble) {
    auto type_name = ClassToolkit::getTypeIdWithCvrByClass<volatile double>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("volatile"), std::string::npos);
    EXPECT_NE(type_name.find("double"), std::string::npos);
}

/**
 * @brief Test consistency between by-class and by-object methods
 * @details Verifies both approaches produce identical type names
 */
TEST(ClassToolkitTest, Consistency_ByClassVsByObject) {
    constexpr int value = 42;

    const auto by_class = ClassToolkit::getTypeIdByClass<int>();
    const auto by_object = ClassToolkit::getTypeId(value);

    EXPECT_EQ(by_class, by_object);
}

/**
 * @brief Test consistency of CVR methods
 * @details Verifies CVR handling is consistent across different APIs
 */
TEST(ClassToolkitTest, Consistency_CvrMethods) {
    constexpr int const_value = 100;

    // Both should deduce as "int" (template deduction strips top-level const)
    const auto by_class = ClassToolkit::getTypeIdWithCvrByClass<int>();
    const auto by_object = ClassToolkit::getTypeIdWithCvr(const_value);

    EXPECT_EQ(by_class, by_object);
}

/**
 * @brief Test getFields basic structure extraction
 * @details Verifies all fields are extracted with correct string representations
 */
TEST(ClassToolkitTest, GetFields_BasicStructure) {
    const TestPerson person{"Bob", 30, 88.5};
    auto fields = ClassToolkit::getFields(person);

    EXPECT_EQ(fields.size(), 3);
    EXPECT_EQ(fields["name"], "Bob");
    EXPECT_EQ(fields["age"], "30");
    EXPECT_EQ(fields["score"], "88.5");
}

/**
 * @brief Test getFields with empty string and zero values
 * @details Verifies edge case handling for default/empty values
 */
TEST(ClassToolkitTest, GetFields_EmptyString) {
    const TestPerson person{"", 0, 0.0};
    auto fields = ClassToolkit::getFields(person);

    EXPECT_EQ(fields.size(), 3);
    EXPECT_EQ(fields["name"], "");
    EXPECT_EQ(fields["age"], "0");
    EXPECT_EQ(fields["score"], "0");
}

/**
 * @brief Test getFields with negative values
 * @details Verifies correct string representation of negative numbers
 */
TEST(ClassToolkitTest, GetFields_NegativeValues) {
    const TestPerson person{"Charlie", -5, -10.5};
    auto fields = ClassToolkit::getFields(person);

    EXPECT_EQ(fields.size(), 3);
    EXPECT_EQ(fields["name"], "Charlie");
    EXPECT_EQ(fields["age"], "-5");
    EXPECT_EQ(fields["score"], "-10.5");
}

/**
 * @brief Test that constructor is deleted (compile-time check)
 * @details Verifies ClassToolkit cannot be instantiated as it's a utility class
 */
TEST(ClassToolkitTest, ConstructorDeleted) {
    // This test verifies at compile time that ClassToolkit cannot be instantiated
    // If the following line compiles, the test fails
    static_assert(std::is_constructible_v<ClassToolkit> == false,
                  "ClassToolkit should not be constructible");
}

// ==================== New Feature Tests ====================

/**
 * @brief Test isSameType with identical types
 * @details Verifies true is returned for same types
 */
TEST(ClassToolkitTest, IsSameType_IdenticalTypes) {
    EXPECT_TRUE((ClassToolkit::isSameType<int, int>()));
    EXPECT_TRUE((ClassToolkit::isSameType<std::string, std::string>()));
}

/**
 * @brief Test isSameType with different types
 * @details Verifies false is returned for different types
 */
TEST(ClassToolkitTest, IsSameType_DifferentTypes) {
    EXPECT_FALSE((ClassToolkit::isSameType<int, double>()));
    EXPECT_FALSE((ClassToolkit::isSameType<int, std::string>()));
}

/**
 * @brief Test isBaseOf with valid inheritance
 * @details Verifies true is returned when Base is actually a base of Derived
 */
TEST(ClassToolkitTest, IsBaseOf_ValidInheritance) {
    struct Base {};
    struct Derived : Base {};
    
    EXPECT_TRUE((ClassToolkit::isBaseOf<Base, Derived>()));
}

/**
 * @brief Test isBaseOf with no inheritance
 * @details Verifies false is returned when there's no inheritance relationship
 */
TEST(ClassToolkitTest, IsBaseOf_NoInheritance) {
    struct ClassA {};
    struct ClassB {};
    
    EXPECT_FALSE((ClassToolkit::isBaseOf<ClassA, ClassB>()));
}

/**
 * @brief Test isPolymorphic with polymorphic class
 * @details Verifies true is returned for classes with virtual functions
 */
TEST(ClassToolkitTest, IsPolymorphic_PolymorphicClass) {
    struct Polymorphic {
        virtual ~Polymorphic() = default;
    };
    
    EXPECT_TRUE((ClassToolkit::isPolymorphic<Polymorphic>()));
}

/**
 * @brief Test isPolymorphic with non-polymorphic class
 * @details Verifies false is returned for classes without virtual functions
 */
TEST(ClassToolkitTest, IsPolymorphic_NonPolymorphicClass) {
    struct NonPolymorphic {
        int x;
    };
    
    EXPECT_FALSE((ClassToolkit::isPolymorphic<NonPolymorphic>()));
}

/**
 * @brief Test isAbstract with abstract class
 * @details Verifies true is returned for abstract classes
 */
TEST(ClassToolkitTest, IsAbstract_AbstractClass) {
    struct Abstract {
        virtual void foo() = 0;
        virtual ~Abstract() = default;
    };
    
    EXPECT_TRUE((ClassToolkit::isAbstract<Abstract>()));
}

/**
 * @brief Test isAbstract with concrete class
 * @details Verifies false is returned for concrete classes
 */
TEST(ClassToolkitTest, IsAbstract_ConcreteClass) {
    struct Concrete {
        void foo() {}
    };
    
    EXPECT_FALSE((ClassToolkit::isAbstract<Concrete>()));
}

/**
 * @brief Test getTypeSize for basic types
 * @details Verifies correct size is returned
 */
TEST(ClassToolkitTest, GetTypeSize_BasicTypes) {
    EXPECT_EQ(ClassToolkit::getTypeSize<char>(), 1);
    EXPECT_EQ(ClassToolkit::getTypeSize<int>(), sizeof(int));
    EXPECT_EQ(ClassToolkit::getTypeSize<double>(), sizeof(double));
}

/**
 * @brief Test getTypeSize for custom struct
 * @details Verifies size calculation for user-defined types
 */
TEST(ClassToolkitTest, GetTypeSize_CustomStruct) {
    EXPECT_GT(ClassToolkit::getTypeSize<TestPerson>(), 0);
}

/**
 * @brief Test getTypeAlignment for basic types
 * @details Verifies correct alignment is returned
 */
TEST(ClassToolkitTest, GetTypeAlignment_BasicTypes) {
    EXPECT_EQ(ClassToolkit::getTypeAlignment<char>(), alignof(char));
    EXPECT_EQ(ClassToolkit::getTypeAlignment<int>(), alignof(int));
    EXPECT_EQ(ClassToolkit::getTypeAlignment<double>(), alignof(double));
}

/**
 * @brief Test isDefaultConstructible with constructible types
 * @details Verifies true is returned for default constructible types
 */
TEST(ClassToolkitTest, IsDefaultConstructible_ConstructibleTypes) {
    EXPECT_TRUE((ClassToolkit::isDefaultConstructible<int>()));
    EXPECT_TRUE((ClassToolkit::isDefaultConstructible<std::string>()));
    EXPECT_TRUE((ClassToolkit::isDefaultConstructible<TestPerson>()));
}

/**
 * @brief Test isCopyConstructible with copyable types
 * @details Verifies true is returned for copy constructible types
 */
TEST(ClassToolkitTest, IsCopyConstructible_CopyableTypes) {
    EXPECT_TRUE((ClassToolkit::isCopyConstructible<int>()));
    EXPECT_TRUE((ClassToolkit::isCopyConstructible<std::string>()));
}

/**
 * @brief Test isMoveConstructible with movable types
 * @details Verifies true is returned for move constructible types
 */
TEST(ClassToolkitTest, IsMoveConstructible_MovableTypes) {
    EXPECT_TRUE((ClassToolkit::isMoveConstructible<int>()));
    EXPECT_TRUE((ClassToolkit::isMoveConstructible<std::string>()));
}

/**
 * @brief Test isTriviallyCopyable with trivial types
 * @details Verifies true is returned for trivially copyable types
 */
TEST(ClassToolkitTest, IsTriviallyCopyable_TrivialTypes) {
    EXPECT_TRUE((ClassToolkit::isTriviallyCopyable<int>()));
    EXPECT_TRUE((ClassToolkit::isTriviallyCopyable<double>()));
}

/**
 * @brief Test isTriviallyDestructible with trivial types
 * @details Verifies true is returned for trivially destructible types
 */
TEST(ClassToolkitTest, IsTriviallyDestructible_TrivialTypes) {
    EXPECT_TRUE((ClassToolkit::isTriviallyDestructible<int>()));
    EXPECT_TRUE((ClassToolkit::isTriviallyDestructible<double>()));
}

/**
 * @brief Test getTypeHash returns consistent values
 * @details Verifies hash is consistent for same type
 */
TEST(ClassToolkitTest, GetTypeHash_ConsistentValues) {
    auto hash1 = ClassToolkit::getTypeHash<int>();
    auto hash2 = ClassToolkit::getTypeHash<int>();
    EXPECT_EQ(hash1, hash2);
}

/**
 * @brief Test getTypeHash returns different values for different types
 * @details Verifies hash differs for different types
 */
TEST(ClassToolkitTest, GetTypeHash_DifferentTypes) {
    auto hash_int = ClassToolkit::getTypeHash<int>();
    auto hash_double = ClassToolkit::getTypeHash<double>();
    EXPECT_NE(hash_int, hash_double);
}

/**
 * @brief Test getFieldNames returns all field names
 * @details Verifies all fields are listed correctly
 */
TEST(ClassToolkitTest, GetFieldNames_ReturnsAllFields) {
    auto names = ClassToolkit::getFieldNames<TestPerson>();
    
    ASSERT_EQ(names.size(), 3);
    EXPECT_EQ(names[0], "name");
    EXPECT_EQ(names[1], "age");
    EXPECT_EQ(names[2], "score");
}

/**
 * @brief Test getFieldCount returns correct count
 * @details Verifies field count matches ReflectTraits definition
 */
TEST(ClassToolkitTest, GetFieldCount_ReturnsCorrectCount) {
    EXPECT_EQ(ClassToolkit::getFieldCount<TestPerson>(), 3);
}

/**
 * @brief Test getFieldByName retrieves correct value
 * @details Verifies field value is correctly retrieved by name
 */
TEST(ClassToolkitTest, GetFieldByName_RetrievesCorrectValue) {
    const TestPerson person{"Alice", 25, 95.5};
    
    EXPECT_EQ(ClassToolkit::getFieldByName(person, "name"), "Alice");
    EXPECT_EQ(ClassToolkit::getFieldByName(person, "age"), "25");
    EXPECT_EQ(ClassToolkit::getFieldByName(person, "score"), "95.5");
}

/**
 * @brief Test getFieldByName with invalid field name throws exception
 * @details Validates proper error handling
 */
TEST(ClassToolkitTest, GetFieldByName_InvalidFieldName_ThrowsException) {
    const TestPerson person{"Bob", 30, 88.5};
    
    EXPECT_THROW(ClassToolkit::getFieldByName(person, "invalid_field"), std::invalid_argument);
}

/**
 * @brief Test compareObjects with identical objects
 * @details Verifies true is returned when all fields match
 */
TEST(ClassToolkitTest, CompareObjects_IdenticalObjects) {
    const TestPerson person1{"Alice", 25, 95.5};
    const TestPerson person2{"Alice", 25, 95.5};
    
    EXPECT_TRUE(ClassToolkit::compareObjects(person1, person2));
}

/**
 * @brief Test compareObjects with different objects
 * @details Verifies false is returned when any field differs
 */
TEST(ClassToolkitTest, CompareObjects_DifferentObjects) {
    const TestPerson person1{"Alice", 25, 95.5};
    const TestPerson person2{"Bob", 25, 95.5};
    
    EXPECT_FALSE(ClassToolkit::compareObjects(person1, person2));
}

/**
 * @brief Test getObjectDiff with identical objects
 * @details Verifies empty diff is returned
 */
TEST(ClassToolkitTest, GetObjectDiff_IdenticalObjects) {
    const TestPerson person1{"Alice", 25, 95.5};
    const TestPerson person2{"Alice", 25, 95.5};
    
    auto diff = ClassToolkit::getObjectDiff(person1, person2);
    EXPECT_TRUE(diff.empty());
}

/**
 * @brief Test getObjectDiff with different objects
 * @details Verifies only different fields are included in diff
 */
TEST(ClassToolkitTest, GetObjectDiff_DifferentObjects) {
    const TestPerson person1{"Alice", 25, 95.5};
    const TestPerson person2{"Bob", 30, 88.5};
    
    auto diff = ClassToolkit::getObjectDiff(person1, person2);
    
    EXPECT_EQ(diff.size(), 3); // All fields are different
    EXPECT_EQ(diff["name"].first, "Alice");
    EXPECT_EQ(diff["name"].second, "Bob");
}

/**
 * @brief Test getObjectDiff with partially different objects
 * @details Verifies only changed fields are in diff
 */
TEST(ClassToolkitTest, GetObjectDiff_PartiallyDifferent) {
    const TestPerson person1{"Alice", 25, 95.5};
    const TestPerson person2{"Alice", 30, 95.5};
    
    auto diff = ClassToolkit::getObjectDiff(person1, person2);
    
    EXPECT_EQ(diff.size(), 1); // Only age is different
    EXPECT_EQ(diff["age"].first, "25");
    EXPECT_EQ(diff["age"].second, "30");
}

/**
 * @brief Test getTypeInfo returns comprehensive information
 * @details Verifies type info string contains expected sections
 */
TEST(ClassToolkitTest, GetTypeInfo_ComprehensiveInformation) {
    auto info = ClassToolkit::getTypeInfo<int>();
    
    EXPECT_NE(info.find("Type:"), std::string::npos);
    EXPECT_NE(info.find("Size:"), std::string::npos);
    EXPECT_NE(info.find("Alignment:"), std::string::npos);
    EXPECT_NE(info.find("Is Polymorphic:"), std::string::npos);
}

/**
 * @brief Test getTypeInfo for custom class
 * @details Verifies type info includes class name
 */
TEST(ClassToolkitTest, GetTypeInfo_CustomClass) {
    auto info = ClassToolkit::getTypeInfo<TestPerson>();
    
    EXPECT_NE(info.find("TestPerson"), std::string::npos);
}