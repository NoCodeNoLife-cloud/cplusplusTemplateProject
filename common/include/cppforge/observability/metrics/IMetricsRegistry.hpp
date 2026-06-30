/// @file IMetricsRegistry.hpp
/// @brief Metrics registry interface for cppforge observability.
/// @details Defines the contract for registering and manipulating metrics.

#pragma once

#include <string>
#include <vector>

namespace cppforge::observability::metrics
{

/// @brief Abstract metrics registry interface.
/// @details Provides methods to register and update counters, gauges, and histograms.
class IMetricsRegistry
{
  public:
    virtual ~IMetricsRegistry() = default;

    /// @brief Register a new counter metric.
    /// @param name The metric name.
    /// @param help The metric help text.
    virtual void registerCounter(const std::string &name, const std::string &help) = 0;

    /// @brief Increment a counter by the given value.
    /// @param name The counter name.
    /// @param value The increment amount (default 1.0).
    virtual void incrementCounter(const std::string &name, double value = 1.0) = 0;

    /// @brief Register a new gauge metric.
    /// @param name The metric name.
    /// @param help The metric help text.
    virtual void registerGauge(const std::string &name, const std::string &help) = 0;

    /// @brief Set a gauge to the given value.
    /// @param name The gauge name.
    /// @param value The value to set.
    virtual void setGauge(const std::string &name, double value) = 0;

    /// @brief Register a new histogram metric.
    /// @param name The metric name.
    /// @param help The metric help text.
    /// @param buckets The bucket boundaries.
    virtual void registerHistogram(const std::string &name, const std::string &help, std::vector<double> buckets) = 0;

    /// @brief Observe a value in a histogram.
    /// @param name The histogram name.
    /// @param value The observed value.
    virtual void observeHistogram(const std::string &name, double value) = 0;

    /// @brief Serialize all metrics to Prometheus text format.
    /// @return The serialized metrics string.
    virtual std::string serialize() const = 0;
};

} // namespace cppforge::observability::metrics
