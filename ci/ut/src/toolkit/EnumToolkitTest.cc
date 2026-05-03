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
    Success = 0, Failed = -1, Pending = 1,
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

/**
 * @brief Test getAllEnumValues with basic enum
 * @details Verifies all enum values are returned in a vector
 */
TEST(EnumToolkitTest, GetAllEnumValues_BasicEnum) {
    const auto values = EnumToolkit::getAllEnumValues<Color>();
    
    EXPECT_EQ(values.size(), 4);
    EXPECT_EQ(values[0], Color::Red);
    EXPECT_EQ(values[1], Color::Green);
    EXPECT_EQ(values[2], Color::Blue);
    EXPECT_EQ(values[3], Color::Yellow);
}

/**
 * @brief Test getAllEnumValues with custom value enum
 * @details Verifies all enum values including negative values are returned
 */
TEST(EnumToolkitTest, GetAllEnumValues_CustomValueEnum) {
    const auto values = EnumToolkit::getAllEnumValues<Status>();
    
    EXPECT_EQ(values.size(), 3);
    // Values may not be in declaration order, so we check if all are present
    bool has_success = false, has_failed = false, has_pending = false;
    for (const auto& val : values) {
        if (val == Status::Success) has_success = true;
        if (val == Status::Failed) has_failed = true;
        if (val == Status::Pending) has_pending = true;
    }
    EXPECT_TRUE(has_success);
    EXPECT_TRUE(has_failed);
    EXPECT_TRUE(has_pending);
}

/**
 * @brief Test getAllEnumNames with basic enum
 * @details Verifies all enum names are returned as strings
 */
TEST(EnumToolkitTest, GetAllEnumNames_BasicEnum) {
    const auto names = EnumToolkit::getAllEnumNames<Color>();
    
    EXPECT_EQ(names.size(), 4);
    EXPECT_EQ(names[0], "Red");
    EXPECT_EQ(names[1], "Green");
    EXPECT_EQ(names[2], "Blue");
    EXPECT_EQ(names[3], "Yellow");
}

/**
 * @brief Test getAllEnumNames with custom value enum
 * @details Verifies all enum names are returned correctly
 */
TEST(EnumToolkitTest, GetAllEnumNames_CustomValueEnum) {
    const auto names = EnumToolkit::getAllEnumNames<Status>();
    
    EXPECT_EQ(names.size(), 3);
    bool has_success = false, has_failed = false, has_pending = false;
    for (const auto& name : names) {
        if (name == "Success") has_success = true;
        if (name == "Failed") has_failed = true;
        if (name == "Pending") has_pending = true;
    }
    EXPECT_TRUE(has_success);
    EXPECT_TRUE(has_failed);
    EXPECT_TRUE(has_pending);
}

/**
 * @brief Test isValidEnumName with valid names
 * @details Verifies valid enum names return true
 */
TEST(EnumToolkitTest, IsValidEnumName_ValidNames) {
    EXPECT_TRUE(EnumToolkit::isValidEnumName<Color>("Red"));
    EXPECT_TRUE(EnumToolkit::isValidEnumName<Color>("Green"));
    EXPECT_TRUE(EnumToolkit::isValidEnumName<Color>("Blue"));
    EXPECT_TRUE(EnumToolkit::isValidEnumName<Color>("Yellow"));
}

/**
 * @brief Test isValidEnumName with invalid names
 * @details Verifies invalid enum names return false
 */
TEST(EnumToolkitTest, IsValidEnumName_InvalidNames) {
    EXPECT_FALSE(EnumToolkit::isValidEnumName<Color>("InvalidColor"));
    EXPECT_FALSE(EnumToolkit::isValidEnumName<Color>(""));
    EXPECT_FALSE(EnumToolkit::isValidEnumName<Color>("red")); // case-sensitive
}

/**
 * @brief Test getEnumCount with basic enum
 * @details Verifies correct count of enum values
 */
TEST(EnumToolkitTest, GetEnumCount_BasicEnum) {
    EXPECT_EQ(EnumToolkit::getEnumCount<Color>(), 4);
}

/**
 * @brief Test getEnumCount with custom value enum
 * @details Verifies correct count for enums with custom values
 */
TEST(EnumToolkitTest, GetEnumCount_CustomValueEnum) {
    EXPECT_EQ(EnumToolkit::getEnumCount<Status>(), 3);
}

/**
 * @brief Test enumToInteger with basic enum
 * @details Verifies conversion to underlying integer values
 */
