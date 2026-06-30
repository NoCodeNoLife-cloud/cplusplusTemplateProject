/// @file IMiddleware.hpp
/// @brief Middleware interface for cppforge HTTP pipeline.
/// @details Defines the contract for HTTP middleware components.

#pragma once

#include <cppforge/http/IHttpRequest.hpp>
#include <cppforge/http/IHttpResponse.hpp>
#include <functional>

namespace cppforge::middleware
{

/// @brief Abstract middleware interface.
/// @details Each middleware processes a request/response pair and optionally calls next() to continue the chain.
class IMiddleware
{
  public:
    virtual ~IMiddleware() = default;

    /// @brief Handle an HTTP request/response pair.
    /// @param req The incoming HTTP request.
    /// @param res The outgoing HTTP response.
    /// @param next Callback to invoke the next middleware in the chain.
    virtual void handle(const http::IHttpRequest &req, http::IHttpResponse &res, std::function<void()> next) = 0;
};

} // namespace cppforge::middleware
