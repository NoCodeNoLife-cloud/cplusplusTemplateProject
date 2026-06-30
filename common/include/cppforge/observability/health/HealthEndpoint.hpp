/// @file HealthEndpoint.hpp
/// @brief Health endpoint for cppforge observability.
/// @details Aggregates multiple health checks and produces a JSON status report.

#pragma once

#include <cppforge/observability/health/IHealthCheck.hpp>
#include <cppforge/http/IHttpRequest.hpp>
#include <cppforge/http/IHttpResponse.hpp>
#include <memory>
#include <vector>

namespace cppforge::observability::health
{

/// @brief Aggregates health checks and produces a JSON health report.
/// @details Runs all registered health checks and returns a JSON response
///          with overall status and individual check results.
class HealthEndpoint
{
  public:
    /// @brief Register a health check.
    /// @param check The health check to register.
    void registerCheck(std::shared_ptr<IHealthCheck> check);

    /// @brief Handle an HTTP request by running all health checks.
    /// @param req The incoming HTTP request.
    /// @param res The outgoing HTTP response (set to JSON body).
    void handle(const http::IHttpRequest &req, http::IHttpResponse &res) const;

  private:
    std::vector<std::shared_ptr<IHealthCheck>> checks_;
};

} // namespace cppforge::observability::health

