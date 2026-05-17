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

namespace glog::config {
/// @brief Configures Google Logging (glog) library with specified parameters
/// @details This class handles the initialization and configuration of the glog library
/// based on the provided configuration parameters
class GLogConfigurator final {
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
    auto operator=(const GLogConfigurator&) -> GLogConfigurator& = delete;

    /// @brief Move constructor
    GLogConfigurator(GLogConfigurator&&) noexcept = default;

    /// @brief Move assignment operator
    auto operator=(GLogConfigurator&&) noexcept -> GLogConfigurator& = default;

    /// @brief Execute the configuration process
    /// @return True if configuration was successful
    auto execute() const -> void;

    /// @brief Get the current configuration parameters
    /// @return A const reference to the GLogParameters object
    [[nodiscard]] auto getConfig() const noexcept -> const parameter::GLogParameters&;

    /// @brief Update the configuration parameters
    /// @param config The new configuration parameters
    auto updateConfig(const parameter::GLogParameters& config) noexcept -> void;

private:
    /// @brief Perform the actual glog configuration
    static auto doConfig(const parameter::GLogParameters& config) noexcept -> void;

    /// @brief Clean up glog resources
    static auto clean() noexcept -> void;

    std::string glog_yaml_path_;
    parameter::GLogParameters config_;
};
}