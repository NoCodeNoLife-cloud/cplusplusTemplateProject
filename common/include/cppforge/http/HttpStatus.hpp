/// @file HttpStatus.hpp
/// @brief HTTP status code enumeration for cppforge framework.
/// @details Defines common HTTP status codes as a strongly-typed enum.

#pragma once

namespace cppforge::http
{

/// @brief Common HTTP status codes.
enum class HttpStatus
{
    OK = 200,
    Created = 201,
    NoContent = 204,
    BadRequest = 400,
    Unauthorized = 401,
    Forbidden = 403,
    NotFound = 404,
    InternalServerError = 500,
    ServiceUnavailable = 503
};

} // namespace cppforge::http