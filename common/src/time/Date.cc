#include "src/time/Date.hpp"

#include <fmt/format.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <functional>
#include <ctime>

namespace common::time {
    namespace {
        // Helper function to check if a year is a leap year
        auto isLeapYear(const int32_t year) -> bool {
            return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        }

        // Helper function to get days in a month
        auto getDaysInMonth(const int32_t year, const int32_t month) -> int32_t {
            static const int32_t daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
            int32_t days = daysInMonth[month];
            if (month == 2 && isLeapYear(year)) {
                days = 29;
            }

            return days;
        }
    }

    auto Date::isValidDate(const int32_t year, const int32_t month, const int32_t day, const int32_t hours, const int32_t minutes, const int32_t seconds) -> bool {
        if (month < 1 || month > 12) {
            return false;
        }

        if (day < 1 || day > getDaysInMonth(year, month)) {
            return false;
        }

        if (hours < 0 || hours > 23) {
            return false;
        }

        if (minutes < 0 || minutes > 59) {
            return false;
        }

        if (seconds < 0 || seconds > 59) {
            return false;
        }

        return true;
    }

    Date::Date() noexcept : time_point_(std::chrono::system_clock::now()) {
    }

    Date::Date(const int32_t year, const int32_t month, const int32_t day) {
if (!isValidDate(year, month, day)) {
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
        if (timeT == -1) {
throw std::invalid_argument("Date::Date: Invalid date components.");
        }
        time_point_ = std::chrono::system_clock::from_time_t(timeT);
}

    Date::Date(const int32_t year, const int32_t month, const int32_t day, const int32_t hours, const int32_t minutes, const int32_t seconds) {
if (!isValidDate(year, month, day, hours, minutes, seconds)) {
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
        if (timeT == -1) {
throw std::invalid_argument("Date::Date: Invalid date-time components.");
        }
        time_point_ = std::chrono::system_clock::from_time_t(timeT);
}

    Date::Date(const int64_t timestamp) noexcept : time_point_(std::chrono::milliseconds(timestamp)) {
}

    auto Date::clone() const -> Date {
        return Date(getTime());
    }

    auto Date::equals(const Date &other) const noexcept -> bool {
        return time_point_ == other.time_point_;
    }

    auto Date::after(const Date &other) const noexcept -> bool {
        return time_point_ > other.time_point_;
    }

    auto Date::before(const Date &other) const noexcept -> bool {
        return time_point_ < other.time_point_;
    }

    auto Date::getTime() const noexcept -> int64_t {
        return std::chrono::duration_cast<std::chrono::milliseconds>(time_point_.time_since_epoch()).count();
    }

    auto Date::getYear() const -> int32_t {
        return toTm().tm_year + 1900;
    }

    auto Date::getMonth() const -> int32_t {
        return toTm().tm_mon + 1;
    }

    auto Date::getDay() const -> int32_t {
        return toTm().tm_mday;
    }

    auto Date::getHours() const -> int32_t {
        return toTm().tm_hour;
    }

    auto Date::getMinutes() const -> int32_t {
        return toTm().tm_min;
    }

    auto Date::getSeconds() const -> int32_t {
        return toTm().tm_sec;
    }

    auto Date::toString() const -> std::string {
        const auto tm = toTm();
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        const auto result = oss.str();
return result;
    }

    auto Date::hashCode() const noexcept -> size_t {
        return std::hash<int64_t>{}(getTime());
    }

    auto Date::toTm() const -> std::tm {
        const auto timeT = std::chrono::system_clock::to_time_t(time_point_);
        std::tm tm = {};
#ifdef _WIN32
        localtime_s(&tm, &timeT);
#else
        localtime_r(&timeT, &tm);
#endif
        return tm;
    }

    // Comparison operators
    bool Date::operator==(const Date &other) const noexcept {
        return equals(other);
    }

    bool Date::operator!=(const Date &other) const noexcept {
        return !equals(other);
    }

    bool Date::operator<(const Date &other) const noexcept {
        return before(other);
    }

    bool Date::operator<=(const Date &other) const noexcept {
        return before(other) || equals(other);
    }

    bool Date::operator>(const Date &other) const noexcept {
        return after(other);
    }

    bool Date::operator>=(const Date &other) const noexcept {
        return after(other) || equals(other);
    }
}
