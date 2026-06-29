/**
 * @file StringToolkitTest.cc
 * @brief Unit tests for the StringToolkit class
 * @details Tests cover string splitting, concatenation, trimming, replacement, and prefix/suffix checking.
 */

#include <string>
#include <vector>
#include <gtest/gtest.h>

#include <cppforge/toolkit/StringToolkit.hpp>

using namespace cppforge::toolkit;

/**
 * @brief Test fixture for StringToolkit tests
 */
class StringToolkitTest : public testing::Test
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
 * @brief Test split with char delimiter - basic case
 * @details Verifies string is correctly split by character delimiter
 */
TEST_F(StringToolkitTest, SplitByChar_Basic)
{
    const auto result = StringToolkit::split("apple,banana,cherry", ',');

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "apple");
    EXPECT_EQ(result[1], "banana");
    EXPECT_EQ(result[2], "cherry");
}

/**
 * @brief Test split by char when no delimiter is present
 * @details Verifies the entire string is returned as a single element
 */
TEST_F(StringToolkitTest, SplitByChar_NoDelimiter)
{
    const auto result = StringToolkit::split("hello", ',');

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello");
}

/**
 * @brief Test split by char on an empty string
 * @details Verifies a single-element vector containing "" is returned
 */
TEST_F(StringToolkitTest, SplitByChar_EmptyString)
{
    const auto result = StringToolkit::split("", ',');

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "");
}

/**
 * @brief Test splitWithEmpty with consecutive delimiters
 * @details Verifies empty strings are inserted between consecutive delimiters,
 *          and non-delimiter trailing content is preserved as a normal token
 */
TEST_F(StringToolkitTest, SplitWithEmpty_ConsecutiveDelimiters)
{
    const auto result = StringToolkit::splitWithEmpty("a,,b,c", ',');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "b");
    EXPECT_EQ(result[3], "c");
}

/**
 * @brief Test splitWithEmpty with a trailing delimiter
 * @details Verifies an empty string is appended for trailing delimiter
 */
TEST_F(StringToolkitTest, SplitWithEmpty_TrailingDelimiter)
{
    const auto result = StringToolkit::splitWithEmpty("a,b,", ',');

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
    EXPECT_EQ(result[2], "");
}

/**
 * @brief Test split by string delimiter â€?basic case
 * @details Verifies multi-character delimiter splits correctly
 */
TEST_F(StringToolkitTest, SplitByString_Basic)
{
    const auto result = StringToolkit::split("one::two::three", "::");

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "one");
    EXPECT_EQ(result[1], "two");
    EXPECT_EQ(result[2], "three");
}

/**
 * @brief Test split by string with an empty delimiter
 * @details Verifies the entire string is returned when delimiter is empty
 */
TEST_F(StringToolkitTest, SplitByString_EmptyDelimiter)
{
    const auto result = StringToolkit::split("hello", "");

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello");
}

/**
 * @brief Test split by string when delimiter is not found
 * @details Verifies the entire string is returned as a single element
 */
TEST_F(StringToolkitTest, SplitByString_NoMatch)
{
    const auto result = StringToolkit::split("hello world", "::");

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello world");
}

/**
 * @brief Test concatenate with char delimiter - basic case
 * @details Verifies vector of strings is joined with character delimiter
 */
TEST_F(StringToolkitTest, ConcatenateByChar_Basic)
{
    const std::vector<std::string> parts = {"apple", "banana", "cherry"};
    const auto result = StringToolkit::join(parts, ",");

    EXPECT_EQ(result, "apple,banana,cherry");
}

/**
 * @brief Test concatenate (join) with an empty vector
 * @details Verifies empty string is returned for empty vector
 */
TEST_F(StringToolkitTest, ConcatenateByChar_EmptyVector)
{
    const std::vector<std::string> parts;
    const auto result = StringToolkit::join(parts, ",");

    EXPECT_EQ(result, "");
}

/**
 * @brief Test concatenate (join) with a single element
 * @details Verifies single element is returned without delimiter
 */
