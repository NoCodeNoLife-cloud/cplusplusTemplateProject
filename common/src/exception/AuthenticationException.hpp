/**
* @file AuthenticationException.hpp
 * @brief AuthenticationException class declaration
 * @details This header defines the AuthenticationException class that provides functionality for Custom exception classes for authentication and other operations.
 */

#pragma once
#include <stdexcept>
#include <string>

namespace common::exception
{
    /// @brief Custom exception class for authentication-related errors
    class [[nodiscard]] AuthenticationException : public std::runtime_error
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
        AuthenticationException(AuthenticationException&& other) ;

        /// @brief Copy constructor
        /// @param other Exception object to copy from
        AuthenticationException(const AuthenticationException& other);

        /// @brief Assignment operator
        /// @param other Exception object to assign from
        AuthenticationException& operator=(const AuthenticationException& other) = default;

        /// @brief Move assignment operator
        /// @param other Exception object to move from
        AuthenticationException& operator=(AuthenticationException&& other)  = default;

        /// @brief Virtual destructor for proper cleanup in inheritance hierarchy
        ~AuthenticationException()  override;
    };
}