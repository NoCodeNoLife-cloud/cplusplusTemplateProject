/// @file IHttpResponse.hpp
/// @brief HTTP response interface for cppforge framework.
/// @details Defines the contract for writing HTTP response data.

#pragma once

#include <string>

namespace cppforge::http
{

/// @brief Abstract HTTP response interface.
/// @details Provides methods to set status code, headers, and body content.
class IHttpResponse
{
  public:
    virtual ~IHttpResponse() = default;

    /// @brief Set the HTTP status code.
    /// @param code The status code (e.g., 200, 404).
    virtual void status(int code) = 0;

    /// @brief Set a response header.
    /// @param key The header name.
    /// @param value The header value.
    virtual void header(const std::string &key, const std::string &value) = 0;

    /// @brief Set the response body content.
    /// @param content The body content as a string.
    virtual void body(const std::string &content) = 0;
};

} // namespace cppforge::http