TEST_F(StringToolkitTest, ConcatenateByChar_SingleElement)
{
    const std::vector<std::string> parts = {"hello"};
    const auto result = StringToolkit::join(parts, ",");

    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test concatenate (join) with a multi-character delimiter
 * @details Verifies joining with string delimiter "::" works correctly
 */
TEST_F(StringToolkitTest, ConcatenateByString_Basic)
{
    const std::vector<std::string> parts = {"one", "two", "three"};
    const auto result = StringToolkit::join(parts, "::");

    EXPECT_EQ(result, "one::two::three");
}

/**
 * @brief Test concatenate (join) with string delimiter on empty vector
 * @details Verifies empty string is returned for empty vector
 */
TEST_F(StringToolkitTest, ConcatenateByString_EmptyVector)
{
    const std::vector<std::string> parts;
    const auto result = StringToolkit::join(parts, "::");

    EXPECT_EQ(result, "");
}

/**
 * @brief Test startsWith functionality
 * @details Verifies prefix detection works correctly
 */
TEST_F(StringToolkitTest, StartsWith_Match)
{
    EXPECT_TRUE(StringToolkit::startsWith("hello world", "hello"));
}

/**
 * @brief Test startsWith when prefix is not present
 * @details Verifies false is returned for non-matching prefix
 */
TEST_F(StringToolkitTest, StartsWith_NoMatch)
{
    EXPECT_FALSE(StringToolkit::startsWith("hello world", "world"));
}

/**
 * @brief Test startsWith when prefix is longer than the string
 * @details Verifies false is returned for over-long prefix
 */
TEST_F(StringToolkitTest, StartsWith_PrefixLongerThanString)
{
    EXPECT_FALSE(StringToolkit::startsWith("hi", "hello"));
}

/**
 * @brief Test startsWith with an empty prefix
 * @details Verifies true is returned for empty prefix (vacuously true)
 */
TEST_F(StringToolkitTest, StartsWith_EmptyPrefix)
{
    EXPECT_TRUE(StringToolkit::startsWith("hello", ""));
}

/**
 * @brief Test endsWith functionality
 * @details Verifies suffix detection works correctly
 */
TEST_F(StringToolkitTest, EndsWith_Match)
{
    EXPECT_TRUE(StringToolkit::endsWith("hello world", "world"));
}

/**
 * @brief Test endsWith when suffix is not present
 * @details Verifies false is returned for non-matching suffix
 */
TEST_F(StringToolkitTest, EndsWith_NoMatch)
{
    EXPECT_FALSE(StringToolkit::endsWith("hello world", "hello"));
}

/**
 * @brief Test endsWith when suffix is longer than the string
 * @details Verifies false is returned for over-long suffix
 */
TEST_F(StringToolkitTest, EndsWith_SuffixLongerThanString)
{
    EXPECT_FALSE(StringToolkit::endsWith("hi", "hello"));
}

/**
 * @brief Test endsWith with an empty suffix
 * @details Verifies true is returned for empty suffix (vacuously true)
 */
TEST_F(StringToolkitTest, EndsWith_EmptySuffix)
{
    EXPECT_TRUE(StringToolkit::endsWith("hello", ""));
}

/**
 * @brief Test trim functionality for both sides
 * @details Verifies leading and trailing whitespace is removed
 */
TEST_F(StringToolkitTest, Trim_BothSides)
{
    const auto result = StringToolkit::trim("  hello world  ");
    EXPECT_EQ(result, "hello world");
}

/**
 * @brief Test trim with only leading whitespace
 * @details Verifies leading whitespace is removed, trailing is untouched
 */
TEST_F(StringToolkitTest, Trim_LeadingOnly)
{
    const auto result = StringToolkit::trim("   hello");
    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test trim with only trailing whitespace
 * @details Verifies trailing whitespace is removed, leading is untouched
 */
TEST_F(StringToolkitTest, Trim_TrailingOnly)
{
    const auto result = StringToolkit::trim("hello   ");
    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test trim with no whitespace present
 * @details Verifies unchanged string is returned
 */
TEST_F(StringToolkitTest, Trim_NoWhitespace)
{
    const auto result = StringToolkit::trim("hello");
    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test trim with all whitespace
 * @details Verifies empty string is returned
 */
TEST_F(StringToolkitTest, Trim_AllWhitespace)
{
    const auto result = StringToolkit::trim("   ");
    EXPECT_EQ(result, "");
}

/**
 * @brief Test trim on an empty string
 * @details Verifies empty string is returned
 */
TEST_F(StringToolkitTest, Trim_EmptyString)
{
    const auto result = StringToolkit::trim("");
    EXPECT_EQ(result, "");
}

/**
 * @brief Test replaceAll functionality for multiple occurrences
 * @details Verifies all occurrences are replaced
 */
TEST_F(StringToolkitTest, ReplaceAll_MultipleOccurrences)
{
    const auto result = StringToolkit::replaceAll("hello world hello", "hello", "hi");
    EXPECT_EQ(result, "hi world hi");
}

/**
 * @brief Test replaceAll when no occurrences exist
 * @details Verifies the original string is returned unchanged
 */
TEST_F(StringToolkitTest, ReplaceAll_NoOccurrences)
{
    const auto result = StringToolkit::replaceAll("hello world", "xyz", "abc");
    EXPECT_EQ(result, "hello world");
}

/**
 * @brief Test replaceAll with an empty search string
 * @details Verifies the original string is returned when from is empty
 */
TEST_F(StringToolkitTest, ReplaceAll_EmptyFrom)
{
    const auto result = StringToolkit::replaceAll("hello", "", "x");
    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test replaceAll with overlapping patterns
 * @details Verifies overlapping matches are handled correctly (left-to-right)
 */
TEST_F(StringToolkitTest, ReplaceAll_OverlappingReplacement)
{
    const auto result = StringToolkit::replaceAll("aaa", "aa", "bb");
    EXPECT_EQ(result, "bba");
}

/**
 * @brief Test join (alias for concatenate with string delimiter)
 * @details Verifies string joining with custom delimiter
 */
TEST_F(StringToolkitTest, Join_Basic)
{
    const std::vector<std::string> parts = {"2024", "01", "15"};
    const auto result = StringToolkit::join(parts, "-");

    EXPECT_EQ(result, "2024-01-15");
}

/**
 * @brief Test join with an empty vector
 * @details Verifies empty string is returned when no elements to join
 */
TEST_F(StringToolkitTest, Join_EmptyVector)
{
    const std::vector<std::string> parts;
    const auto result = StringToolkit::join(parts, ",");

    EXPECT_EQ(result, "");
}

/**
 * @brief Test toUpperCase functionality
 * @details Verifies string is converted to uppercase correctly
 */
TEST_F(StringToolkitTest, ToUpperCase_Basic)
{
    const auto result = StringToolkit::toUpperCase("hello world");
    EXPECT_EQ(result, "HELLO WORLD");
}

/**
 * @brief Test toUpperCase on an already-uppercase string
 * @details Verifies idempotency â€?uppercase input yields same output
 */
TEST_F(StringToolkitTest, ToUpperCase_AlreadyUpper)
{
    const auto result = StringToolkit::toUpperCase("HELLO");
    EXPECT_EQ(result, "HELLO");
}

/**
 * @brief Test toUpperCase with mixed case input
 * @details Verifies all characters are converted to uppercase
 */
TEST_F(StringToolkitTest, ToUpperCase_MixedCase)
{
    const auto result = StringToolkit::toUpperCase("Hello World");
    EXPECT_EQ(result, "HELLO WORLD");
}

/**
 * @brief Test toUpperCase on an empty string
 * @details Verifies empty string is returned unchanged
 */
TEST_F(StringToolkitTest, ToUpperCase_EmptyString)
{
    const auto result = StringToolkit::toUpperCase("");
    EXPECT_EQ(result, "");
}

/**
 * @brief Test toLowerCase functionality
 * @details Verifies string is converted to lowercase correctly
 */
TEST_F(StringToolkitTest, ToLowerCase_Basic)
{
    const auto result = StringToolkit::toLowerCase("HELLO WORLD");
    EXPECT_EQ(result, "hello world");
}

/**
 * @brief Test toLowerCase on an already-lowercase string
 * @details Verifies idempotency â€?lowercase input yields same output
 */
TEST_F(StringToolkitTest, ToLowerCase_AlreadyLower)
{
    const auto result = StringToolkit::toLowerCase("hello");
    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test toLowerCase with mixed case input
 * @details Verifies all characters are converted to lowercase
 */
TEST_F(StringToolkitTest, ToLowerCase_MixedCase)
{
    const auto result = StringToolkit::toLowerCase("Hello World");
    EXPECT_EQ(result, "hello world");
}

/**
 * @brief Test toLowerCase on an empty string
 * @details Verifies empty string is returned unchanged
 */
TEST_F(StringToolkitTest, ToLowerCase_EmptyString)
{
    const auto result = StringToolkit::toLowerCase("");
    EXPECT_EQ(result, "");
}

/**
 * @brief Test trimLeft functionality
 * @details Verifies only leading whitespace is removed
 */
TEST_F(StringToolkitTest, TrimLeft_Basic)
{
    const auto result = StringToolkit::trimLeft("   hello world");
    EXPECT_EQ(result, "hello world");
}

/**
 * @brief Test trimLeft with no leading whitespace
 * @details Verifies the original string is returned unchanged
 */
TEST_F(StringToolkitTest, TrimLeft_NoLeadingWhitespace)
{
    const auto result = StringToolkit::trimLeft("hello   ");
    EXPECT_EQ(result, "hello   ");
}

/**
 * @brief Test trimLeft on a string of all whitespace
 * @details Verifies empty string is returned after stripping all whitespace
 */
TEST_F(StringToolkitTest, TrimLeft_AllWhitespace)
{
    const auto result = StringToolkit::trimLeft("   ");
    EXPECT_EQ(result, "");
}

/**
 * @brief Test trimLeft on an empty string
 * @details Verifies empty string is returned unchanged
 */
TEST_F(StringToolkitTest, TrimLeft_EmptyString)
{
    const auto result = StringToolkit::trimLeft("");
    EXPECT_EQ(result, "");
}

/**
 * @brief Test trimRight functionality
 * @details Verifies only trailing whitespace is removed
 */
TEST_F(StringToolkitTest, TrimRight_Basic)
{
    const auto result = StringToolkit::trimRight("hello world   ");
    EXPECT_EQ(result, "hello world");
}

/**
 * @brief Test trimRight with no trailing whitespace
 * @details Verifies the original string is returned unchanged
 */
TEST_F(StringToolkitTest, TrimRight_NoTrailingWhitespace)
{
    const auto result = StringToolkit::trimRight("   hello");
    EXPECT_EQ(result, "   hello");
}

/**
 * @brief Test trimRight on a string of all whitespace
 * @details Verifies empty string is returned after stripping all whitespace
 */
TEST_F(StringToolkitTest, TrimRight_AllWhitespace)
{
    const auto result = StringToolkit::trimRight("   ");
    EXPECT_EQ(result, "");
}

/**
 * @brief Test trimRight on an empty string
 * @details Verifies empty string is returned unchanged
 */
TEST_F(StringToolkitTest, TrimRight_EmptyString)
{
    const auto result = StringToolkit::trimRight("");
    EXPECT_EQ(result, "");
}

/**
 * @brief Test trimRight on a single non-space character
 * @details Verifies unchanged string is returned for single char with no trailing space
 */
TEST_F(StringToolkitTest, TrimRight_SingleNonSpace)
{
    const auto result = StringToolkit::trimRight("a");
    EXPECT_EQ(result, "a");
}

/**
 * @brief Test trimRight on a single space character
 * @details Verifies single space is trimmed to empty string
 */
TEST_F(StringToolkitTest, TrimRight_SingleSpace)
{
    const auto result = StringToolkit::trimRight(" ");
    EXPECT_EQ(result, "");
}

/**
 * @brief Test trimRight with mixed content and trailing spaces
 * @details Verifies trailing whitespace is removed from content with letters and spaces
 */
TEST_F(StringToolkitTest, TrimRight_MixedContent)
{
    const auto result = StringToolkit::trimRight("a b c   ");
    EXPECT_EQ(result, "a b c");
}

/**
 * @brief Test contains functionality
 * @details Verifies substring detection works correctly
 */
TEST_F(StringToolkitTest, Contains_Found)
{
    EXPECT_TRUE(StringToolkit::contains("hello world", "world"));
}

/**
 * @brief Test contains when substring is not present
 * @details Verifies false is returned for non-existent substring
 */
TEST_F(StringToolkitTest, Contains_NotFound)
{
    EXPECT_FALSE(StringToolkit::contains("hello world", "xyz"));
}

/**
 * @brief Test contains with an empty substring
 * @details Verifies true is returned for empty substring (vacuously true)
 */
TEST_F(StringToolkitTest, Contains_EmptySubstring)
{
    EXPECT_TRUE(StringToolkit::contains("hello", ""));
}

/**
 * @brief Test contains on an empty source string
 * @details Verifies false is returned when searching in an empty string
 */
TEST_F(StringToolkitTest, Contains_EmptyString)
{
    EXPECT_FALSE(StringToolkit::contains("", "hello"));
}

/**
 * @brief Test indexOf functionality
 * @details Verifies first occurrence position is found correctly
 */
TEST_F(StringToolkitTest, IndexOf_Found)
{
    const auto result = StringToolkit::indexOf("hello world", "world");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 6);
}

/**
 * @brief Test indexOf when substring is not found
 * @details Verifies std::nullopt is returned for non-existent substring
 */
TEST_F(StringToolkitTest, IndexOf_NotFound)
{
    const auto result = StringToolkit::indexOf("hello world", "xyz");
    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Test indexOf with a custom start position
 * @details Verifies search begins from the specified offset
 */
TEST_F(StringToolkitTest, IndexOf_WithStartPos)
{
    const auto result = StringToolkit::indexOf("aaa", "a", 1);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 1);
}

/**
 * @brief Test indexOf with a start position beyond string length
 * @details Verifies std::invalid_argument is thrown for out-of-range start position
 */
TEST_F(StringToolkitTest, IndexOf_InvalidStartPos)
{
    EXPECT_THROW((void)StringToolkit::indexOf("hello", "e", 10), std::invalid_argument);
}

/**
 * @brief Test lastIndexOf functionality
 * @details Verifies last occurrence position is found correctly
 */
TEST_F(StringToolkitTest, LastIndexOf_Found)
{
    const auto result = StringToolkit::lastIndexOf("hello world hello", "hello");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 12);
}

/**
 * @brief Test lastIndexOf when substring is not found
 * @details Verifies std::nullopt is returned for non-existent substring
 */
TEST_F(StringToolkitTest, LastIndexOf_NotFound)
{
    const auto result = StringToolkit::lastIndexOf("hello world", "xyz");
    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Test lastIndexOf with a single occurrence
 * @details Verifies it finds the same position as indexOf for single occurrence
 */
TEST_F(StringToolkitTest, LastIndexOf_SingleOccurrence)
{
    const auto result = StringToolkit::lastIndexOf("hello", "ello");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 1);
}

/**
 * @brief Test substring functionality
 * @details Verifies substring extraction works correctly
 */
TEST_F(StringToolkitTest, Substring_Basic)
{
    const auto result = StringToolkit::substring("hello world", 6, 5);
    EXPECT_EQ(result, "world");
}

/**
 * @brief Test substring when count exceeds remaining length
 * @details Verifies substring is returned from start position to end of string
 */
TEST_F(StringToolkitTest, Substring_ToEnd)
{
    const auto result = StringToolkit::substring("hello world", 6, 100);
    EXPECT_EQ(result, "world");
}

/**
 * @brief Test substring from the beginning of the string
 * @details Verifies substring starting at index 0 returns the full string
 */
TEST_F(StringToolkitTest, Substring_FromStart)
{
    const auto result = StringToolkit::substring("hello", 0, 5);
    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test substring with a start position beyond string length
 * @details Verifies std::invalid_argument is thrown for invalid start position
 */
TEST_F(StringToolkitTest, Substring_InvalidStartPos)
{
    EXPECT_THROW((void)StringToolkit::substring("hello", 10, 2), std::invalid_argument);
}

/**
 * @brief Test left functionality
 * @details Verifies leftmost characters extraction
 */
TEST_F(StringToolkitTest, Left_Basic)
{
    const auto result = StringToolkit::left("hello world", 5);
    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test left when count exceeds string length
 * @details Verifies the entire string is returned when count > length
 */
TEST_F(StringToolkitTest, Left_ExceedsLength)
{
    const auto result = StringToolkit::left("hello", 10);
    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test left with zero count
 * @details Verifies empty string is returned when count is zero
 */
TEST_F(StringToolkitTest, Left_ZeroCount)
{
    const auto result = StringToolkit::left("hello", 0);
    EXPECT_EQ(result, "");
}

/**
 * @brief Test left on an empty string
 * @details Verifies empty string is returned regardless of count
 */
TEST_F(StringToolkitTest, Left_EmptyString)
{
    const auto result = StringToolkit::left("", 5);
    EXPECT_EQ(result, "");
}

/**
 * @brief Test right functionality
 * @details Verifies rightmost characters extraction
 */
TEST_F(StringToolkitTest, Right_Basic)
{
    const auto result = StringToolkit::right("hello world", 5);
    EXPECT_EQ(result, "world");
}

/**
 * @brief Test right when count exceeds string length
 * @details Verifies the entire string is returned when count > length
 */
TEST_F(StringToolkitTest, Right_ExceedsLength)
{
    const auto result = StringToolkit::right("hello", 10);
    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test right with zero count
 * @details Verifies empty string is returned when count is zero
 */
TEST_F(StringToolkitTest, Right_ZeroCount)
{
    const auto result = StringToolkit::right("hello", 0);
    EXPECT_EQ(result, "");
}

/**
 * @brief Test right on an empty string
 * @details Verifies empty string is returned regardless of count
 */
TEST_F(StringToolkitTest, Right_EmptyString)
{
    const auto result = StringToolkit::right("", 5);
    EXPECT_EQ(result, "");
}

/**
 * @brief Test padLeft functionality
 * @details Verifies left padding works correctly
 */
TEST_F(StringToolkitTest, PadLeft_Basic)
{
    const auto result = StringToolkit::padLeft("42", 5, '0');
    EXPECT_EQ(result, "00042");
}

/**
 * @brief Test padLeft when the string is longer than target length
 * @details Verifies original string is returned unchanged when no padding is needed
 */
TEST_F(StringToolkitTest, PadLeft_NoPaddingNeeded)
{
    const auto result = StringToolkit::padLeft("hello", 3, ' ');
    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test padLeft with default padding character (space)
 * @details Verifies left padding uses space when no char is specified
 */
TEST_F(StringToolkitTest, PadLeft_DefaultChar)
{
    const auto result = StringToolkit::padLeft("test", 6);
    EXPECT_EQ(result, "  test");
}

/**
 * @brief Test padRight functionality
 * @details Verifies right padding works correctly
 */
TEST_F(StringToolkitTest, PadRight_Basic)
{
    const auto result = StringToolkit::padRight("42", 5, '0');
    EXPECT_EQ(result, "42000");
}

/**
 * @brief Test padRight when the string is longer than target length
 * @details Verifies original string is returned unchanged when no padding is needed
 */
TEST_F(StringToolkitTest, PadRight_NoPaddingNeeded)
{
    const auto result = StringToolkit::padRight("hello", 3, ' ');
    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test padRight with default padding character (space)
 * @details Verifies right padding uses space when no char is specified
 */
TEST_F(StringToolkitTest, PadRight_DefaultChar)
{
    const auto result = StringToolkit::padRight("test", 6);
    EXPECT_EQ(result, "test  ");
}

/**
 * @brief Test repeat functionality
 * @details Verifies string repetition works correctly
 */
TEST_F(StringToolkitTest, Repeat_Basic)
{
    const auto result = StringToolkit::repeat("abc", 3);
    EXPECT_EQ(result, "abcabcabc");
}

/**
 * @brief Test repeat with zero count
 * @details Verifies empty string is returned when count is zero
 */
TEST_F(StringToolkitTest, Repeat_ZeroTimes)
{
    const auto result = StringToolkit::repeat("hello", 0);
    EXPECT_EQ(result, "");
}

/**
 * @brief Test repeat on an empty string
 * @details Verifies empty string is returned regardless of count
 */
TEST_F(StringToolkitTest, Repeat_EmptyString)
{
    const auto result = StringToolkit::repeat("", 5);
    EXPECT_EQ(result, "");
}

/**
 * @brief Test repeat with count of one
 * @details Verifies the original string is returned unchanged when count is one
 */
TEST_F(StringToolkitTest, Repeat_Once)
{
    const auto result = StringToolkit::repeat("hello", 1);
    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test reverse functionality
 * @details Verifies string reversal works correctly
 */
TEST_F(StringToolkitTest, Reverse_Basic)
{
    const auto result = StringToolkit::reverse("hello");
    EXPECT_EQ(result, "olleh");
}

/**
 * @brief Test reverse on a palindrome string
 * @details Verifies palindrome remains unchanged after reversal
 */
TEST_F(StringToolkitTest, Reverse_Palindrome)
{
    const auto result = StringToolkit::reverse("aba");
    EXPECT_EQ(result, "aba");
}

/**
 * @brief Test reverse on an empty string
 * @details Verifies empty string is returned unchanged
 */
TEST_F(StringToolkitTest, Reverse_EmptyString)
{
    const auto result = StringToolkit::reverse("");
    EXPECT_EQ(result, "");
}

/**
 * @brief Test reverse on a single character string
 * @details Verifies single character remains unchanged after reversal
 */
TEST_F(StringToolkitTest, Reverse_SingleChar)
{
    const auto result = StringToolkit::reverse("a");
    EXPECT_EQ(result, "a");
}

/**
 * @brief Test isEmpty functionality
 * @details Verifies empty string detection
 */
TEST_F(StringToolkitTest, IsEmpty_True)
{
    EXPECT_TRUE(StringToolkit::isEmpty(""));
}

/**
 * @brief Test isEmpty on a non-empty string
 * @details Verifies false is returned for non-empty string
 */
TEST_F(StringToolkitTest, IsEmpty_False)
{
    EXPECT_FALSE(StringToolkit::isEmpty("hello"));
}

/**
 * @brief Test isEmpty on a whitespace-only string
 * @details Verifies false is returned since whitespace is not empty
 */
TEST_F(StringToolkitTest, IsEmpty_Whitespace)
{
    EXPECT_FALSE(StringToolkit::isEmpty("   "));
}

/**
 * @brief Test isBlank functionality
 * @details Verifies blank string detection (empty or all whitespace)
 */
TEST_F(StringToolkitTest, IsBlank_Empty)
{
    EXPECT_TRUE(StringToolkit::isBlank(""));
}

/**
 * @brief Test isBlank on a whitespace-only string
 * @details Verifies true is returned for whitespace-only strings
 */
TEST_F(StringToolkitTest, IsBlank_AllWhitespace)
{
    EXPECT_TRUE(StringToolkit::isBlank("   "));
}

/**
 * @brief Test isBlank on a non-blank string
 * @details Verifies false is returned for strings with non-whitespace characters
 */
TEST_F(StringToolkitTest, IsBlank_NotBlank)
{
    EXPECT_FALSE(StringToolkit::isBlank("hello"));
}

/**
 * @brief Test isBlank on a string with leading and trailing whitespace
 * @details Verifies false is returned when non-whitespace content is present
 */
TEST_F(StringToolkitTest, IsBlank_MixedWhitespace)
{
    EXPECT_FALSE(StringToolkit::isBlank("  hello  "));
}

/**
 * @brief Test isNumeric functionality
 * @details Verifies numeric string detection
 */
TEST_F(StringToolkitTest, IsNumeric_True)
{
    EXPECT_TRUE(StringToolkit::isNumeric("12345"));
}

/**
 * @brief Test isNumeric on a non-numeric string
 * @details Verifies false is returned for strings containing non-digit characters
 */
TEST_F(StringToolkitTest, IsNumeric_False)
{
    EXPECT_FALSE(StringToolkit::isNumeric("123a5"));
}

/**
 * @brief Test isNumeric on an empty string
 * @details Verifies false is returned for empty strings
 */
TEST_F(StringToolkitTest, IsNumeric_Empty)
{
    EXPECT_FALSE(StringToolkit::isNumeric(""));
}

/**
 * @brief Test isNumeric on a single digit character
 * @details Verifies true is returned for a single numeric digit
 */
TEST_F(StringToolkitTest, IsNumeric_SingleDigit)
{
    EXPECT_TRUE(StringToolkit::isNumeric("0"));
}

/**
 * @brief Test isAlpha functionality
 * @details Verifies alphabetic string detection
 */
TEST_F(StringToolkitTest, IsAlpha_True)
{
    EXPECT_TRUE(StringToolkit::isAlpha("hello"));
}

/**
 * @brief Test isAlpha on a string containing digits
 * @details Verifies false is returned for strings with non-alphabetic characters
 */
TEST_F(StringToolkitTest, IsAlpha_False)
{
    EXPECT_FALSE(StringToolkit::isAlpha("hello123"));
}

/**
 * @brief Test isAlpha on an empty string
 * @details Verifies false is returned for empty strings
 */
TEST_F(StringToolkitTest, IsAlpha_Empty)
{
    EXPECT_FALSE(StringToolkit::isAlpha(""));
}

/**
 * @brief Test isAlpha on uppercase letters
 * @details Verifies true is returned for uppercase alphabetic strings
 */
TEST_F(StringToolkitTest, IsAlpha_UpperCase)
{
    EXPECT_TRUE(StringToolkit::isAlpha("HELLO"));
}

/**
 * @brief Test isAlphanumeric functionality
 * @details Verifies alphanumeric string detection
 */
TEST_F(StringToolkitTest, IsAlphanumeric_True)
{
    EXPECT_TRUE(StringToolkit::isAlphanumeric("hello123"));
}

/**
 * @brief Test isAlphanumeric on a string with special characters
 * @details Verifies false is returned for strings containing non-alphanumeric chars
 */
TEST_F(StringToolkitTest, IsAlphanumeric_False)
{
    EXPECT_FALSE(StringToolkit::isAlphanumeric("hello@123"));
}

/**
 * @brief Test isAlphanumeric on an empty string
 * @details Verifies false is returned for empty strings
 */
TEST_F(StringToolkitTest, IsAlphanumeric_Empty)
{
    EXPECT_FALSE(StringToolkit::isAlphanumeric(""));
}

/**
 * @brief Test isAlphanumeric on a string with only letters
 * @details Verifies true is returned for purely alphabetic strings
 */
TEST_F(StringToolkitTest, IsAlphanumeric_OnlyLetters)
{
    EXPECT_TRUE(StringToolkit::isAlphanumeric("hello"));
}

/**
 * @brief Test isAlphanumeric on a string with only digits
 * @details Verifies true is returned for purely numeric strings
 */
TEST_F(StringToolkitTest, IsAlphanumeric_OnlyDigits)
{
    EXPECT_TRUE(StringToolkit::isAlphanumeric("123"));
}

/**
 * @brief Test splitOnce functionality
 * @details Verifies splitting at first delimiter occurrence
 */
TEST_F(StringToolkitTest, SplitOnce_Found)
{
    const auto result = StringToolkit::splitOnce("key=value=extra", "=");
    EXPECT_EQ(result.first, "key");
    EXPECT_EQ(result.second, "value=extra");
}

/**
 * @brief Test splitOnce when delimiter is not present
 * @details Verifies the entire string is returned as first, second is empty
 */
TEST_F(StringToolkitTest, SplitOnce_NotFound)
{
    const auto result = StringToolkit::splitOnce("hello", "=");
    EXPECT_EQ(result.first, "hello");
    EXPECT_EQ(result.second, "");
}

/**
 * @brief Test splitOnce when delimiter is at the start
 * @details Verifies first element is empty when string starts with delimiter
 */
TEST_F(StringToolkitTest, SplitOnce_AtBeginning)
{
    const auto result = StringToolkit::splitOnce("=value", "=");
    EXPECT_EQ(result.first, "");
    EXPECT_EQ(result.second, "value");
}

/**
 * @brief Test splitOnce with an empty delimiter
 * @details Verifies std::invalid_argument is thrown for empty delimiter
 */
TEST_F(StringToolkitTest, SplitOnce_EmptyDelimiter)
{
    EXPECT_THROW((void)StringToolkit::splitOnce("hello", ""), std::invalid_argument);
}

/**
 * @brief Test remove functionality
 * @details Verifies substring removal works correctly
 */
TEST_F(StringToolkitTest, Remove_Basic)
{
    const auto result = StringToolkit::remove("hello world hello", "hello ");
    EXPECT_EQ(result, "world hello");
}

/**
 * @brief Test remove when substring is not found
 * @details Verifies the original string is returned unchanged
 */
TEST_F(StringToolkitTest, Remove_NoOccurrences)
{
    const auto result = StringToolkit::remove("hello world", "xyz");
    EXPECT_EQ(result, "hello world");
}

/**
 * @brief Test remove when all characters match the substring
 * @details Verifies empty string is returned when entire string is removed
 */
TEST_F(StringToolkitTest, Remove_AllOccurrences)
{
    const auto result = StringToolkit::remove("aaa", "a");
    EXPECT_EQ(result, "");
}

/**
 * @brief Test remove with an empty substring to remove
 * @details Verifies std::invalid_argument is thrown for empty search string
 */
TEST_F(StringToolkitTest, Remove_EmptyToRemove)
{
    EXPECT_THROW((void)StringToolkit::remove("hello", ""), std::invalid_argument);
}

/**
 * @brief Test removeChars functionality
 * @details Verifies character removal works correctly
 */
TEST_F(StringToolkitTest, RemoveChars_Basic)
{
    const auto result = StringToolkit::removeChars("hello world", "aeiou");
    EXPECT_EQ(result, "hll wrld");
}

/**
 * @brief Test removeChars when no characters match
 * @details Verifies the original string is returned unchanged
 */
TEST_F(StringToolkitTest, RemoveChars_NoMatches)
{
    const auto result = StringToolkit::removeChars("hello", "xyz");
    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test removeChars when all characters are removed
 * @details Verifies empty string is returned when all chars match the removal set
 */
TEST_F(StringToolkitTest, RemoveChars_RemoveAll)
{
    const auto result = StringToolkit::removeChars("aaa", "a");
    EXPECT_EQ(result, "");
}

/**
 * @brief Test removeChars with an empty character set
 * @details Verifies std::invalid_argument is thrown for empty chars to remove
 */
TEST_F(StringToolkitTest, RemoveChars_EmptyCharsToRemove)
{
    EXPECT_THROW((void)StringToolkit::removeChars("hello", ""), std::invalid_argument);
}

/**
 * @brief Test truncate functionality
 * @details Verifies string truncation with suffix works correctly
 */
TEST_F(StringToolkitTest, Truncate_Basic)
{
    const auto result = StringToolkit::truncate("hello world", 8, "...");
    EXPECT_EQ(result, "hello...");
}

/**
 * @brief Test truncate when string is shorter than max length
 * @details Verifies the original string is returned unchanged when no truncation is needed
 */
TEST_F(StringToolkitTest, Truncate_NoTruncationNeeded)
{
    const auto result = StringToolkit::truncate("hello", 10, "...");
    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test truncate with an empty suffix
 * @details Verifies truncation works correctly when no suffix is appended
 */
TEST_F(StringToolkitTest, Truncate_EmptySuffix)
{
    const auto result = StringToolkit::truncate("hello world", 5, "");
    EXPECT_EQ(result, "hello");
}

/**
 * @brief Test truncate with zero max length
 * @details Verifies empty string is returned when max length is zero
 */
TEST_F(StringToolkitTest, Truncate_ZeroMaxLength)
{
    const auto result = StringToolkit::truncate("hello", 0, "...");
    EXPECT_EQ(result, "");
}

/**
 * @brief Test truncate when suffix exceeds max length
 * @details Verifies std::invalid_argument is thrown when suffix alone exceeds max length
 */
TEST_F(StringToolkitTest, Truncate_SuffixTooLong)
{
    EXPECT_THROW((void)StringToolkit::truncate("hello", 2, "..."), std::invalid_argument);
}
