/// @file Histogram.hpp
/// @brief Thread-safe histogram metric for cppforge observability.
/// @details Tracks the distribution of observed values across configurable buckets.

#pragma once

#include <algorithm>
#include <mutex>
#include <numeric>
#include <vector>

namespace cppforge::observability::metrics
{

/// @brief A thread-safe histogram that tracks value distributions across buckets.
class Histogram
{
  public:
    /// @brief Construct a histogram with the given bucket boundaries.
    /// @param buckets Sorted bucket boundary values.
    explicit Histogram(std::vector<double> buckets) : buckets_(std::move(buckets)), counts_(buckets_.size() + 1, 0)
    {
        std::sort(buckets_.begin(), buckets_.end());
    }

    /// @brief Observe a value, incrementing the appropriate bucket count.
    /// @param value The observed value.
    void observe(double value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sum_ += value;
        count_++;
        auto it = std::lower_bound(buckets_.begin(), buckets_.end(), value);
        auto idx = static_cast<std::size_t>(std::distance(buckets_.begin(), it));
        counts_[idx]++;
    }

    /// @brief Get the bucket boundaries.
    /// @return Reference to the bucket boundaries.
    const std::vector<double> &buckets() const
    {
        return buckets_;
    }

    /// @brief Get the cumulative counts for each bucket.
    /// @return Vector of cumulative counts (size = buckets.size() + 1 for +Inf).
    std::vector<std::uint64_t> cumulativeCounts() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::uint64_t> cumulative(counts_.size(), 0);
        std::partial_sum(counts_.begin(), counts_.end(), cumulative.begin());
        return cumulative;
    }

    /// @brief Get the total count of observations.
    /// @return The total observation count.
    std::uint64_t count() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_;
    }

    /// @brief Get the sum of all observed values.
    /// @return The sum.
    double sum() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return sum_;
    }

  private:
    mutable std::mutex mutex_;
    std::vector<double> buckets_;
    std::vector<std::uint64_t> counts_;
    double sum_ = 0.0;
    std::uint64_t count_ = 0;
};

} // namespace cppforge::observability::metrics
