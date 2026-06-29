/**
 * @file GLogConfigurator.cc
 * @brief Implementation of glog configuration manager
 * @details This file contains the implementation of GLogConfigurator class methods,
 *          including YAML-based configuration loading, glog initialization,
 *          custom log format installation, and resource cleanup.
 */

#include <cppforge/glog/config/GLogConfigurator.hpp>

#include <thread>
#include <fmt/format.h>
#include <glog/logging.h>

#include <cppforge/glog/formatter/PrefixFormatter.hpp>

namespace cppforge::glog::config
{
    std::once_flag GLogConfigurator::glog_init_flag_;
    std::atomic<bool> GLogConfigurator::glog_initialized_{false};

    GLogConfigurator::GLogConfigurator(std::string glog_yaml_path) : glog_yaml_path_(std::move(glog_yaml_path))
    {
        config_.deserializeFromYamlFile(glog_yaml_path_);
    }

    void GLogConfigurator::execute() const
    {
        std::call_once(glog_init_flag_, [this]
        {
            doConfig(config_);
            if (const auto result = std::atexit(clean); result != 0)
            {
                google::ShutdownGoogleLogging();
                throw std::runtime_error(fmt::format("Failed to register cleanup function! Error code: {}", result));
            }
            glog_initialized_.store(true, std::memory_order_release);
        });
        LOG(INFO) << "glog configured...";
    }

    auto GLogConfigurator::getConfig() const noexcept -> const param::GLogParam&
    {
        return config_;
    }

    void GLogConfigurator::updateConfig(const param::GLogParam& config) noexcept
    {
        config_ = config;
    }

    void GLogConfigurator::doConfig(const param::GLogParam& config) noexcept
    {
        google::InitGoogleLogging(config.logName().c_str());
        FLAGS_minloglevel = config.minLogLevel();
        FLAGS_logtostderr = config.logToStderr();
        FLAGS_alsologtostderr = false;
        FLAGS_log_dir = "";

        if (config.customLogFormat())
        {
            google::InstallPrefixFormatter(&formatter::PrefixFormatter::MyPrefixFormatter);
        }
    }

    void GLogConfigurator::clean() noexcept
    {
        google::ShutdownGoogleLogging();
        glog_initialized_.store(false, std::memory_order_release);
    }

    auto GLogConfigurator::isInitialized() noexcept -> bool
    {
        return glog_initialized_.load(std::memory_order_acquire);
    }
}
