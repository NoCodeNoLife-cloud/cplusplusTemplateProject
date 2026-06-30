/**
 * @file TracingStarter.cc
 * @brief DI registration implementation for the tracing starter
 */

#include <cppforge/starter/tracing/TracingStarter.hpp>
#include <cppforge/starter/tracing/Tracer.hpp>
#include <glog/logging.h>

namespace cppforge::starter::tracing
{
    void TracingStarter::registerServices(di::IServiceCollection& collection, const TracingConfig& config)
    {
        collection.addSingleton<Tracer>([config]() -> std::shared_ptr<Tracer> {
            LOG(INFO) << "Registering Tracer singleton (service=" << config.service_name
                      << ", enabled=" << (config.enabled ? "true" : "false")
                      << ", max_spans=" << config.max_spans_stored << ")";
            return std::make_shared<Tracer>(config.max_spans_stored);
        });
    }
}
