#include <gtest/gtest.h>
#include "toolkit/StringToolkit.hpp"
#include <string>
#include <vector>

using namespace common::toolkit;

// Test split with char delimiter
TEST(StringToolkitTest, SplitByChar_Basic) {
    const auto result = StringToolkit::split("apple,banana,cherry", ',');
    
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "apple");
    EXPECT_EQ(result[1], "banana");
    EXPECT_EQ(result[2], "cherry");
}

TEST(StringToolkitTest, SplitByChar_NoDelimiter) {
    const auto result = StringToolkit::split("hello", ',');
    
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello");
}

TEST(StringToolkitTest, SplitByChar_EmptyString) {
    const auto result = StringToolkit::split("", ',');
    
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "");
}

// Test splitWithEmpty with char delimiter
TEST(StringToolkitTest, SplitWithEmpty_ConsecutiveDelimiters) {
    const auto result = StringToolkit::splitWithEmpty("a,,b,c", ',');
    
    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "b");
    EXPECT_EQ(result[3], "c");
}

TEST(StringToolkitTest, SplitWithEmpty_TrailingDelimiter) {
    const auto result = StringToolkit::splitWithEmpty("a,b,", ',');
    
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
    EXPECT_EQ(result[2], "");
}

// Test split with string delimiter
TEST(StringToolkitTest, SplitByString_Basic) {
    const auto result = StringToolkit::split("one::two::three", "::");
    
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "one");
    EXPECT_EQ(result[1], "two");
    EXPECT_EQ(result[2], "three");
}

TEST(StringToolkitTest, SplitByString_EmptyDelimiter) {
    const auto result = StringToolkit::split("hello", "");
    
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello");
}

TEST(StringToolkitTest, SplitByString_NoMatch) {
    const auto result = StringToolkit::split("hello world", "::");
    
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello world");
}

// Test concatenate with char
TEST(StringToolkitTest, ConcatenateByChar_Basic) {
    const std::vector<std::string> parts = {"apple", "banana", "cherry"};
    const auto result = StringToolkit::concatenate(parts, ',');
    
    EXPECT_EQ(result, "apple,banana,cherry");
}

TEST(StringToolkitTest, ConcatenateByChar_EmptyVector) {
    const std::vector<std::string> parts;
    const auto result = StringToolkit::concatenate(parts, ',');
    
    EXPECT_EQ(result, "");
}

TEST(StringToolkitTest, ConcatenateByChar_SingleElement) {
    const std::vector<std::string> parts = {"hello"};
    const auto result = StringToolkit::concatenate(parts, ',');
    
    EXPECT_EQ(result, "hello");
}

// Test concatenate with string delimiter
TEST(StringToolkitTest, ConcatenateByString_Basic) {
    const std::vector<std::string> parts = {"one", "two", "three"};
    const auto result = StringToolkit::concatenate(parts, "::");
    
    EXPECT_EQ(result, "one::two::three");
}

TEST(StringToolkitTest, ConcatenateByString_EmptyVector) {
    const std::vector<std::string> parts;
    const auto result = StringToolkit::concatenate(parts, "::");
    
    EXPECT_EQ(result, "");
}

// Test startsWith
TEST(StringToolkitTest, StartsWith_Match) {
    EXPECT_TRUE(StringToolkit::startsWith("hello world", "hello"));
}

TEST(StringToolkitTest, StartsWith_NoMatch) {
    EXPECT_FALSE(StringToolkit::startsWith("hello world", "world"));
}

TEST(StringToolkitTest, StartsWith_PrefixLongerThanString) {
    EXPECT_FALSE(StringToolkit::startsWith("hi", "hello"));
}

TEST(StringToolkitTest, StartsWith_EmptyPrefix) {
    EXPECT_TRUE(StringToolkit::startsWith("hello", ""));
}

// Test endsWith
TEST(StringToolkitTest, EndsWith_Match) {
    EXPECT_TRUE(StringToolkit::endsWith("hello world", "world"));
}

TEST(StringToolkitTest, EndsWith_NoMatch) {
    EXPECT_FALSE(StringToolkit::endsWith("hello world", "hello"));
}

TEST(StringToolkitTest, EndsWith_SuffixLongerThanString) {
    EXPECT_FALSE(StringToolkit::endsWith("hi", "hello"));
}

TEST(StringToolkitTest, EndsWith_EmptySuffix) {
    EXPECT_TRUE(StringToolkit::endsWith("hello", ""));
}

// Test trim
TEST(StringToolkitTest, Trim_BothSides) {
    const auto result = StringToolkit::trim("  hello world  ");
    EXPECT_EQ(result, "hello world");
}

TEST(StringToolkitTest, Trim_LeadingOnly) {
    const auto result = StringToolkit::trim("   hello");
    EXPECT_EQ(result, "hello");
}

TEST(StringToolkitTest, Trim_TrailingOnly) {
    const auto result = StringToolkit::trim("hello   ");
    EXPECT_EQ(result, "hello");
}

TEST(StringToolkitTest, Trim_NoWhitespace) {
    const auto result = StringToolkit::trim("hello");
    EXPECT_EQ(result, "hello");
}

TEST(StringToolkitTest, Trim_AllWhitespace) {
    const auto result = StringToolkit::trim("   ");
    EXPECT_EQ(result, "");
}

TEST(StringToolkitTest, Trim_EmptyString) {
    const auto result = StringToolkit::trim("");
    EXPECT_EQ(result, "");
}

// Test replaceAll
TEST(StringToolkitTest, ReplaceAll_MultipleOccurrences) {
    const auto result = StringToolkit::replaceAll("hello world hello", "hello", "hi");
    EXPECT_EQ(result, "hi world hi");
}

TEST(StringToolkitTest, ReplaceAll_NoOccurrences) {
    const auto result = StringToolkit::replaceAll("hello world", "xyz", "abc");
    EXPECT_EQ(result, "hello world");
}

TEST(StringToolkitTest, ReplaceAll_EmptyFrom) {
    const auto result = StringToolkit::replaceAll("hello", "", "x");
    EXPECT_EQ(result, "hello");
}

TEST(StringToolkitTest, ReplaceAll_OverlappingReplacement) {
    const auto result = StringToolkit::replaceAll("aaa", "aa", "bb");
    EXPECT_EQ(result, "bba");
}

// Test join (alias for concatenate with string delimiter)
TEST(StringToolkitTest, Join_Basic) {
    const std::vector<std::string> parts = {"2024", "01", "15"};
    const auto result = StringToolkit::join(parts, "-");
    
    EXPECT_EQ(result, "2024-01-15");
}

TEST(StringToolkitTest, Join_EmptyVector) {
    const std::vector<std::string> parts;
    const auto result = StringToolkit::join(parts, ",");
    
    EXPECT_EQ(result, "");
}
