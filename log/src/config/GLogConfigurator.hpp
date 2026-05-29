/**
 * @file GLogConfigurator.hpp
 * @brief Google Logging (glog) configuration manager
 * @details This header defines the GLogConfigurator class that handles initialization
 *          and configuration of the glog library based on YAML configuration files,
 *          including log level settings, output destinations, and custom formatting.
 */

#pragma once
#include <string>

#include "parameter/GLogParameters.hpp"

namespace glog::config
{
    /// @brief Configures Google Logging (glog) library with specified parameters
    /// @details This class handles the initialization and configuration of the glog library
    /// based on the provided configuration parameters
    class GLogConfigurator final
    {
    public:
        /// @brief Default constructor is deleted to enforce path initialization
        GLogConfigurator() = delete;

        /// @brief Constructor with YAML configuration file path
        /// @param glog_yaml_path Path to the YAML configuration file
        explicit GLogConfigurator(std::string glog_yaml_path) noexcept;

        /// @brief Destructor
        ~GLogConfigurator() noexcept = default;

        /// @brief Copy constructor is deleted to prevent copying
        GLogConfigurator(const GLogConfigurator&) = delete;

        /// @brief Copy assignment operator is deleted to prevent copying
        GLogConfigurator& operator=(const GLogConfigurator&) = delete;

        /// @brief Move constructor
        GLogConfigurator(GLogConfigurator&&) noexcept = default;

        /// @brief Move assignment operator
        GLogConfigurator& operator=(GLogConfigurator&&) noexcept = default;

        /// @brief Execute the configuration process
        /// @return True if configuration was successful
        void execute() const;

        /// @brief Get the current configuration parameters
        /// @return A const reference to the GLogParameters object
        [[nodiscard]] const parameter::GLogParameters& getConfig() const noexcept;

        /// @brief Update the configuration parameters
        /// @param config The new configuration parameters
        void updateConfig(const parameter::GLogParameters& config) noexcept;

    private:
        /// @brief Perform the actual glog configuration
        static void doConfig(const parameter::GLogParameters& config) noexcept;

        /// @brief Clean up glog resources
        static void clean() noexcept;

        std::string glog_yaml_path_;
        parameter::GLogParameters config_;
    };
}