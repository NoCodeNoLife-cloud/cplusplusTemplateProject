/// @file MiddlewareChain.hpp
/// @brief Middleware chain for cppforge HTTP pipeline.
/// @details Manages an ordered sequence of middleware and executes them in order.

#pragma once

#include <cppforge/http/IHttpRequest.hpp>
#include <cppforge/http/IHttpResponse.hpp>
#include <cppforge/middleware/IMiddleware.hpp>
#include <memory>
#include <vector>

namespace cppforge::middleware
{

/// @brief Executes an ordered chain of middleware components.
/// @details Middleware are executed in the order they are added. Each middleware
///          can short-circuit the chain by not calling next().
/// @note Not thread-safe. Build the chain before sharing across threads.
class MiddlewareChain
{
  public:
    /// @brief Add a middleware to the chain.
    /// @param middleware The middleware to add.
    void add(std::shared_ptr<IMiddleware> middleware);

    /// @brief Execute the middleware chain.
    /// @param req The incoming HTTP request.
    /// @param res The outgoing HTTP response.
    void execute(const http::IHttpRequest &req, http::IHttpResponse &res) const;

    /// @brief Get the number of middleware in the chain.
    /// @return The count of registered middleware.
    std::size_t size() const;

  private:
    std::vector<std::shared_ptr<IMiddleware>> middlewares_;
};

} // namespace cppforge::middleware