TEST(EnumToolkitTest, EnumToInteger_BasicEnum) {
    EXPECT_EQ(EnumToolkit::enumToInteger(Color::Red), 0);
    EXPECT_EQ(EnumToolkit::enumToInteger(Color::Green), 1);
    EXPECT_EQ(EnumToolkit::enumToInteger(Color::Blue), 2);
    EXPECT_EQ(EnumToolkit::enumToInteger(Color::Yellow), 3);
}

/**
 * @brief Test enumToInteger with custom value enum
 * @details Verifies conversion preserves custom integer values
 */
TEST(EnumToolkitTest, EnumToInteger_CustomValueEnum) {
    EXPECT_EQ(EnumToolkit::enumToInteger(Status::Success), 0);
    EXPECT_EQ(EnumToolkit::enumToInteger(Status::Failed), -1);
    EXPECT_EQ(EnumToolkit::enumToInteger(Status::Pending), 1);
}

/**
 * @brief Test integerToEnum with valid integers
 * @details Verifies safe conversion from integers to enum values
 */
TEST(EnumToolkitTest, IntegerToEnum_ValidIntegers) {
    const auto red = EnumToolkit::integerToEnum<Color>(0);
    const auto green = EnumToolkit::integerToEnum<Color>(1);
    const auto blue = EnumToolkit::integerToEnum<Color>(2);
    
    ASSERT_TRUE(red.has_value());
    EXPECT_EQ(red.value(), Color::Red);
    
    ASSERT_TRUE(green.has_value());
    EXPECT_EQ(green.value(), Color::Green);
    
    ASSERT_TRUE(blue.has_value());
    EXPECT_EQ(blue.value(), Color::Blue);
}

/**
 * @brief Test integerToEnum with invalid integers
 * @details Verifies empty optional is returned for out-of-range values
 */
TEST(EnumToolkitTest, IntegerToEnum_InvalidIntegers) {
    const auto invalid = EnumToolkit::integerToEnum<Color>(999);
    EXPECT_FALSE(invalid.has_value());
}

/**
 * @brief Test integerToEnum with custom value enum
 * @details Verifies conversion works with negative and non-sequential values
 */
TEST(EnumToolkitTest, IntegerToEnum_CustomValueEnum) {
    const auto success = EnumToolkit::integerToEnum<Status>(0);
    const auto failed = EnumToolkit::integerToEnum<Status>(-1);
    const auto pending = EnumToolkit::integerToEnum<Status>(1);
    
    ASSERT_TRUE(success.has_value());
    EXPECT_EQ(success.value(), Status::Success);
    
    ASSERT_TRUE(failed.has_value());
    EXPECT_EQ(failed.value(), Status::Failed);
    
    ASSERT_TRUE(pending.has_value());
    EXPECT_EQ(pending.value(), Status::Pending);
}

/**
 * @brief Test integerToEnum round-trip conversion
 * @details Verifies enum -> int -> enum conversion preserves value
 */
TEST(EnumToolkitTest, IntegerToEnum_RoundTrip) {
    constexpr auto original = Color::Yellow;
    const auto int_value = EnumToolkit::enumToInteger(original);
    const auto converted = EnumToolkit::integerToEnum<Color>(int_value);
    
    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted.value(), original);
}

/**
 * @brief Test formatEnumInfo output
 * @details Verifies formatted string contains expected information
 */
TEST(EnumToolkitTest, FormatEnumInfo_BasicEnum) {
    const auto info = EnumToolkit::formatEnumInfo<Color>();
    
    EXPECT_NE(info.find("Enum Type:"), std::string::npos);
    EXPECT_NE(info.find("Color"), std::string::npos);
    EXPECT_NE(info.find("Count:"), std::string::npos);
    EXPECT_NE(info.find("4"), std::string::npos);
    EXPECT_NE(info.find("Red"), std::string::npos);
    EXPECT_NE(info.find("Green"), std::string::npos);
    EXPECT_NE(info.find("Blue"), std::string::npos);
    EXPECT_NE(info.find("Yellow"), std::string::npos);
}

/**
 * @brief Test formatEnumInfo with custom value enum
 * @details Verifies formatted string includes custom values
 */
TEST(EnumToolkitTest, FormatEnumInfo_CustomValueEnum) {
    const auto info = EnumToolkit::formatEnumInfo<Status>();
    
    EXPECT_NE(info.find("Enum Type:"), std::string::npos);
    EXPECT_NE(info.find("Status"), std::string::npos);
    EXPECT_NE(info.find("Count:"), std::string::npos);
    EXPECT_NE(info.find("3"), std::string::npos);
    EXPECT_NE(info.find("Success"), std::string::npos);
    EXPECT_NE(info.find("Failed"), std::string::npos);
    EXPECT_NE(info.find("Pending"), std::string::npos);
}