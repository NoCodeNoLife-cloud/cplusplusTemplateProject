#pragma once
#include <string>
#include <vector>

namespace common::toolkit {
    /// @brief A utility class for string manipulation operations.
    /// This class provides static methods for common string operations such as splitting and concatenating strings.
    class StringToolkit {
    public:
        StringToolkit() = delete;

        /// @brief Converts all characters in a string to uppercase.
        /// @param str The string to convert.
        /// @return A new string with all characters converted to uppercase.
        [[nodiscard]] static auto toUpperCase(const std::string &str) -> std::string;

        /// @brief Converts all characters in a string to lowercase.
        /// @param str The string to convert.
        /// @return A new string with all characters converted to lowercase.
        [[nodiscard]] static auto toLowerCase(const std::string &str) -> std::string;

        /// @brief Splits a string into a vector of strings based on a specified character.
        /// @param target The string to be split.
        /// @param split_char The character used as the delimiter for splitting.
        /// @return A vector of strings resulting from the split operation.
        [[nodiscard]] static auto split(const std::string &target, char split_char) -> std::vector<std::string>;

        /// @brief Splits a string into a vector of strings based on a specified character, including empty strings for consecutive delimiters.
        /// @param target The string to be split.
        /// @param split_char The character used as the delimiter for splitting.
        /// @return A vector of strings resulting from the split operation, including empty strings.
        [[nodiscard]] static auto splitWithEmpty(const std::string &target, char split_char) -> std::vector<std::string>;

        /// @brief Splits a string into a vector of strings based on a specified substring.
        /// @param target The string to be split.
        /// @param delimiter The substring used as the delimiter for splitting.
        /// @return A vector of strings resulting from the split operation.
        [[nodiscard]] static auto split(const std::string &target, const std::string &delimiter) -> std::vector<std::string>;

        /// @brief Concatenates a vector of strings into a single string with a specified character between each element.
        /// @param source The vector of strings to be concatenated.
        /// @param split_char The character to insert between each concatenated string.
        /// @return A single string composed of all elements in the vector separated by the specified character.
        [[nodiscard]] static auto concatenate(const std::vector<std::string> &source, char split_char) -> std::string;

        /// @brief Concatenates a vector of strings into a single string with a specified string between each element.
        /// @param source The vector of strings to be concatenated.
        /// @param delimiter The string to insert between each concatenated string.
        /// @return A single string composed of all elements in the vector separated by the specified string.
        [[nodiscard]] static auto concatenate(const std::vector<std::string> &source, const std::string &delimiter) -> std::string;

        /// @brief Checks if a string starts with a specified prefix.
        /// @param str The string to check.
        /// @param prefix The prefix to check for.
        /// @return true if the string starts with the prefix, false otherwise.
        [[nodiscard]] static auto startsWith(const std::string &str, const std::string &prefix) -> bool;

        /// @brief Checks if a string ends with a specified suffix.
        /// @param str The string to check.
        /// @param suffix The suffix to check for.
        /// @return true if the string ends with the suffix, false otherwise.
        [[nodiscard]] static auto endsWith(const std::string &str, const std::string &suffix) -> bool;

        /// @brief Trims whitespace from the beginning and end of a string.
        /// @param str The string to trim.
        /// @return A new string with whitespace removed from both ends.
        [[nodiscard]] static auto trim(const std::string &str) -> std::string;

        /// @brief Trims whitespace from the beginning of a string.
        /// @param str The string to trim.
        /// @return A new string with leading whitespace removed.
        [[nodiscard]] static auto trimLeft(const std::string &str) -> std::string;

        /// @brief Trims whitespace from the end of a string.
        /// @param str The string to trim.
        /// @return A new string with trailing whitespace removed.
        [[nodiscard]] static auto trimRight(const std::string &str) -> std::string;

        /// @brief Replaces all occurrences of a substring with another substring.
        /// @param str The string to perform replacements on.
        /// @param from The substring to replace.
        /// @param to The replacement substring.
        /// @return A new string with all occurrences replaced.
        [[nodiscard]] static auto replaceAll(const std::string &str, const std::string &from, const std::string &to) -> std::string;

        /// @brief Checks if a string contains a specified substring.
        /// @param str The string to search in.
        /// @param substr The substring to search for.
        /// @return true if the string contains the substring, false otherwise.
        [[nodiscard]] static auto contains(const std::string &str, const std::string &substr) -> bool;

        /// @brief Finds the first occurrence of a substring in a string.
        /// @param str The string to search in.
        /// @param substr The substring to search for.
        /// @param start The position to start searching from (default is 0).
        /// @return The position of the first occurrence, or std::string::npos if not found.
        [[nodiscard]] static auto indexOf(const std::string &str, const std::string &substr, size_t start = 0) -> size_t;

        /// @brief Finds the last occurrence of a substring in a string.
        /// @param str The string to search in.
        /// @param substr The substring to search for.
        /// @return The position of the last occurrence, or std::string::npos if not found.
        [[nodiscard]] static auto lastIndexOf(const std::string &str, const std::string &substr) -> size_t;

