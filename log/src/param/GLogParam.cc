/**
 * @file GLogParam.cc
 * @brief Implementation of glog configuration parameters
 * @details This file contains the implementation of GLogParam class methods,
 *          including parameter accessors, YAML deserialization, and equality comparison.
 */

#include <cppforge/glog/param/GLogParam.hpp>

#include <fmt/format.h>
#include <yaml-cpp/yaml.h>

namespace cppforge::glog::param
{
    GLogParam::GLogParam(const int32_t min_log_level, std::string log_name, const bool log_to_stderr,
                         const bool custom_log_format)
        : min_log_level_(min_log_level), log_name_(std::move(log_name)), log_to_stderr_(log_to_stderr),
          custom_log_format_(custom_log_format)
    {
    }

    auto GLogParam::minLogLevel() const noexcept -> int32_t
    {
        return min_log_level_;
    }

    void GLogParam::minLogLevel(const int32_t min_log_level) noexcept
    {
        min_log_level_ = min_log_level;
    }

    auto GLogParam::logName() const noexcept -> const std::string&
    {
        return log_name_;
    }

    void GLogParam::logName(const std::string& log_name) noexcept
    {
        log_name_ = log_name;
    }

    auto GLogParam::logToStderr() const noexcept -> bool
    {
        return log_to_stderr_;
    }

    void GLogParam::logToStderr(const bool log_to_stderr) noexcept
    {
        log_to_stderr_ = log_to_stderr;
    }

    auto GLogParam::customLogFormat() const noexcept -> bool
    {
        return custom_log_format_;
    }

    void GLogParam::customLogFormat(const bool custom_log_format) noexcept
    {
        custom_log_format_ = custom_log_format;
    }

    void GLogParam::parseFromNode(const YAML::Node& node) noexcept
    {
        if (node["minLogLevel"])
            min_log_level_ = node["minLogLevel"].as<int32_t>();
        if (node["logName"])
            log_name_ = node["logName"].as<std::string>();
        if (node["logToStderr"])
            log_to_stderr_ = node["logToStderr"].as<bool>();
        if (node["customLogFormat"])
            custom_log_format_ = node["customLogFormat"].as<bool>();
    }

    void GLogParam::deserializeFromYamlFile(const std::filesystem::path& path)
    {
        if (!std::filesystem::exists(path))
        {
            throw std::runtime_error(fmt::format("Configuration file does not exist: {}", path.string()));
        }

        const YAML::Node node = YAML::LoadFile(path.string());
        if (node["glog"])
        {
            parseFromNode(node["glog"]);
        }
        else
        {
            parseFromNode(node);
        }
    }

    auto GLogParam::operator==(const GLogParam& other) const noexcept -> bool
    {
        return min_log_level_ == other.min_log_level_
            && log_name_ == other.log_name_
            && log_to_stderr_ == other.log_to_stderr_
            && custom_log_format_ == other.custom_log_format_;
    }

    auto GLogParam::operator!=(const GLogParam& other) const noexcept -> bool
    {
        return !(*this == other);
    }
}

bool YAML::convert<cppforge::glog::param::GLogParam>::decode(const Node& node, cppforge::glog::param::GLogParam& rhs)
{
    if (!node.IsMap())
    {
        return false;
    }

    if (node["minLogLevel"])
    {
        rhs.minLogLevel(node["minLogLevel"].as<int32_t>());
    }
    if (node["logName"])
    {
        rhs.logName(node["logName"].as<std::string>());
    }
    if (node["logToStderr"])
    {
        rhs.logToStderr(node["logToStderr"].as<bool>());
    }
    if (node["customLogFormat"])
    {
        rhs.customLogFormat(node["customLogFormat"].as<bool>());
    }
    return true;
}

YAML::Node YAML::convert<cppforge::glog::param::GLogParam>::encode(const cppforge::glog::param::GLogParam& rhs)
{
    Node node;
    node["minLogLevel"] = rhs.minLogLevel();
    node["logName"] = rhs.logName();
    node["logToStderr"] = rhs.logToStderr();
    node["customLogFormat"] = rhs.customLogFormat();
    return node;
}
