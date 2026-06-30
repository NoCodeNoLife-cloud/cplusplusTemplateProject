/// @file IHttpRequest.hpp
/// @brief HTTP request interface for cppforge framework.
/// @details Defines the contract for reading HTTP request data.

#pragma once

#include <string>

namespace cppforge::http
{

/// @brief Abstract HTTP request interface.
/// @details Provides read-only access to method, path, headers, body, and query parameters.
class IHttpRequest
{
  public:
    virtual ~IHttpRequest() = default;

    /// @brief Get the HTTP method (GET, POST, PUT, DELETE, etc.).
    /// @return The request method as a string.
    virtual std::string method() const = 0;

    /// @brief Get the request path.
    /// @return The URL path.
    virtual std::string path() const = 0;

    /// @brief Get a request header value by key.
    /// @param key The header name.
    /// @return The header value, or empty string if not found.
    virtual std::string header(const std::string &key) const = 0;

    /// @brief Get the request body.
    /// @return The body content as a string.
    virtual std::string body() const = 0;

    /// @brief Get a query parameter value by key.
    /// @param key The parameter name.
    /// @return The parameter value, or empty string if not found.
    virtual std::string queryParam(const std::string &key) const = 0;
};

} // namespace cppforge::http
