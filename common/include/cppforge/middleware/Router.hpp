/// @file Router.hpp
/// @brief HTTP router for cppforge framework.
/// @details Maps HTTP method + path combinations to handler functions.

#pragma once

#include <cppforge/http/IHttpRequest.hpp>
#include <cppforge/http/IHttpResponse.hpp>
#include <functional>
#include <string>
#include <vector>

namespace cppforge::middleware
{

/// @brief Routes HTTP requests to registered handlers based on method and path.
class Router
{
  public:
    /// @brief Handler function type.
    using Handler = std::function<void(const http::IHttpRequest &, http::IHttpResponse &)>;

    /// @brief Register a GET route.
    /// @param path The URL path to match.
    /// @param handler The handler function.
    /// @return Reference to this router for chaining.
    Router &get(const std::string &path, Handler handler);

    /// @brief Register a POST route.
    /// @param path The URL path to match.
    /// @param handler The handler function.
    /// @return Reference to this router for chaining.
    Router &post(const std::string &path, Handler handler);

    /// @brief Register a PUT route.
    /// @param path The URL path to match.
    /// @param handler The handler function.
    /// @return Reference to this router for chaining.
    Router &put(const std::string &path, Handler handler);

    /// @brief Register a DELETE route.
    /// @param path The URL path to match.
    /// @param handler The handler function.
    /// @return Reference to this router for chaining.
    Router &del(const std::string &path, Handler handler);

    /// @brief Handle an incoming request by matching it to a registered route.
    /// @param req The incoming HTTP request.
    /// @param res The outgoing HTTP response.
    void handle(const http::IHttpRequest &req, http::IHttpResponse &res) const;

  private:
    /// @brief A registered route entry.
    struct Route
    {
        std::string method;
        std::string path;
        Handler handler;
    };

    std::vector<Route> routes_;
};

} // namespace cppforge::middleware
