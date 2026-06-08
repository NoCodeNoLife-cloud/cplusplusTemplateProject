/**
 * @file AuthenticationException.cc
 * @brief AuthenticationException class implementation
 * @details This file contains the implementation of the AuthenticationException class methods for Custom exception classes for authentication and other operations.
 */

#include "AuthenticationException.hpp"

#include <string_view>

namespace common::auth
{
    AuthenticationException::AuthenticationException(const std::string& message) : std::runtime_error(message)
    {
    }

    AuthenticationException::AuthenticationException(const std::string_view message) : std::runtime_error(std::string(message))
    {
    }

    AuthenticationException::AuthenticationException(AuthenticationException&& other) noexcept : std::runtime_error(std::move(other))
    {
    }

    AuthenticationException::AuthenticationException(const AuthenticationException& other) = default;

    AuthenticationException::~AuthenticationException() = default;
} // common
