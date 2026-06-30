/// @file Gauge.hpp
/// @brief Thread-safe gauge metric for cppforge observability.
/// @details A metric that can go up and down.

#pragma once

#include <mutex>

namespace cppforge::observability::metrics
{

/// @brief A thread-safe gauge that can be set to arbitrary values.
class Gauge
{
  public:
    /// @brief Construct a gauge with initial value 0.
    Gauge() = default;

    /// @brief Set the gauge to the given value.
    /// @param value The value to set.
    void set(double value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        value_ = value;
    }

    /// @brief Get the current gauge value.
    /// @return The current value.
    double get() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return value_;
    }

  private:
    mutable std::mutex mutex_;
    double value_ = 0.0;
};

} // namespace cppforge::observability::metrics
