/// @file PrometheusExporter.cc
/// @brief Implementation of the Prometheus metrics exporter.

#include <cppforge/observability/metrics/PrometheusExporter.hpp>
#include <glog/logging.h>
#include <sstream>

namespace cppforge::observability::metrics
{

void PrometheusExporter::registerCounter(const std::string &name, const std::string &help)
{
    std::lock_guard<std::mutex> lock(mutex_);
    metadata_[name] = {help, "counter"};
    counters_[name] = std::make_shared<Counter>();
}

void PrometheusExporter::incrementCounter(const std::string &name, double value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = counters_.find(name);
    if (it != counters_.end())
    {
        it->second->increment(value);
    }
    else
    {
        LOG(WARNING) << "Attempted to increment unregistered counter: " << name;
    }
}

void PrometheusExporter::registerGauge(const std::string &name, const std::string &help)
{
    std::lock_guard<std::mutex> lock(mutex_);
    metadata_[name] = {help, "gauge"};
    gauges_[name] = std::make_shared<Gauge>();
}

void PrometheusExporter::setGauge(const std::string &name, double value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = gauges_.find(name);
    if (it != gauges_.end())
    {
        it->second->set(value);
    }
    else
    {
        LOG(WARNING) << "Attempted to set unregistered gauge: " << name;
    }
}

void PrometheusExporter::registerHistogram(const std::string &name, const std::string &help, std::vector<double> buckets)
{
    std::lock_guard<std::mutex> lock(mutex_);
    metadata_[name] = {help, "histogram"};
    histograms_[name] = std::make_shared<Histogram>(std::move(buckets));
}

void PrometheusExporter::observeHistogram(const std::string &name, double value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = histograms_.find(name);
    if (it != histograms_.end())
    {
        it->second->observe(value);
    }
    else
    {
        LOG(WARNING) << "Attempted to observe unregistered histogram: " << name;
    }
}

std::string PrometheusExporter::serialize() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;

    for (const auto &[name, meta] : metadata_)
    {
        oss << "# HELP " << name << " " << meta.help << "\n";
        oss << "# TYPE " << name << " " << meta.type << "\n";

        if (meta.type == "counter")
        {
            auto it = counters_.find(name);
            if (it != counters_.end())
            {
                oss << name << " " << it->second->get() << "\n";
            }
        }
        else if (meta.type == "gauge")
        {
            auto it = gauges_.find(name);
            if (it != gauges_.end())
            {
                oss << name << " " << it->second->get() << "\n";
            }
        }
        else if (meta.type == "histogram")
        {
            auto it = histograms_.find(name);
            if (it != histograms_.end())
            {
                const auto &hist = it->second;
                auto cumulative = hist->cumulativeCounts();
                const auto &buckets = hist->buckets();
                for (std::size_t i = 0; i < buckets.size(); ++i)
                {
                    oss << name << "_bucket{le=\"";
                    if (buckets[i] == std::numeric_limits<double>::infinity())
                    {
                        oss << "+Inf";
                    }
                    else
                    {
                        oss << buckets[i];
                    }
                    oss << "\"} " << cumulative[i] << "\n";
                }
                // +Inf bucket
                oss << name << "_bucket{le=\"+Inf\"} " << hist->count() << "\n";
                oss << name << "_sum " << hist->sum() << "\n";
                oss << name << "_count " << hist->count() << "\n";
            }
        }
    }

    return oss.str();
}

} // namespace cppforge::observability::metrics