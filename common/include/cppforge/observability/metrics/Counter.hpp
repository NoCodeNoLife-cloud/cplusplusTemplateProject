/// @file Counter.hpp
/// @brief Thread-safe counter metric for cppforge observability.
/// @details A monotonically increasing counter value.

#pragma once

#include <mutex>

namespace cppforge::observability::metrics
{

/// @brief A thread-safe monotonically increasing counter.
class Counter
{
  public:
    /// @brief Construct a counter with initial value 0.
    Counter() = default;

    /// @brief Increment the counter by the given value.
    /// @param value The increment amount (default 1.0).
    void increment(double value = 1.0)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        value_ += value;
    }

    /// @brief Get the current counter value.
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
