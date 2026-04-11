#include <gtest/gtest.h>
#include "toolkit/EnumToolkit.hpp"
#include <string>
#include <optional>

using namespace common::toolkit;

// Test enum for testing purposes
enum class Color {
    Red,
    Green,
    Blue,
    Yellow,
};

enum class Status : int {
    Success = 0,
    Failed = -1,
    Pending = 1,
};

// Test getEnumName functionality
TEST(EnumToolkitTest, GetEnumName_BasicEnum) {
    const auto name_red = EnumToolkit::getEnumName(Color::Red);
    const auto name_green = EnumToolkit::getEnumName(Color::Green);
    const auto name_blue = EnumToolkit::getEnumName(Color::Blue);

    EXPECT_EQ(name_red, "Red");
    EXPECT_EQ(name_green, "Green");
    EXPECT_EQ(name_blue, "Blue");
}

TEST(EnumToolkitTest, GetEnumName_EnumWithCustomValues) {
    const auto name_success = EnumToolkit::getEnumName(Status::Success);
    const auto name_failed = EnumToolkit::getEnumName(Status::Failed);
    const auto name_pending = EnumToolkit::getEnumName(Status::Pending);

    EXPECT_EQ(name_success, "Success");
    EXPECT_EQ(name_failed, "Failed");
    EXPECT_EQ(name_pending, "Pending");
}

// Test getEnumTypeName functionality
TEST(EnumToolkitTest, GetEnumTypeName_BasicEnum) {
    const auto type_name = EnumToolkit::getEnumTypeName<Color>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("Color"), std::string::npos);
}

TEST(EnumToolkitTest, GetEnumTypeName_EnumWithUnderlyingType) {
    const auto type_name = EnumToolkit::getEnumTypeName<Status>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("Status"), std::string::npos);
}

// Test stringToEnum functionality
TEST(EnumToolkitTest, StringToEnum_ValidString) {
    const auto red = EnumToolkit::stringToEnum<Color>("Red");
    const auto green = EnumToolkit::stringToEnum<Color>("Green");
    const auto blue = EnumToolkit::stringToEnum<Color>("Blue");

    ASSERT_TRUE(red.has_value());
    EXPECT_EQ(red.value(), Color::Red);

    ASSERT_TRUE(green.has_value());
    EXPECT_EQ(green.value(), Color::Green);

    ASSERT_TRUE(blue.has_value());
    EXPECT_EQ(blue.value(), Color::Blue);
}

TEST(EnumToolkitTest, StringToEnum_InvalidString) {
    const auto invalid = EnumToolkit::stringToEnum<Color>("InvalidColor");
    const auto empty = EnumToolkit::stringToEnum<Color>("");

    EXPECT_FALSE(invalid.has_value());
    EXPECT_FALSE(empty.has_value());
}

TEST(EnumToolkitTest, StringToEnum_CaseSensitive) {
    // magic_enum is case-sensitive by default
    const auto lowercase_red = EnumToolkit::stringToEnum<Color>("red");
    const auto uppercase_red = EnumToolkit::stringToEnum<Color>("RED");

    EXPECT_FALSE(lowercase_red.has_value());
    EXPECT_FALSE(uppercase_red.has_value());
}

TEST(EnumToolkitTest, StringToEnum_EnumWithCustomValues) {
    const auto success = EnumToolkit::stringToEnum<Status>("Success");
    const auto failed = EnumToolkit::stringToEnum<Status>("Failed");
    const auto pending = EnumToolkit::stringToEnum<Status>("Pending");

    ASSERT_TRUE(success.has_value());
    EXPECT_EQ(success.value(), Status::Success);

    ASSERT_TRUE(failed.has_value());
    EXPECT_EQ(failed.value(), Status::Failed);

    ASSERT_TRUE(pending.has_value());
    EXPECT_EQ(pending.value(), Status::Pending);
}

// Test round-trip conversion
TEST(EnumToolkitTest, RoundTrip_EnumToStringAndBack) {
    constexpr auto original = Color::Yellow;
    const auto name = EnumToolkit::getEnumName(original);
    const auto converted = EnumToolkit::stringToEnum<Color>(name);

    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted.value(), original);
}

TEST(EnumToolkitTest, RoundTrip_StatusEnum) {
    constexpr auto original = Status::Failed;
    const auto name = EnumToolkit::getEnumName(original);
    const auto converted = EnumToolkit::stringToEnum<Status>(name);

    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted.value(), original);
}

// Test that constructor is deleted (compile-time check)
TEST(EnumToolkitTest, ConstructorDeleted) {
    // This test verifies at compile time that EnumToolkit cannot be instantiated
    static_assert(std::is_constructible_v<EnumToolkit> == false,
                  "EnumToolkit should not be constructible");
}
