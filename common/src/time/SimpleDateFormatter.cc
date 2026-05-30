/**
 * @file SimpleDateFormatter.cc
 * @brief SimpleDateFormatter class implementation
 * @details This file contains the implementation of the SimpleDateFormatter class methods for Time and date utilities including profilers and formatters.
 */

#include "src/time/SimpleDateFormatter.hpp"

#include <fmt/format.h>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <stdexcept>
#include <ctime>
#include <glog/logging.h>

namespace common::time
{
    SimpleDateFormatter::SimpleDateFormatter(const std::string& pattern)
    {
        validatePattern(pattern);
        pattern_ = pattern;
    }

    void SimpleDateFormatter::applyPattern(const std::string& newPattern)
    {
        validatePattern(newPattern);
        pattern_ = newPattern;
    }

    std::string SimpleDateFormatter::toPattern() const
    {
        return pattern_;
    }

    std::string SimpleDateFormatter::format(const std::tm& date) const
    {
        std::ostringstream oss;
        try
        {
            oss << std::put_time(&date, pattern_.c_str());
            if (oss.fail())
            {
                throw std::runtime_error("SimpleDateFormatter::format: Failed to format date with pattern '" + pattern_ + "'");
            }
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("SimpleDateFormatter::format: Error formatting date - " + std::string(e.what()));
        }
        const auto result = oss.str();
        return result;
    }

    std::string SimpleDateFormatter::format(const std::chrono::system_clock::time_point& tp) const
    {
        std::time_t time = std::chrono::system_clock::to_time_t(tp);
        std::tm tm_struct = {};
#ifdef _WIN32
        if (localtime_s(&tm_struct, &time) != 0)
        {
            throw std::runtime_error("SimpleDateFormatter::format: Failed to convert time_point to local time");
        }
#else
        if (localtime_r(&time, &tm_struct) == nullptr)
        {
            throw std::runtime_error("SimpleDateFormatter::format: Failed to convert time_point to local time");
        }
#endif

        return format(tm_struct);
    }

    std::tm SimpleDateFormatter::parse(const std::string& dateStr) const
    {
        std::tm date = {};
        std::istringstream iss(dateStr);
        iss >> std::get_time(&date, pattern_.c_str());

        if (iss.fail())
        {
            throw std::runtime_error("SimpleDateFormatter::parse: Failed to parse date string '" + dateStr + "' with pattern '" + pattern_ + "'");
        }

        // Check if the entire string was consumed (for strict parsing)
        iss >> std::ws; // Skip any remaining whitespace
        if (!iss.eof())
        {
            throw std::runtime_error("SimpleDateFormatter::parse: Extra characters after parsing date string '" + dateStr + "'");
        }
        return date;
    }

    bool SimpleDateFormatter::equals(const SimpleDateFormatter& other) const noexcept
    {
        return pattern_ == other.pattern_;
    }

    size_t SimpleDateFormatter::hashCode() const noexcept
    {
        return std::hash<std::string>{}(pattern_);
    }

    void SimpleDateFormatter::validatePattern(const std::string& pat)
    {
        if (pat.empty())
        {
            DLOG(WARNING) << "SimpleDateFormatter validatePattern: Pattern is empty";
            throw std::invalid_argument("SimpleDateFormatter::validatePattern: Pattern cannot be empty");
        }
    }
}
