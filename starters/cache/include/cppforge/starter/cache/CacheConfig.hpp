/**
 * @file CacheConfig.hpp
 * @brief Configuration options for the cache starter
 * @details Defines CacheConfig struct that encapsulates L1 (in-memory) and L2 (Redis)
 *          cache parameters including capacity, TTL, and connection settings.
 */

#pragma once
#include <chrono>
#include <cstdint>
#include <string>

namespace cppforge::starter::cache
{
    /// @brief Configuration for the multi-level cache system
    /// @details Holds parameters for L1 (in-memory LRU) and optional L2 (Redis) cache layers.
    struct CacheConfig
    {
        /// @brief Maximum number of entries in the L1 cache
        std::size_t l1_capacity = 1000;

        /// @brief Time-to-live for L1 cache entries
        std::chrono::seconds l1_ttl = std::chrono::seconds(300);

        /// @brief Whether to enable the L2 (Redis) cache layer
        bool enable_l2 = false;

        /// @brief Redis server hostname
        std::string redis_host = "localhost";

        /// @brief Redis server port
        uint16_t redis_port = 6379;

        /// @brief Time-to-live for L2 cache entries
        std::chrono::seconds l2_ttl = std::chrono::seconds(3600);
    };
}