        /// @brief Extracts a substring from a string.
        /// @param str The source string.
        /// @param start The starting position of the substring.
        /// @param length The length of the substring to extract.
        /// @return The extracted substring.
        [[nodiscard]] static auto substring(const std::string &str, size_t start, size_t length) -> std::string;

        /// @brief Extracts the leftmost n characters from a string.
        /// @param str The source string.
        /// @param count The number of characters to extract from the left.
        /// @return The leftmost n characters, or the entire string if it's shorter than count.
        [[nodiscard]] static auto left(const std::string &str, size_t count) -> std::string;

        /// @brief Extracts the rightmost n characters from a string.
        /// @param str The source string.
        /// @param count The number of characters to extract from the right.
        /// @return The rightmost n characters, or the entire string if it's shorter than count.
        [[nodiscard]] static auto right(const std::string &str, size_t count) -> std::string;

        /// @brief Pads a string on the left with a specified character to reach a desired width.
        /// @param str The string to pad.
        /// @param width The desired total width after padding.
        /// @param padChar The character to use for padding (default is space).
        /// @return The padded string.
        [[nodiscard]] static auto padLeft(const std::string &str, size_t width, char padChar = ' ') -> std::string;

        /// @brief Pads a string on the right with a specified character to reach a desired width.
        /// @param str The string to pad.
        /// @param width The desired total width after padding.
        /// @param padChar The character to use for padding (default is space).
        /// @return The padded string.
        [[nodiscard]] static auto padRight(const std::string &str, size_t width, char padChar = ' ') -> std::string;

        /// @brief Repeats a string a specified number of times.
        /// @param str The string to repeat.
        /// @param count The number of times to repeat the string.
        /// @return A new string consisting of the original string repeated count times.
        [[nodiscard]] static auto repeat(const std::string &str, size_t count) -> std::string;

        /// @brief Reverses the characters in a string.
        /// @param str The string to reverse.
        /// @return A new string with characters in reverse order.
        [[nodiscard]] static auto reverse(const std::string &str) -> std::string;

        /// @brief Checks if a string is empty.
        /// @param str The string to check.
        /// @return true if the string is empty, false otherwise.
        [[nodiscard]] static auto isEmpty(const std::string &str) -> bool;

        /// @brief Checks if a string is blank (empty or contains only whitespace).
        /// @param str The string to check.
        /// @return true if the string is blank, false otherwise.
        [[nodiscard]] static auto isBlank(const std::string &str) -> bool;

        /// @brief Checks if a string contains only numeric characters.
        /// @param str The string to check.
        /// @return true if the string contains only digits, false otherwise.
        [[nodiscard]] static auto isNumeric(const std::string &str) -> bool;

        /// @brief Checks if a string contains only alphabetic characters.
        /// @param str The string to check.
        /// @return true if the string contains only letters, false otherwise.
        [[nodiscard]] static auto isAlpha(const std::string &str) -> bool;

        /// @brief Checks if a string contains only alphanumeric characters.
        /// @param str The string to check.
        /// @return true if the string contains only letters and digits, false otherwise.
        [[nodiscard]] static auto isAlphanumeric(const std::string &str) -> bool;

        /// @brief Splits a string into two parts at the first occurrence of a delimiter.
        /// @param target The string to split.
        /// @param delimiter The delimiter to split on.
        /// @return A pair containing the part before and after the first delimiter. If delimiter is not found, returns {target, ""}.
        [[nodiscard]] static auto splitOnce(const std::string &target, const std::string &delimiter) -> std::pair<std::string, std::string>;

        /// @brief Removes all occurrences of a substring from a string.
        /// @param str The string to remove from.
        /// @param toRemove The substring to remove.
        /// @return A new string with all occurrences of the substring removed.
        [[nodiscard]] static auto remove(const std::string &str, const std::string &toRemove) -> std::string;

        /// @brief Removes all occurrences of specified characters from a string.
        /// @param str The string to remove from.
        /// @param charsToRemove A string containing all characters to be removed.
        /// @return A new string with all specified characters removed.
        [[nodiscard]] static auto removeChars(const std::string &str, const std::string &charsToRemove) -> std::string;

        /// @brief Truncates a string to a maximum length with an optional suffix.
        /// @param str The string to truncate.
        /// @param maxLength The maximum length of the resulting string (including suffix).
        /// @param suffix The suffix to append if truncation occurs (default is "...").
        /// @return The truncated string with suffix if needed, or the original string if it's within the limit.
        [[nodiscard]] static auto truncate(const std::string &str, size_t maxLength, const std::string &suffix = "...") -> std::string;

        /// @brief Joins a vector of strings with a specified delimiter.
        /// @param parts The vector of strings to join.
        /// @param delimiter The string to insert between each part.
        /// @return A single joined string.
        [[nodiscard]] static auto join(const std::vector<std::string> &parts, const std::string &delimiter) -> std::string;
    };
}
