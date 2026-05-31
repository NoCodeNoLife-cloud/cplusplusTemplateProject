/**
 * @file SimpleDateFormatter.hpp
 * @brief SimpleDateFormatter class declaration
 * @details This header defines the SimpleDateFormatter class that provides functionality for Time and date utilities including profilers and formatters.
 */

#pragma once
#include <string>
#include <chrono>

namespace common::time
{
    /// @brief A utility class for formatting and parsing dates based on a pattern.
    /// The class provides methods to format `std::tm` objects into
    /// strings and parse strings back into `std::tm` objects using a specified pattern.
    /// It supports various date and time format specifiers similar to those used in
    /// `strftime` and `strptime` functions.
    class SimpleDateFormatter
    {
    public:
        /// @brief Construct a SimpleDateFormatter with the specified pattern
        /// @param pattern The pattern to use for formatting and parsing
        /// @throws std::invalid_argument If the pattern is empty
        explicit SimpleDateFormatter(const std::string& pattern);

        /// @brief Applies a new pattern to the date formatter.
        /// @param newPattern The new pattern to apply.
        /// @throws std::invalid_argument If the new pattern is empty
        void applyPattern(const std::string& newPattern);

        /// @brief Retrieves the current pattern used by the date formatter.
        /// @return The current pattern as a string.
        [[nodiscard]] std::string toPattern() const;

        /// @brief Formats a given date according to the current pattern.
        /// @param date The date to format.
        /// @return The formatted date as a string.
        /// @throws std::runtime_error If formatting fails
        [[nodiscard]] std::string format(const std::tm& date) const;

        /// @brief Formats a given chrono time_point according to the current pattern.
        /// @param tp The time_point to format.
        /// @return The formatted date as a string.
        /// @throws std::runtime_error If formatting fails
        [[nodiscard]] std::string format(const std::chrono::system_clock::time_point& tp) const;

        /// @brief Parses a date string according to the current pattern.
        /// @param dateStr The date string to parse.
        /// @return The parsed date as a `std::tm` structure.
        /// @throws std::runtime_error If parsing fails
        [[nodiscard]] std::tm parse(const std::string& dateStr) const;

        /// @brief Compares this formatter with another for equality.
        /// @param other The other formatter to compare with.
        /// @return True if both formatters are equal, false otherwise.
        [[nodiscard]] bool equals(const SimpleDateFormatter& other) const ;

        /// @brief Computes the hash code for this formatter.
        /// @return The hash code as a size_t.
        [[nodiscard]] size_t hashCode() const ;

    private:
        std::string pattern_{};

        /// @brief Validates a given pattern string.
        /// @param pat The pattern to validate.
        /// @throws std::invalid_argument If the pattern is empty
        static void validatePattern(const std::string& pat);
    };
}