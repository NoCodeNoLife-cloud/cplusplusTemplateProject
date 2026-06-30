/// @file IHealthCheck.hpp
/// @brief Health check interface for cppforge observability.
/// @details Defines the contract for individual health check components.

#pragma once

#include <string>

namespace cppforge::observability::health
{

/// @brief Abstract health check interface.
/// @details Each health check reports whether a specific component is healthy.
class IHealthCheck
{
  public:
    virtual ~IHealthCheck() = default;

    /// @brief Get the name of this health check.
    /// @return The check name.
    virtual std::string name() const = 0;

    /// @brief Perform the health check.
    /// @return true if healthy, false otherwise.
    virtual bool check() const = 0;
};

} // namespace cppforge::observability::health
