/**
 * @file GLogParameters.hpp
 * @brief Configuration parameters for Google Logging (glog)
 * @details This header defines the GLogParameters class that encapsulates all configuration
 *          options for the glog logging system, including log level, log name, output settings,
 *          custom format options, and YAML-based configuration loading support.
 */

#pragma once
#include <filesystem>
#include <string>
#include <yaml-cpp/node/node.h>

#include "interface/IYamlConfigurable.hpp"

namespace glog::parameter
{
    /// @brief Configuration parameters for Google Logging (glog) library.
    /// @details This class encapsulates all the configuration options for the glog logging system.
    /// It provides getter and setter methods for each parameter and supports YAML serialization.
    class GLogParameters final : public common::interfaces::IYamlConfigurable
    {
    public:
        /// @brief Default constructor.
        GLogParameters() = default;

        /// @brief Constructor with parameters.
        /// @param min_log_level Minimum log level
        /// @param log_name Log name
        /// @param log_to_stderr Whether to log to stderr
        GLogParameters(int32_t min_log_level, std::string log_name, bool log_to_stderr);

        /// @brief Get the minimum log level.
        /// @return The minimum log level as an integer.
        [[nodiscard]] auto minLogLevel() const noexcept -> int32_t;

        /// @brief Set the minimum log level.
        /// @param min_log_level The minimum log level to set.
        void minLogLevel(int32_t min_log_level) noexcept;

        /// @brief Get the log name.
        /// @return The log name as a string.
        [[nodiscard]] auto logName() const noexcept -> std::string;

        /// @brief Set the log name.
        /// @param log_name The log name to set.
        void logName(const std::string& log_name);

        /// @brief Check if logging to stderr is enabled.
        /// @return True if logging to stderr is enabled, false otherwise.
        [[nodiscard]] auto logToStderr() const noexcept -> bool;

        /// @brief Enable or disable logging to stderr.
        /// @param log_to_stderr True to enable logging to stderr, false to disable.
        void logToStderr(bool log_to_stderr) noexcept;

        /// @brief Check if custom log format is enabled.
        /// @return True if custom log format is enabled, false otherwise.
        [[nodiscard]] auto customLogFormat() const noexcept -> bool;

        /// @brief Enable or disable custom log format.
        /// @param custom_log_format True to enable custom log format, false to disable.
        void customLogFormat(bool custom_log_format) noexcept;

        /// @brief Deserialize object configuration from a YAML file
        /// @param path The file path to the YAML configuration file
        /// @throws std::runtime_error If the file cannot be read or parsed
        void deserializedFromYamlFile(const std::filesystem::path& path) override;

        /// @brief Equality operator.
        /// @param other The other GLogParameters to compare with.
        /// @return True if both objects are equal, false otherwise.
        [[nodiscard]] auto operator==(const GLogParameters& other) const noexcept -> bool;

        /// @brief Inequality operator.
        /// @param other The other GLogParameters to compare with.
        /// @return True if both objects are not equal, false otherwise.
        [[nodiscard]] auto operator!=(const GLogParameters& other) const noexcept -> bool;

    private:
        int32_t min_log_level_{};
        std::string log_name_{};
        bool log_to_stderr_{};
        bool custom_log_format_{false};
    };
}

/// @brief YAML serialization specialization for GLogParameters.
/// Provides methods to encode and decode GLogParameters to/from YAML nodes.
template <>
struct YAML::convert<glog::parameter::GLogParameters>
{
    /// @brief Decode a YAML node into a GLogParameters object.
    /// @param node The YAML node containing the configuration data.
    /// @param rhs The GLogParameters object to populate.
    /// @return True if decoding was successful.
    static bool decode(const Node& node, glog::parameter::GLogParameters& rhs);

    /// @brief Encode a GLogParameters object into a YAML node.
    /// @param rhs The GLogParameters object to encode.
    /// @return A YAML node containing the configuration data.
    static Node encode(const glog::parameter::GLogParameters& rhs);
};