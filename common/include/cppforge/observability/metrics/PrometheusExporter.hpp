/// @file PrometheusExporter.hpp
/// @brief Prometheus text format metrics exporter for cppforge observability.
/// @details Implements IMetricsRegistry and serializes metrics in Prometheus exposition format.

#pragma once

#include <cppforge/observability/metrics/Counter.hpp>
#include <cppforge/observability/metrics/Gauge.hpp>
#include <cppforge/observability/metrics/Histogram.hpp>
#include <cppforge/observability/metrics/IMetricsRegistry.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace cppforge::observability::metrics
{

/// @brief Prometheus-compatible metrics registry and exporter.
/// @details Thread-safe implementation that serializes metrics in Prometheus text format.
class PrometheusExporter : public IMetricsRegistry
{
  public:
    void registerCounter(const std::string &name, const std::string &help) override;
    void incrementCounter(const std::string &name, double value) override;
    void registerGauge(const std::string &name, const std::string &help) override;
    void setGauge(const std::string &name, double value) override;
    void registerHistogram(const std::string &name, const std::string &help, std::vector<double> buckets) override;
    void observeHistogram(const std::string &name, double value) override;
    std::string serialize() const override;

  private:
    /// @brief Metadata for a registered metric.
    struct MetricMeta
    {
        std::string help;
        std::string type; // "counter", "gauge", "histogram"
    };

    mutable std::mutex mutex_;
    std::unordered_map<std::string, MetricMeta> metadata_;
    std::unordered_map<std::string, std::shared_ptr<Counter>> counters_;
    std::unordered_map<std::string, std::shared_ptr<Gauge>> gauges_;
    std::unordered_map<std::string, std::shared_ptr<Histogram>> histograms_;
};

} // namespace cppforge::observability::metrics
