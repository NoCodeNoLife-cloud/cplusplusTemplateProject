/**
 * @file GLogConfigurator.cc
 * @brief Implementation of glog configuration manager
 * @details This file contains the implementation of GLogConfigurator class methods,
 *          including YAML-based configuration loading, glog initialization,
 *          custom log format installation, and resource cleanup.
 */

#include "GLogConfigurator.hpp"

#include <glog/logging.h>
#include <iostream>
#include <thread>
#include <fmt/format.h>

#include "formatter/CustomGlogPrefixFormatter.hpp"

namespace glog::config
{
    // Static variable to hold the custom log sink for cleanup
    static std::unique_ptr<google::LogSink> static_custom_log_sink_;

    GLogConfigurator::GLogConfigurator(std::string glog_yaml_path) : glog_yaml_path_(std::move(glog_yaml_path))
    {
        config_.deserializedFromYamlFile(glog_yaml_path_);
    }

    void GLogConfigurator::execute() const
    {
        doConfig(config_);
        if (const auto result = std::atexit(clean); result != 0)
        {
            throw std::runtime_error(fmt::format("Failed to register cleanup function! Error code: {}", result));
        }
        DLOG(INFO) << "glog configured...";
    }

    auto GLogConfigurator::getConfig() const noexcept -> const parameter::GLogParameters&
    {
        return config_;
    }

    void GLogConfigurator::updateConfig(const parameter::GLogParameters& config) noexcept
    {
        config_ = config;
    }

    void GLogConfigurator::doConfig(const parameter::GLogParameters& config) noexcept
    {
        google::InitGoogleLogging(config.logName().c_str());
        FLAGS_minloglevel = config.minLogLevel();
        FLAGS_logtostderr = config.logToStderr();
        FLAGS_alsologtostderr = false;
        FLAGS_log_dir = "";

        // Apply custom log format if enabled
        if (config.customLogFormat())
        {
            google::InstallPrefixFormatter(&formatter::CustomGlogPrefixFormatter::MyPrefixFormatter);
            DLOG(INFO) << "Custom log format enabled...";
        }
    }

    void GLogConfigurator::clean() noexcept
    {
        if (static_custom_log_sink_)
        {
            google::RemoveLogSink(static_custom_log_sink_.get());
            static_custom_log_sink_.reset();
        }
        google::ShutdownGoogleLogging();
    }
}
