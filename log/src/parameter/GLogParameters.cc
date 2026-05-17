/**
 * @file GLogParameters.cc
 * @brief Implementation of glog configuration parameters
 * @details This file contains the implementation of GLogParameters class methods,
 *          including parameter accessors, YAML deserialization, and equality comparison.
 */

#include "GLogParameters.hpp"

#include <yaml-cpp/yaml.h>
#include <fmt/format.h>

namespace glog::parameter {
GLogParameters::GLogParameters(const int32_t min_log_level, std::string log_name, const bool log_to_stderr) : min_log_level_(min_log_level), log_name_(std::move(log_name)), log_to_stderr_(log_to_stderr) {
}

auto GLogParameters::minLogLevel() const noexcept -> int32_t {
    return min_log_level_;
}

auto GLogParameters::minLogLevel(const int32_t min_log_level) noexcept -> void {
    min_log_level_ = min_log_level;
}

auto GLogParameters::logName() const noexcept -> std::string {
    return log_name_;
}

auto GLogParameters::logName(const std::string& log_name) -> void {
    log_name_ = log_name;
}

auto GLogParameters::logToStderr() const noexcept -> bool {
    return log_to_stderr_;
}

auto GLogParameters::logToStderr(const bool log_to_stderr) noexcept -> void {
    log_to_stderr_ = log_to_stderr;
}

auto GLogParameters::customLogFormat() const noexcept -> bool {
    return custom_log_format_;
}

auto GLogParameters::customLogFormat(const bool custom_log_format) noexcept -> void {
    custom_log_format_ = custom_log_format;
}

auto GLogParameters::deserializedFromYamlFile(const std::filesystem::path& path) -> void {
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error(fmt::format("Configuration file does not exist: {}", path.string()));
    }

    try {
        if (const YAML::Node node = YAML::LoadFile(path.string()); node["glog"]) {
            const YAML::Node& glog_node = node["glog"];
            if (glog_node["minLogLevel"]) {
                min_log_level_ = glog_node["minLogLevel"].as<int32_t>();
            }
            if (glog_node["logName"]) {
                log_name_ = glog_node["logName"].as<std::string>();
            }
            if (glog_node["logToStderr"]) {
                log_to_stderr_ = glog_node["logToStderr"].as<bool>();
            }
            if (glog_node["customLogFormat"]) {
                custom_log_format_ = glog_node["customLogFormat"].as<bool>();
            }
        } else {
            // If there's no "glog" section, try to parse the fields directly from root
            if (node["minLogLevel"]) {
                min_log_level_ = node["minLogLevel"].as<int32_t>();
            }
            if (node["logName"]) {
                log_name_ = node["logName"].as<std::string>();
            }
            if (node["logToStderr"]) {
                log_to_stderr_ = node["logToStderr"].as<bool>();
            }
            if (node["customLogFormat"]) {
                custom_log_format_ = node["customLogFormat"].as<bool>();
            }
        }
    } catch (const YAML::Exception& e) {
        throw std::runtime_error(fmt::format("Failed to parse YAML file '{}': {}", path.string(), e.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error(fmt::format("Error processing configuration file '{}': {}", path.string(), e.what()));
    }
}

auto GLogParameters::operator==(const GLogParameters& other) const noexcept -> bool {
    return min_log_level_ == other.min_log_level_ && log_name_ == other.log_name_ && log_to_stderr_ == other.log_to_stderr_ && custom_log_format_ == other.custom_log_format_;
}

auto GLogParameters::operator!=(const GLogParameters& other) const noexcept -> bool {
    return !(*this == other);
}
}

auto YAML::convert<glog::parameter::GLogParameters>::decode(const YAML::Node& node, glog::parameter::GLogParameters& rhs) -> bool {
    if (!node.IsMap()) {
        return false;
    }

    if (node["minLogLevel"]) {
        rhs.minLogLevel(node["minLogLevel"].as<int32_t>());
    }
    if (node["logName"]) {
        rhs.logName(node["logName"].as<std::string>());
    }
    if (node["logToStderr"]) {
        rhs.logToStderr(node["logToStderr"].as<bool>());
    }
    if (node["customLogFormat"]) {
        rhs.customLogFormat(node["customLogFormat"].as<bool>());
    }
    return true;
}

auto YAML::convert<glog::parameter::GLogParameters>::encode(const glog::parameter::GLogParameters& rhs) -> YAML::Node {
    YAML::Node node;
    node["minLogLevel"] = rhs.minLogLevel();
    node["logName"] = rhs.logName();
    node["logToStderr"] = rhs.logToStderr();
    node["customLogFormat"] = rhs.customLogFormat();
    return node;
}