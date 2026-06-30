/**
 * @file CronExpression.hpp
 * @brief Cron expression parser and next-execution-time calculator
 * @details Supports standard 5-field cron expressions:
 *          minute hour day-of-month month day-of-week
 *          Fields support: * (any), specific values, ranges (1-5), steps (*/5).
 */

#pragma once
#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace cppforge::starter::scheduler
{
    /// @brief Parses and evaluates cron expressions
    /// @details Supports 5-field cron: minute hour day-of-month month day-of-week.
    ///          Field syntax: * | value | range (a-b) | step (*/n or a-b/n).
    class CronExpression
    {
    public:
        /// @brief Construct a CronExpression from a cron string
        /// @param expression Cron expression string (e.g. "*/5 * * * *")
        /// @throws std::invalid_argument if the expression is malformed
        explicit CronExpression(const std::string& expression);

        /// @brief Calculate the next execution time after the given time point
        /// @param from The reference time point
        /// @return The next time point matching the cron expression
        [[nodiscard]] std::chrono::system_clock::time_point nextExecutionTime(
            std::chrono::system_clock::time_point from) const;

        /// @brief Get the original cron expression string
        /// @return The cron expression string
        [[nodiscard]] const std::string& expression() const;

    private:
        /// @brief Represents the set of allowed values for a single cron field
        struct CronField
        {
            std::vector<int> values;
        };

        std::string expression_;
        CronField minutes_;    // 0-59
        CronField hours_;      // 0-23
        CronField days_;       // 1-31
        CronField months_;     // 1-12
        CronField weekdays_;   // 0-6 (0 = Sunday)

        /// @brief Parse a single cron field into a set of allowed values
        /// @param field_str The field string (e.g. "*/5", "1-3", "7", "*")
        /// @param min Minimum allowed value
        /// @param max Maximum allowed value
        /// @return Parsed CronField
        [[nodiscard]] static CronField parseField(const std::string& field_str, int min, int max);

        /// @brief Check if a time point matches all cron fields
        /// @param tp The time point to check
        /// @return true if the time matches the cron expression
        [[nodiscard]] bool matches(std::chrono::system_clock::time_point tp) const;
    };
}
