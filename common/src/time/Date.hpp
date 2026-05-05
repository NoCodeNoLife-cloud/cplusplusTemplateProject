#pragma once
#include <chrono>
#include <string>
#include <cstdint>

namespace common::time {
/// @brief A class representing a specific instant in time, with millisecond precision.
/// The class represents a specific point in time, measured in milliseconds
/// since the Unix epoch (January 1, 1970, 00:00:00 GMT). It provides methods
/// for creating, manipulating, and comparing dates, as well as converting
/// between different date representations.
class Date {
public:
    /// @brief Default constructor, initializes to current time
    Date() noexcept;

    /// @brief Construct a date with year, month, and day
    /// @param year The year (e.g., 2023)
    /// @param month The month (1-12)
    /// @param day The day of the month (1-31)
    /// @throws std::invalid_argument If date components are invalid
    Date(int32_t year, int32_t month, int32_t day);

    /// @brief Construct a date with year, month, day, hours, minutes, and seconds
    /// @param year The year (e.g., 2023)
    /// @param month The month (1-12)
    /// @param day The day of the month (1-31)
    /// @param hours The hours (0-23)
    /// @param minutes The minutes (0-59)
    /// @param seconds The seconds (0-59)
    /// @throws std::invalid_argument If date-time components are invalid
    Date(int32_t year, int32_t month, int32_t day, int32_t hours, int32_t minutes, int32_t seconds);

    /// @brief Construct a date from a timestamp
    /// @param timestamp Milliseconds since Unix epoch
    explicit Date(int64_t timestamp) noexcept;

    /// @brief Destructor
    ~Date() = default;

    /// @brief Creates and returns a copy of this object.
    /// @return A copy of this date
    [[nodiscard]] auto clone() const -> Date;

    /// @brief Compares this date to the specified date.
    /// @param other The date to compare with
    /// @return true if the dates are equal, false otherwise
    [[nodiscard]] auto equals(const Date& other) const noexcept -> bool;

    /// @brief Tests if this date is after the specified date.
    /// @param other The date to compare with
    /// @return true if this date is after the other date, false otherwise
    [[nodiscard]] auto after(const Date& other) const noexcept -> bool;

    /// @brief Tests if this date is before the specified date.
    /// @param other The date to compare with
    /// @return true if this date is before the other date, false otherwise
    [[nodiscard]] auto before(const Date& other) const noexcept -> bool;

    /// @brief Returns the number of milliseconds since January 1, 1970, 00:00:00 GMT.
    /// @return The timestamp in milliseconds
    [[nodiscard]] auto getTime() const noexcept -> int64_t;

    /// @brief Returns the year represented by this date.
    /// @return The year (e.g., 2023)
    [[nodiscard]] auto getYear() const -> int32_t;

    /// @brief Returns the month represented by this date.
    /// @return The month (1-12)
    [[nodiscard]] auto getMonth() const -> int32_t;

    /// @brief Returns the day of the month represented by this date.
    /// @return The day of the month (1-31)
    [[nodiscard]] auto getDay() const -> int32_t;

    /// @brief Returns the hour represented by this date.
    /// @return The hour (0-23)
    [[nodiscard]] auto getHours() const -> int32_t;

    /// @brief Returns the minute represented by this date.
    /// @return The minute (0-59)
    [[nodiscard]] auto getMinutes() const -> int32_t;

    /// @brief Returns the second represented by this date.
    /// @return The second (0-59)
    [[nodiscard]] auto getSeconds() const -> int32_t;

    /// @brief Converts this date to a string representation.
    /// @return A string representation of the date in format "YYYY-MM-DD HH:MM:SS"
    [[nodiscard]] auto toString() const -> std::string;

    /// @brief Returns a hash code value for this date.
    /// @return The hash code value
    [[nodiscard]] auto hashCode() const noexcept -> size_t;

    // Comparison operators
    [[nodiscard]] bool operator==(const Date& other) const noexcept;

    [[nodiscard]] bool operator!=(const Date& other) const noexcept;

    [[nodiscard]] bool operator<(const Date& other) const noexcept;

    [[nodiscard]] bool operator<=(const Date& other) const noexcept;

    [[nodiscard]] bool operator>(const Date& other) const noexcept;

    [[nodiscard]] bool operator>=(const Date& other) const noexcept;

private:
    std::chrono::system_clock::time_point time_point_{};

    /// @brief Converts the time point to a std::tm structure.
    /// @return The tm structure representation of the date
    [[nodiscard]] auto toTm() const -> std::tm;

    /// @brief Validates if the given date components form a valid date
    /// @param year The year
    /// @param month The month (1-12)
    /// @param day The day (1-31)
    /// @param hours The hours (0-23)
    /// @param minutes The minutes (0-59)
    /// @param seconds The seconds (0-59)
    /// @return true if the date components are valid, false otherwise
    [[nodiscard]] static auto isValidDate(int32_t year, int32_t month, int32_t day, int32_t hours = 0, int32_t minutes = 0, int32_t seconds = 0) -> bool;
};
}