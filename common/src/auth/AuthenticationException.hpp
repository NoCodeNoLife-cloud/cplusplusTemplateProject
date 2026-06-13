/**
 * @file AuthenticationException.hpp
 * @brief Exception type for authentication/authorisation failures
 * @details Represents errors during authentication (login failure, token
 *          expired) and authorisation (insufficient permissions).  Extends
 *          std::runtime_error with an error code and category for structured
 *          error handling.
 *
 * @par Thread Safety
 * This class is const-only thread-safe (immutable after construction).
 */

#pragma once
#include <stdexcept>
#include <string>
#include <string_view>

namespace common::auth
{
    /// @brief Custom exception class for authentication-related errors
    class AuthenticationException : public std::runtime_error
    {
    public:
        /// @brief Constructor with error message
        /// @param message Error description
        explicit AuthenticationException(const std::string& message);

        /// @brief Constructor with error message using C++17 string_view for better performance
        /// @param message Error description
        /// @note Provides efficient string handling without unnecessary copies when possible
        explicit AuthenticationException(std::string_view message);

        /// @brief Move constructor for efficient transfer of exception objects
        /// @param other Exception object to move from
        AuthenticationException(AuthenticationException&& other) noexcept ;

        /// @brief Copy constructor
        /// @param other Exception object to copy from
        AuthenticationException(const AuthenticationException& other);

        /// @brief Assignment operator
        /// @param other Exception object to assign from
        AuthenticationException& operator=(const AuthenticationException& other) = default;

        /// @brief Move assignment operator
        /// @param other Exception object to move from
        AuthenticationException& operator=(AuthenticationException&& other) = default;

        /// @brief Virtual destructor for proper cleanup in inheritance hierarchy
        ~AuthenticationException() override;
    };
}