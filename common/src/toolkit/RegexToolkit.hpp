/**
 * @file RegexToolkit.hpp
 * @brief RegexToolkit class declaration
 * @details This header defines the RegexToolkit class that provides functionality for General utility toolkits for strings, arrays, and other operations.
 */

#pragma once

#include <regex>
#include <string>
#include <vector>

namespace common::toolkit
{
    class RegexToolkit
    {
    public:
        /// @brief Checks if the entire string matches the regular expression
        /// @param text The string to check
        /// @param pattern The regular expression pattern
        /// @param flags Regular expression flags (default ECMAScript)
        /// @return true if the entire string matches the pattern, false otherwise
        /// @throws std::invalid_argument if the pattern is invalid
        [[nodiscard]] static auto is_match(const std::string& text, const std::string& pattern, std::regex_constants::syntax_option_type flags = std::regex_constants::ECMAScript) -> bool;

        /// @brief Checks if there is a match for the regular expression anywhere in the string
        /// @param text The string to search
        /// @param pattern The regular expression pattern
        /// @param flags Regular expression flags (default ECMAScript)
        /// @return true if a match is found, false otherwise
        /// @throws std::invalid_argument if the pattern is invalid
        [[nodiscard]] static auto is_search(const std::string& text, const std::string& pattern, std::regex_constants::syntax_option_type flags = std::regex_constants::ECMAScript) -> bool;

        /// @brief Extracts all matches of the regular expression in the string
        /// @param text The string to search
        /// @param pattern The regular expression pattern
        /// @param flags Regular expression flags (default ECMAScript)
        /// @return A vector containing all matched substrings
        /// @throws std::invalid_argument if the pattern is invalid
        [[nodiscard]] static auto get_matches(const std::string& text, const std::string& pattern, std::regex_constants::syntax_option_type flags = std::regex_constants::ECMAScript) -> std::vector<std::string>;

        /// @brief Extracts all matches and their subgroups
        /// @param text The string to search
        /// @param pattern The regular expression pattern
        /// @param flags Regular expression flags (default ECMAScript)
        /// @return A vector of vectors containing matches and their subgroups
        /// @throws std::invalid_argument if the pattern is invalid
        [[nodiscard]] static auto get_matches_with_groups(const std::string& text, const std::string& pattern, std::regex_constants::syntax_option_type flags = std::regex_constants::ECMAScript) -> std::vector<std::vector<std::string>>;

        /// @brief Replaces all matches of the regular expression with a replacement string
        /// @param text The string to process
        /// @param pattern The regular expression pattern
        /// @param replacement The replacement string
        /// @param flags Regular expression flags (default ECMAScript)
        /// @return The string with replacements made
        /// @throws std::invalid_argument if the pattern is invalid
        [[nodiscard]] static auto replace_all(const std::string& text, const std::string& pattern, const std::string& replacement, std::regex_constants::syntax_option_type flags = std::regex_constants::ECMAScript) -> std::string;

        /// @brief Splits a string by the regular expression delimiter
        /// @param text The string to split
        /// @param pattern The regular expression pattern for delimiters
        /// @param flags Regular expression flags (default ECMAScript)
        /// @return A vector containing the split parts
        /// @throws std::invalid_argument if the pattern is invalid
        [[nodiscard]] static auto split(const std::string& text, const std::string& pattern, std::regex_constants::syntax_option_type flags = std::regex_constants::ECMAScript) -> std::vector<std::string>;
    };
}
