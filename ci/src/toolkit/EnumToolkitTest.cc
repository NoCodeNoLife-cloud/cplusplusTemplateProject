/**
 * @file EnumToolkitTest.cc
 * @brief Unit tests for the EnumToolkit class
 * @details Tests cover enum-to-string conversion, string-to-enum parsing, and round-trip validation.
 */

#include <gtest/gtest.h>
#include "toolkit/EnumToolkit.hpp"
#include <string>
#include <optional>

using namespace common::toolkit;

/**
 * @brief Test enum for testing purposes
 */
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

/**
 * @brief Test getEnumName with basic enum values
 * @details Verifies correct string representation of enum constants
 */
TEST(EnumToolkitTest, GetEnumName_BasicEnum) {
    const auto name_red = EnumToolkit::getEnumName(Color::Red);
    const auto name_green = EnumToolkit::getEnumName(Color::Green);
    const auto name_blue = EnumToolkit::getEnumName(Color::Blue);

    EXPECT_EQ(name_red, "Red");
    EXPECT_EQ(name_green, "Green");
    EXPECT_EQ(name_blue, "Blue");
}

/**
 * @brief Test getEnumName with custom underlying type enum
 * @details Verifies enums with explicit values are handled correctly
 */
TEST(EnumToolkitTest, GetEnumName_EnumWithCustomValues) {
    const auto name_success = EnumToolkit::getEnumName(Status::Success);
    const auto name_failed = EnumToolkit::getEnumName(Status::Failed);
    const auto name_pending = EnumToolkit::getEnumName(Status::Pending);

    EXPECT_EQ(name_success, "Success");
    EXPECT_EQ(name_failed, "Failed");
    EXPECT_EQ(name_pending, "Pending");
}

/**
 * @brief Test getEnumTypeName for basic enum
 * @details Verifies enum type name extraction
 */
TEST(EnumToolkitTest, GetEnumTypeName_BasicEnum) {
    const auto type_name = EnumToolkit::getEnumTypeName<Color>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("Color"), std::string::npos);
}

/**
 * @brief Test getEnumTypeName for enum with underlying type
 * @details Verifies type name includes enum identifier
 */
TEST(EnumToolkitTest, GetEnumTypeName_EnumWithUnderlyingType) {
    const auto type_name = EnumToolkit::getEnumTypeName<Status>();

    EXPECT_FALSE(type_name.empty());
    EXPECT_NE(type_name.find("Status"), std::string::npos);
}

/**
 * @brief Test stringToEnum with valid strings
 * @details Verifies correct parsing of enum names to enum values
 */
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

/**
 * @brief Test stringToEnum with invalid strings
 * @details Verifies std::nullopt is returned for unrecognized names
 */
TEST(EnumToolkitTest, StringToEnum_InvalidString) {
    const auto invalid = EnumToolkit::stringToEnum<Color>("InvalidColor");
    const auto empty = EnumToolkit::stringToEnum<Color>("");

    EXPECT_FALSE(invalid.has_value());
    EXPECT_FALSE(empty.has_value());
}

/**
 * @brief Test stringToEnum case sensitivity
 * @details Verifies that parsing is case-sensitive (magic_enum default behavior)
 */
TEST(EnumToolkitTest, StringToEnum_CaseSensitive) {
    // magic_enum is case-sensitive by default
    const auto lowercase_red = EnumToolkit::stringToEnum<Color>("red");
    const auto uppercase_red = EnumToolkit::stringToEnum<Color>("RED");

    EXPECT_FALSE(lowercase_red.has_value());
    EXPECT_FALSE(uppercase_red.has_value());
}

/**
 * @brief Test stringToEnum with custom value enum
 * @details Verifies parsing works for enums with non-sequential values
 */
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

/**
 * @brief Test round-trip conversion: enum to string and back
 * @details Verifies bidirectional conversion preserves original value
 */
TEST(EnumToolkitTest, RoundTrip_EnumToStringAndBack) {
    constexpr auto original = Color::Yellow;
    const auto name = EnumToolkit::getEnumName(original);
    const auto converted = EnumToolkit::stringToEnum<Color>(name);

    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted.value(), original);
}

/**
 * @brief Test round-trip conversion for Status enum
 * @details Verifies bidirectional conversion for enums with custom values
 */
TEST(EnumToolkitTest, RoundTrip_StatusEnum) {
    constexpr auto original = Status::Failed;
    const auto name = EnumToolkit::getEnumName(original);
    const auto converted = EnumToolkit::stringToEnum<Status>(name);

    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted.value(), original);
}

/**
 * @brief Test that constructor is deleted (compile-time check)
 * @details Verifies EnumToolkit cannot be instantiated as it's a utility class
 */
TEST(EnumToolkitTest, ConstructorDeleted) {
    // This test verifies at compile time that EnumToolkit cannot be instantiated
    static_assert(std::is_constructible_v<EnumToolkit> == false,
                  "EnumToolkit should not be constructible");
}
