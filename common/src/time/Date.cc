/**
 * @file Date.cc
 * @brief Date class implementation
 * @details This file contains the implementation of the Date class methods for Time and date utilities including profilers and formatters.
 */

#include "time/Date.hpp"

#include <chrono>
#include <ctime>
#include <functional>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <fmt/format.h>
#include <glog/logging.h>

namespace common::time
{
    namespace
    {
        // Helper function to check if a year is a leap year
        bool isLeapYear(const int32_t year)
        {
            return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
        }

        // Helper function to get days in a month
        int32_t getDaysInMonth(const int32_t year, const int32_t month)
        {
            static const int32_t daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
            int32_t days = daysInMonth[month];
            if (month == 2 && isLeapYear(year))
            {
                days = 29;
            }

            return days;
        }
    }

    Date::Date()  : time_point_(std::chrono::system_clock::now())
    {
    }

    Date::Date(const int32_t year, const int32_t month, const int32_t day)
    {
        if (!isValidDate(year, month, day))
        {
            DLOG(WARNING) << fmt::format("Date constructor: Invalid date - year={}, month={}, day={}", year, month, day);
            throw std::invalid_argument("Date::Date: Invalid date components.");
        }

        std::tm tm = {};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        tm.tm_isdst = -1; // Let mktime determine if DST is in effect
        const auto timeT = std::mktime(&tm);
        if (timeT == -1)
        {
            DLOG(WARNING) << fmt::format("Date constructor: mktime failed for date - year={}, month={}, day={}", year, month, day);
            throw std::invalid_argument("Date::Date: Invalid date components.");
        }
        time_point_ = std::chrono::system_clock::from_time_t(timeT);
    }

    Date::Date(const int32_t year, const int32_t month, const int32_t day, const int32_t hours, const int32_t minutes, const int32_t seconds)
    {
        if (!isValidDate(year, month, day, hours, minutes, seconds))
        {
            DLOG(WARNING) << fmt::format("Date constructor: Invalid datetime - year={}, month={}, day={}, hour={}, min={}, sec={}", year, month, day, hours, minutes, seconds);
            throw std::invalid_argument("Date::Date: Invalid date-time components.");
        }

        std::tm tm = {};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hours;
        tm.tm_min = minutes;
        tm.tm_sec = seconds;
        tm.tm_isdst = -1; // Let mktime determine if DST is in effect
        const auto timeT = std::mktime(&tm);
        if (timeT == -1)
        {
            throw std::invalid_argument("Date::Date: Invalid date-time components.");
        }
        time_point_ = std::chrono::system_clock::from_time_t(timeT);
    }

    Date::Date(const int64_t timestamp)  : time_point_(std::chrono::milliseconds(timestamp))
    {
    }

    Date Date::clone() const
    {
        return Date(getTime());
    }

    bool Date::equals(const Date& other) const
    {
        return time_point_ == other.time_point_;
    }

    bool Date::after(const Date& other) const
    {
        return time_point_ > other.time_point_;
    }

    bool Date::before(const Date& other) const
    {
        return time_point_ < other.time_point_;
    }

    int64_t Date::getTime() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(time_point_.time_since_epoch()).count();
    }

    int32_t Date::getYear() const
    {
        return toTm().tm_year + 1900;
    }

    int32_t Date::getMonth() const
    {
        return toTm().tm_mon + 1;
    }

    int32_t Date::getDay() const
    {
        return toTm().tm_mday;
    }

    int32_t Date::getHours() const
    {
        return toTm().tm_hour;
    }

    int32_t Date::getMinutes() const
    {
        return toTm().tm_min;
    }

    int32_t Date::getSeconds() const
    {
        return toTm().tm_sec;
    }

    std::string Date::toString() const
    {
        const auto tm = toTm();
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        const auto result = oss.str();
        return result;
    }

    size_t Date::hashCode() const
    {
        return std::hash<int64_t>{}(getTime());
    }

    // Comparison operators
    bool Date::operator==(const Date& other) const
    {
        return equals(other);
    }

    bool Date::operator!=(const Date& other) const
    {
        return !equals(other);
    }

    bool Date::operator<(const Date& other) const
    {
        return before(other);
    }

    bool Date::operator<=(const Date& other) const
    {
        return before(other) || equals(other);
    }

    bool Date::operator>(const Date& other) const
    {
        return after(other);
    }

    bool Date::operator>=(const Date& other) const
    {
        return after(other) || equals(other);
    }

    std::tm Date::toTm() const
    {
        const auto timeT = std::chrono::system_clock::to_time_t(time_point_);
        std::tm tm = {};
#ifdef _WIN32
        localtime_s(&tm, &timeT);
#else
        localtime_r(&timeT, &tm);
#endif
        return tm;
    }

    bool Date::isValidDate(const int32_t year, const int32_t month, const int32_t day, const int32_t hours, const int32_t minutes, const int32_t seconds)
    {
        if (month < 1 || month > 12)
        {
            return false;
        }

        if (day < 1 || day > getDaysInMonth(year, month))
        {
            return false;
        }

        if (hours < 0 || hours > 23)
        {
            return false;
        }

        if (minutes < 0 || minutes > 59)
        {
            return false;
        }

        if (seconds < 0 || seconds > 59)
        {
            return false;
        }

        return true;
    }
}
