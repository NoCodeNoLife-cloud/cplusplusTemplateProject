/**
 * @file BigInteger.hpp
 * @brief BigInteger class declaration
 * @details This header defines the BigInteger class that provides functionality for Base type classes providing fundamental object functionality.
 */

#pragma once
#include <boost/multiprecision/cpp_int.hpp>
#include <string>
#include <compare>

namespace common::base_type
{
    /// @brief A class representing a big integer using boost::multiprecision::cpp_int
    class BigInteger
    {
    public:
        /// @brief Default constructor, initializes to zero
        BigInteger() noexcept;

        /// @brief Constructs a BigInteger from a string representation
        /// @param str The string representation of the integer
        explicit BigInteger(const std::string& str);

        /// @brief Constructs a BigInteger from an int64_t value
        /// @param num The int64_t value to convert
        explicit BigInteger(int64_t num) noexcept;

        /// @brief Addition operator
        /// @param other The BigInteger to add
        /// @return The result of the addition
        [[nodiscard]] BigInteger operator+(const BigInteger& other) const noexcept;

        /// @brief Subtraction operator
        /// @param other The BigInteger to subtract
        /// @return The result of the subtraction
        [[nodiscard]] BigInteger operator-(const BigInteger& other) const noexcept;

        /// @brief Multiplication operator
        /// @param other The BigInteger to multiply
        /// @return The result of the multiplication
        [[nodiscard]] BigInteger operator*(const BigInteger& other) const noexcept;

        /// @brief Division operator
        /// @param other The BigInteger to divide by
        /// @return The result of the division
        /// @throws std::invalid_argument If attempting to divide by zero
        [[nodiscard]] BigInteger operator/(const BigInteger& other) const;

        /// @brief Modulus operator
        /// @param other The BigInteger to modulo by
        /// @return The result of the modulus operation
        /// @throws std::invalid_argument If attempting to modulo by zero
        [[nodiscard]] BigInteger operator%(const BigInteger& other) const;

        /// @brief Three-way comparison operator
        /// @param other The BigInteger to compare with
        /// @return The result of the comparison
        [[nodiscard]] std::strong_ordering operator<=>(const BigInteger& other) const noexcept;

        /// @brief Equality comparison operator
        /// @param other The BigInteger to compare with
        /// @return true if the values are equal, false otherwise
        [[nodiscard]] bool operator==(const BigInteger& other) const noexcept;

        /// @brief Create a BigInteger from a string
        /// @param str The string to convert
        /// @return The resulting BigInteger
        [[nodiscard]] static BigInteger fromString(const std::string& str);

        /// @brief Create a BigInteger from an int64_t
        /// @param num The int64_t to convert
        /// @return The resulting BigInteger
        [[nodiscard]] static BigInteger fromInt(int64_t num) noexcept;

    private:
        /// @brief Internal constructor from cpp_int (avoids string conversion overhead)
        /// @param value The cpp_int value to wrap
        explicit BigInteger(boost::multiprecision::cpp_int value) noexcept : value_(std::move(value))
        {
        }

        boost::multiprecision::cpp_int value_{};
    };
}