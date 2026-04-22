#include "GLogConfigurator.hpp"

#include <glog/logging.h>
#include <iostream>
#include <thread>
#include <fmt/format.h>

#include "formatter/CustomGlogPrefixFormatter.hpp"

namespace glog::config {
    // Static variable to hold the custom log sink for cleanup
    static std::unique_ptr<google::LogSink> static_custom_log_sink_;

    GLogConfigurator::GLogConfigurator(std::string glog_yaml_path) noexcept : glog_yaml_path_(std::move(glog_yaml_path)) {
        config_.deserializedFromYamlFile(glog_yaml_path_);
    }

    auto GLogConfigurator::execute() const -> void {
        doConfig(config_);
        if (const auto result = std::atexit(clean); result != 0) {
            throw std::runtime_error(fmt::format("Failed to register cleanup function! Error code: {}", result));
        }
        DLOG(INFO) << "glog configured...";
    }

    auto GLogConfigurator::getConfig() const noexcept -> const parameter::GLogParameters & {
        return config_;
    }

    auto GLogConfigurator::updateConfig(const parameter::GLogParameters &config) noexcept -> void {
        config_ = config;
    }

    auto GLogConfigurator::doConfig(const parameter::GLogParameters &config) noexcept -> void {
        google::InitGoogleLogging(config.logName().c_str());
        FLAGS_minloglevel = config.minLogLevel();
        FLAGS_logtostderr = config.logToStderr();
        FLAGS_alsologtostderr = false;
        FLAGS_log_dir = "";

        // Apply custom log format if enabled
        if (config.customLogFormat()) {
            google::InstallPrefixFormatter(&formatter::CustomGlogPrefixFormatter::MyPrefixFormatter);
            DLOG(INFO) << "Custom log format enabled...";
        }
    }

    auto GLogConfigurator::clean() noexcept -> void {
        if (static_custom_log_sink_) {
            google::RemoveLogSink(static_custom_log_sink_.get());
            static_custom_log_sink_.reset();
        }
        google::ShutdownGoogleLogging();
    }
}
