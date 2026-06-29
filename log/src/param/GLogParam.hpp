/**
 * @file GLogParam.hpp
 * @brief Configuration parameters for Google Logging (glog)
 * @details This header defines the GLogParam class that encapsulates all configuration
 *          options for the glog logging system, including log level, log name, output settings,
 *          custom format options, and YAML-based configuration loading support.
 */

#pragma once
#include <filesystem>
#include <string>
#include <yaml-cpp/node/node.h>

#include "interface/serialization/IYamlConfigurable.hpp"

namespace cppforge::glog::param
{
    class GLogParam final : public cppforge::interface::serialization::IYamlConfigurable
    {
    public:
        GLogParam() = default;

        GLogParam(int32_t min_log_level, std::string log_name, bool log_to_stderr,
                  bool custom_log_format = false);

        [[nodiscard]] auto minLogLevel() const noexcept -> int32_t;
        void minLogLevel(int32_t min_log_level) noexcept;

        [[nodiscard]] auto logName() const noexcept -> const std::string&;
        void logName(const std::string& log_name) noexcept;

        [[nodiscard]] auto logToStderr() const noexcept -> bool;
        void logToStderr(bool log_to_stderr) noexcept;

        [[nodiscard]] auto customLogFormat() const noexcept -> bool;
        void customLogFormat(bool custom_log_format) noexcept;

        void deserializeFromYamlFile(const std::filesystem::path& path) override;

        [[nodiscard]] auto operator==(const GLogParam& other) const noexcept -> bool;
        [[nodiscard]] auto operator!=(const GLogParam& other) const noexcept -> bool;

    private:
        void parseFromNode(const YAML::Node& node) noexcept;

        int32_t min_log_level_{};
        std::string log_name_{};
        bool log_to_stderr_{};
        bool custom_log_format_{false};
    };
}

/// @brief YAML serialization specialization for GLogParam.
/// Provides methods to encode and decode GLogParam to/from YAML nodes.
template <>
struct YAML::convert<cppforge::glog::param::GLogParam>
{
    /// @brief Decode a YAML node into a GLogParam object.
    /// @param node The YAML node containing the configuration data.
    /// @param rhs The GLogParam object to populate.
    /// @return True if decoding was successful.
    static bool decode(const Node& node, cppforge::glog::param::GLogParam& rhs);

    /// @brief Encode a GLogParam object into a YAML node.
    /// @param rhs The GLogParam object to encode.
    /// @return A YAML node containing the configuration data.
    static Node encode(const cppforge::glog::param::GLogParam& rhs);
};