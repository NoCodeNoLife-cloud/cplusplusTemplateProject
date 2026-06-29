/**
 * @file GLogConfigurator.hpp
 * @brief Google Logging (glog) configuration manager
 * @details This header defines the GLogConfigurator class that handles initialization
 *          and configuration of the glog library based on YAML configuration files,
 *          including log level settings, output destinations, and custom formatting.
 */

#pragma once
#include <atomic>
#include <mutex>
#include <string>

#include <cppforge/glog/param/GLogParam.hpp>

namespace cppforge::glog::config
{
    /// @brief Configures Google Logging (glog) library with specified parameters
    /// @details This class handles the initialization and configuration of the glog library
    /// based on the provided configuration parameters
    class GLogConfigurator final
    {
    public:
        GLogConfigurator() = delete;

        /// @param glog_yaml_path Path to the YAML configuration file
        explicit GLogConfigurator(std::string glog_yaml_path);

        ~GLogConfigurator() noexcept = default;

        GLogConfigurator(const GLogConfigurator&) = delete;
        GLogConfigurator& operator=(const GLogConfigurator&) = delete;
        GLogConfigurator(GLogConfigurator&&) noexcept = default;
        GLogConfigurator& operator=(GLogConfigurator&&) noexcept = default;

        /// @brief Execute the configuration process (idempotent, thread-safe)
        void execute() const;

        /// @brief Get the current configuration parameters
        [[nodiscard]] auto getConfig() const noexcept -> const param::GLogParam&;

        /// @brief Update the configuration parameters
        void updateConfig(const param::GLogParam& config) noexcept;

        /// @brief Check whether glog has been initialized by this configurator
        [[nodiscard]] static auto isInitialized() noexcept -> bool;

        /// @brief Clean up glog resources (also registered via atexit)
        static void clean() noexcept;

    private:
        static void doConfig(const param::GLogParam& config) noexcept;

        std::string glog_yaml_path_;
        param::GLogParam config_;
        static std::once_flag glog_init_flag_;
        static std::atomic<bool> glog_initialized_;
    };
}
