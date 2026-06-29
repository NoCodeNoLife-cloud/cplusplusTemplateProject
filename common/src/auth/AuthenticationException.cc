/**
 * @file AuthenticationException.cc
 * @brief AuthenticationException implementation â€?error code and message
 * @details Implements the authentication exception with error code assignment
 *          and category label for structured error handling.
 */

#include "AuthenticationException.hpp"

#include <string_view>

namespace cppforge::auth
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
