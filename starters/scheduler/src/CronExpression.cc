/**
 * @file CronExpression.cc
 * @brief Cron expression parser implementation
 */

#include <cppforge/starter/scheduler/CronExpression.hpp>
#include <ctime>
#include <stdexcept>
#include <sstream>
#include <algorithm>

namespace cppforge::starter::scheduler
{
    CronExpression::CronExpression(const std::string& expression) : expression_(expression)
    {
        std::istringstream iss(expression);
        std::string token;
        std::vector<std::string> tokens;

        while (iss >> token)
        {
            tokens.push_back(token);
        }

        if (tokens.size() != 5)
        {
            throw std::invalid_argument("Cron expression must have exactly 5 fields, got " +
                                        std::to_string(tokens.size()) + " in: " + expression);
        }

        minutes_ = parseField(tokens[0], 0, 59);
        hours_ = parseField(tokens[1], 0, 23);
        days_ = parseField(tokens[2], 1, 31);
        months_ = parseField(tokens[3], 1, 12);
        weekdays_ = parseField(tokens[4], 0, 6);
    }

    CronExpression::CronField CronExpression::parseField(const std::string& field_str, int min, int max)
    {
        CronField field;

        if (field_str == "*")
        {
            for (int i = min; i <= max; ++i)
            {
                field.values.push_back(i);
            }
            return field;
        }

        // Handle step values: */n or range/n
        std::string base = field_str;
        int step = 1;
        auto slash_pos = field_str.find('/');
        if (slash_pos != std::string::npos)
        {
            base = field_str.substr(0, slash_pos);
            step = std::stoi(field_str.substr(slash_pos + 1));
            if (step <= 0)
            {
                throw std::invalid_argument("Step value must be positive in: " + field_str);
            }
        }

        // Handle range: a-b
        auto dash_pos = base.find('-');
        if (dash_pos != std::string::npos)
        {
            int range_start = std::stoi(base.substr(0, dash_pos));
            int range_end = std::stoi(base.substr(dash_pos + 1));

            if (range_start < min || range_end > max || range_start > range_end)
            {
                throw std::invalid_argument("Invalid range in cron field: " + field_str);
            }

            for (int i = range_start; i <= range_end; i += step)
            {
                field.values.push_back(i);
            }
        }
        else if (base == "*")
        {
            // */n case
            for (int i = min; i <= max; i += step)
            {
                field.values.push_back(i);
            }
        }
        else
        {
            // Single value
            int val = std::stoi(base);
            if (val < min || val > max)
            {
                throw std::invalid_argument("Value " + std::to_string(val) + " out of range [" +
                                            std::to_string(min) + "," + std::to_string(max) + "] in: " + field_str);
            }
            if (step > 1)
            {
                // value/step means starting from value, step through range
                for (int i = val; i <= max; i += step)
                {
                    field.values.push_back(i);
                }
            }
            else
            {
                field.values.push_back(val);
            }
        }

        return field;
    }

    std::chrono::system_clock::time_point CronExpression::nextExecutionTime(
        std::chrono::system_clock::time_point from) const
    {
        // Start from the next minute (zero out seconds)
        auto time_t_from = std::chrono::system_clock::to_time_t(from);
        std::tm tm_buf{};
#ifdef _WIN32
        localtime_s(&tm_buf, &time_t_from);
#else
        localtime_r(&time_t_from, &tm_buf);
#endif

        tm_buf.tm_sec = 0;
        tm_buf.tm_min += 1; // Move to next minute

        // Normalize
        auto normalized = std::mktime(&tm_buf);
        time_t_from = normalized;

        // Search for the next matching time (limit to 4 years to avoid infinite loop)
        const int max_iterations = 366 * 24 * 60 * 4; // ~4 years of minutes
        for (int i = 0; i < max_iterations; ++i)
        {
#ifdef _WIN32
            localtime_s(&tm_buf, &time_t_from);
#else
            localtime_r(&time_t_from, &tm_buf);
#endif

            auto tp = std::chrono::system_clock::from_time_t(time_t_from);
            if (matches(tp))
            {
                return tp;
            }

            time_t_from += 60; // Advance by 1 minute
        }

        throw std::runtime_error("Could not find next execution time within 4 years for: " + expression_);
    }

    bool CronExpression::matches(std::chrono::system_clock::time_point tp) const
    {
        auto time_t_tp = std::chrono::system_clock::to_time_t(tp);
        std::tm tm_buf{};
#ifdef _WIN32
        localtime_s(&tm_buf, &time_t_tp);
#else
        localtime_r(&time_t_tp, &tm_buf);
#endif

        auto contains = [](const CronField& field, int value) {
            return std::find(field.values.begin(), field.values.end(), value) != field.values.end();
        };

        return contains(minutes_, tm_buf.tm_min) &&
               contains(hours_, tm_buf.tm_hour) &&
               contains(days_, tm_buf.tm_mday) &&
               contains(months_, tm_buf.tm_mon + 1) &&
               contains(weekdays_, tm_buf.tm_wday);
    }

    const std::string& CronExpression::expression() const
    {
        return expression_;
    }
}
