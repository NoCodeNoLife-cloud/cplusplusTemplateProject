/**
 * @file RegexToolkitTest.cc
 * @brief Unit tests for the RegexToolkit class
 * @details Tests cover pattern matching, search, extraction, replacement, and splitting.
 */

#include <gtest/gtest.h>
#include "toolkit/RegexToolkit.hpp"
#include <string>
#include <vector>
#include <stdexcept>

using namespace common::toolkit;

/**
 * @brief Test is_match for full string matching
 * @details Verifies entire string must match the pattern
 */
TEST(RegexToolkitTest, IsMatch_ValidPattern) {
    EXPECT_TRUE(RegexToolkit::is_match("hello123", R"(\w+\d+)"));
    EXPECT_FALSE(RegexToolkit::is_match("hello123!", R"(\w+\d+)"));
}

TEST(RegexToolkitTest, IsMatch_EmailPattern) {
    EXPECT_TRUE(RegexToolkit::is_match("user@example.com", R"([\w.]+@[\w.]+\.\w+)"));
    EXPECT_FALSE(RegexToolkit::is_match("invalid-email", R"([\w.]+@[\w.]+\.\w+)"));
}

TEST(RegexToolkitTest, IsMatch_InvalidPattern) {
    EXPECT_THROW(RegexToolkit::is_match("test", "["), std::invalid_argument);
}

/**
 * @brief Test is_search for partial matching when found
 * @details Verifies substring match returns true
 */
TEST(RegexToolkitTest, IsSearch_Found) {
    EXPECT_TRUE(RegexToolkit::is_search("abc123def", R"(\d+)"));
    EXPECT_TRUE(RegexToolkit::is_search("hello world", R"(world)"));
}

TEST(RegexToolkitTest, IsSearch_NotFound) {
    EXPECT_FALSE(RegexToolkit::is_search("hello world", R"(\d+)"));
}

TEST(RegexToolkitTest, IsSearch_InvalidPattern) {
    EXPECT_THROW(RegexToolkit::is_search("test", "("), std::invalid_argument);
}

/**
 * @brief Test get_matches to extract all matches
 * @details Verifies all occurrences are extracted correctly
 */
TEST(RegexToolkitTest, GetMatches_MultipleNumbers) {
    const auto matches = RegexToolkit::get_matches("a1b2c3d4", R"(\d)");
    
    ASSERT_EQ(matches.size(), 4);
    EXPECT_EQ(matches[0], "1");
    EXPECT_EQ(matches[1], "2");
    EXPECT_EQ(matches[2], "3");
    EXPECT_EQ(matches[3], "4");
}

TEST(RegexToolkitTest, GetMatches_Words) {
    const auto matches = RegexToolkit::get_matches("hello world foo bar", R"(\w+)");
    
    ASSERT_EQ(matches.size(), 4);
    EXPECT_EQ(matches[0], "hello");
    EXPECT_EQ(matches[1], "world");
    EXPECT_EQ(matches[2], "foo");
    EXPECT_EQ(matches[3], "bar");
}

TEST(RegexToolkitTest, GetMatches_NoMatches) {
    const auto matches = RegexToolkit::get_matches("hello world", R"(\d+)");
    EXPECT_TRUE(matches.empty());
}

TEST(RegexToolkitTest, GetMatches_InvalidPattern) {
    EXPECT_THROW(RegexToolkit::get_matches("test", "*"), std::invalid_argument);
}

/**
 * @brief Test get_matches_with_groups to extract capture groups
 * @details Verifies both full match and capture groups are returned
 */
