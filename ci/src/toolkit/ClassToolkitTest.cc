#include <gtest/gtest.h>
#include "toolkit/ClassToolkit.hpp"
#include <string>
#include <type_traits>

using namespace common::toolkit;

// Simple test class for reflection tests
struct TestPerson {
    std::string name;
    int age;
    double score;
};

// ReflectTraits specialization for TestPerson
template<>
struct ReflectTraits<TestPerson> {
    static constexpr std::size_t field_count = 3;
    static constexpr auto fields = std::make_tuple(
        std::make_pair("name", &TestPerson::name),
        std::make_pair("age", &TestPerson::age),
        std::make_pair("score", &TestPerson::score)
    );
};

// Test getTypeId with object instance
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

TEST(ClassToolkitTest, GetTypeId_CustomClass) {
    const TestPerson person{"Alice", 25, 95.5};
    auto type_name = ClassToolkit::getTypeId(person);

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("TestPerson"), std::string::npos);
}

// Test getTypeIdWithCvr with object instance
TEST(ClassToolkitTest, GetTypeIdWithCvr_BasicTypes) {
    constexpr int const_int = 100;
    // Note: getTypeIdWithCvr deduces T from parameter type
    // For const int&, T is deduced as int (not const int)
    auto type_with_cvr = ClassToolkit::getTypeIdWithCvr(const_int);

    EXPECT_FALSE(type_with_cvr.empty());
    // The type should be "int" since template deduction strips top-level const
    EXPECT_NE(type_with_cvr.find("int"), std::string::npos);
}

// Test getTypeIdByClass without instance
TEST(ClassToolkitTest, GetTypeIdByClass_IntType) {
    auto type_name = ClassToolkit::getTypeIdByClass<int>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("int"), std::string::npos);
}

TEST(ClassToolkitTest, GetTypeIdByClass_DoubleType) {
    auto type_name = ClassToolkit::getTypeIdByClass<double>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("double"), std::string::npos);
}

TEST(ClassToolkitTest, GetTypeIdByClass_StringType) {
    auto type_name = ClassToolkit::getTypeIdByClass<std::string>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("string"), std::string::npos);
}

TEST(ClassToolkitTest, GetTypeIdByClass_CustomClass) {
    auto type_name = ClassToolkit::getTypeIdByClass<TestPerson>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("TestPerson"), std::string::npos);
}

// Test getTypeIdWithCvrByClass without instance
TEST(ClassToolkitTest, GetTypeIdWithCvrByClass_ConstInt) {
    auto type_name = ClassToolkit::getTypeIdWithCvrByClass<const int>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("const"), std::string::npos);
    EXPECT_NE(type_name.find("int"), std::string::npos);
}

TEST(ClassToolkitTest, GetTypeIdWithCvrByClass_VolatileDouble) {
    auto type_name = ClassToolkit::getTypeIdWithCvrByClass<volatile double>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("volatile"), std::string::npos);
    EXPECT_NE(type_name.find("double"), std::string::npos);
}

// Test consistency between different methods
TEST(ClassToolkitTest, Consistency_ByClassVsByObject) {
    constexpr int value = 42;

    const auto by_class = ClassToolkit::getTypeIdByClass<int>();
    const auto by_object = ClassToolkit::getTypeId(value);

    EXPECT_EQ(by_class, by_object);
}

TEST(ClassToolkitTest, Consistency_CvrMethods) {
    constexpr int const_value = 100;

    // Both should deduce as "int" (template deduction strips top-level const)
    const auto by_class = ClassToolkit::getTypeIdWithCvrByClass<int>();
    const auto by_object = ClassToolkit::getTypeIdWithCvr(const_value);

    EXPECT_EQ(by_class, by_object);
}

// Test getFields functionality
TEST(ClassToolkitTest, GetFields_BasicStructure) {
    const TestPerson person{"Bob", 30, 88.5};
    auto fields = ClassToolkit::getFields(person);

    EXPECT_EQ(fields.size(), 3);
    EXPECT_EQ(fields["name"], "Bob");
    EXPECT_EQ(fields["age"], "30");
    EXPECT_EQ(fields["score"], "88.5");
}

TEST(ClassToolkitTest, GetFields_EmptyString) {
    const TestPerson person{"", 0, 0.0};
    auto fields = ClassToolkit::getFields(person);

    EXPECT_EQ(fields.size(), 3);
    EXPECT_EQ(fields["name"], "");
    EXPECT_EQ(fields["age"], "0");
    EXPECT_EQ(fields["score"], "0");
}

TEST(ClassToolkitTest, GetFields_NegativeValues) {
    const TestPerson person{"Charlie", -5, -10.5};
    auto fields = ClassToolkit::getFields(person);

    EXPECT_EQ(fields.size(), 3);
    EXPECT_EQ(fields["name"], "Charlie");
    EXPECT_EQ(fields["age"], "-5");
    EXPECT_EQ(fields["score"], "-10.5");
}

// Test that constructor is deleted (compile-time check)
TEST(ClassToolkitTest, ConstructorDeleted) {
    // This test verifies at compile time that ClassToolkit cannot be instantiated
    // If the following line compiles, the test fails
    static_assert(std::is_constructible_v<ClassToolkit> == false,
                  "ClassToolkit should not be constructible");
}
