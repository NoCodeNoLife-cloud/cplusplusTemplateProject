/**
 * @file TracingConfig.hpp
 * @brief Configuration options for the tracing starter
 * @details Defines TracingConfig struct with service name, enable flag, and max spans.
 */

#pragma once
#include <cstddef>
#include <string>

namespace cppforge::starter::tracing
{
    /// @brief Configuration for the distributed tracing subsystem
    struct TracingConfig
    {
        /// @brief Name of the service (used as a tag on all spans)
        std::string service_name = "cppforge";

        /// @brief Whether tracing is enabled
        bool enabled = true;

        /// @brief Maximum number of completed spans to store in memory
        std::size_t max_spans_stored = 10000;
    };
}