TEST(RegexToolkitTest, GetMatchesWithGroups_DatePattern) {
    const auto matches = RegexToolkit::get_matches_with_groups("2024-01-15 and 2023-12-25", R"((\d{4})-(\d{2})-(\d{2}))");
    
    ASSERT_EQ(matches.size(), 2);
    // First match: full match + 3 groups
    ASSERT_EQ(matches[0].size(), 4);
    EXPECT_EQ(matches[0][0], "2024-01-15");
    EXPECT_EQ(matches[0][1], "2024");
    EXPECT_EQ(matches[0][2], "01");
    EXPECT_EQ(matches[0][3], "15");
    
    // Second match
    ASSERT_EQ(matches[1].size(), 4);
    EXPECT_EQ(matches[1][0], "2023-12-25");
    EXPECT_EQ(matches[1][1], "2023");
    EXPECT_EQ(matches[1][2], "12");
    EXPECT_EQ(matches[1][3], "25");
}

TEST(RegexToolkitTest, GetMatchesWithGroups_EmailPattern) {
    const auto matches = RegexToolkit::get_matches_with_groups("user@example.com", R"((\w+)@(\w+)\.(\w+))");
    
    ASSERT_EQ(matches.size(), 1);
    ASSERT_EQ(matches[0].size(), 4);
    EXPECT_EQ(matches[0][0], "user@example.com");
    EXPECT_EQ(matches[0][1], "user");
    EXPECT_EQ(matches[0][2], "example");
    EXPECT_EQ(matches[0][3], "com");
}

TEST(RegexToolkitTest, GetMatchesWithGroups_InvalidPattern) {
    EXPECT_THROW(RegexToolkit::get_matches_with_groups("test", "[invalid"), std::invalid_argument);
}

/**
 * @brief Test replace_all to replace all occurrences
 * @details Verifies all pattern matches are replaced
 */
TEST(RegexToolkitTest, ReplaceAll_SimpleReplacement) {
    const auto result = RegexToolkit::replace_all("hello world hello", R"(hello)", "hi");
    EXPECT_EQ(result, "hi world hi");
}

TEST(RegexToolkitTest, ReplaceAll_DigitsReplacement) {
    const auto result = RegexToolkit::replace_all("a1b2c3", R"(\d)", "X");
    EXPECT_EQ(result, "aXbXcX");
}

TEST(RegexToolkitTest, ReplaceAll_NoMatches) {
    const auto result = RegexToolkit::replace_all("hello world", R"(\d+)", "X");
    EXPECT_EQ(result, "hello world");
}

TEST(RegexToolkitTest, ReplaceAll_InvalidPattern) {
    EXPECT_THROW(RegexToolkit::replace_all("test", "(", "replacement"), std::invalid_argument);
}

/**
 * @brief Test split to split string by regex delimiter
 * @details Verifies string is correctly split by pattern
 */
TEST(RegexToolkitTest, Split_ByComma) {
    const auto parts = RegexToolkit::split("apple,banana,cherry", R"(,)");
    
    ASSERT_EQ(parts.size(), 3);
    EXPECT_EQ(parts[0], "apple");
    EXPECT_EQ(parts[1], "banana");
    EXPECT_EQ(parts[2], "cherry");
}

TEST(RegexToolkitTest, Split_ByWhitespace) {
    const auto parts = RegexToolkit::split("hello   world\tfoo\nbar", R"(\s+)");
    
    ASSERT_EQ(parts.size(), 4);
    EXPECT_EQ(parts[0], "hello");
    EXPECT_EQ(parts[1], "world");
    EXPECT_EQ(parts[2], "foo");
    EXPECT_EQ(parts[3], "bar");
}

TEST(RegexToolkitTest, Split_ByMultipleDelimiters) {
    const auto parts = RegexToolkit::split("a,b;c.d", R"([,;.])");
    
    ASSERT_EQ(parts.size(), 4);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[1], "b");
    EXPECT_EQ(parts[2], "c");
    EXPECT_EQ(parts[3], "d");
}

TEST(RegexToolkitTest, Split_EmptyResult) {
    const auto parts = RegexToolkit::split("", R"(,)");
    EXPECT_EQ(parts.size(), 1);
    EXPECT_EQ(parts[0], "");
}

TEST(RegexToolkitTest, Split_InvalidPattern) {
    EXPECT_THROW(RegexToolkit::split("test", "["), std::invalid_argument);
}
