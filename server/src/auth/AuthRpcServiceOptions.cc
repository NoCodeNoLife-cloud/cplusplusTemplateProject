/**
 * @file AuthRpcServiceOptions.cc
 * @brief Implementation of gRPC service configuration options
 * @details This file contains the implementation of AuthRpcServiceOptions class methods,
 *          including builder pattern, YAML deserialization, and parameter validation
 *          for gRPC server configuration.
 */

#include "AuthRpcServiceOptions.hpp"

#include <functional>
#include <utility>
#include <yaml-cpp/yaml.h>
#include <glog/logging.h>
#include <fmt/format.h>
#include "src/filesystem/type/YamlToolkit.hpp"

namespace app_server::auth
{
    AuthRpcServiceOptions::AuthRpcServiceOptions() = default;

    AuthRpcServiceOptions::AuthRpcServiceOptions(const int32_t max_connection_idle_ms, const int32_t max_connection_age_ms, const int32_t max_connection_age_grace_ms, const int32_t keepalive_time_ms, const int32_t keepalive_timeout_ms, const int32_t keepalive_permit_without_calls, std::string server_address) : max_connection_idle_ms_(max_connection_idle_ms), max_connection_age_ms_(max_connection_age_ms), max_connection_age_grace_ms_(max_connection_age_grace_ms), keepalive_time_ms_(keepalive_time_ms), keepalive_timeout_ms_(keepalive_timeout_ms),
                                                                                                                                                                                                                                                                                                                        keepalive_permit_without_calls_(keepalive_permit_without_calls), server_address_(std::move(server_address))
    {
        validateParameters();
    }

    int32_t AuthRpcServiceOptions::maxConnectionIdleMs() const noexcept
    {
        return max_connection_idle_ms_;
    }

    void AuthRpcServiceOptions::maxConnectionIdleMs(const int32_t value) noexcept
    {
        max_connection_idle_ms_ = value;
    }

    int32_t AuthRpcServiceOptions::maxConnectionAgeMs() const noexcept
    {
        return max_connection_age_ms_;
    }

    void AuthRpcServiceOptions::maxConnectionAgeMs(const int32_t value) noexcept
    {
        max_connection_age_ms_ = value;
    }

    int32_t AuthRpcServiceOptions::maxConnectionAgeGraceMs() const noexcept
    {
        return max_connection_age_grace_ms_;
    }

    void AuthRpcServiceOptions::maxConnectionAgeGraceMs(const int32_t value) noexcept
    {
        max_connection_age_grace_ms_ = value;
    }

    int32_t AuthRpcServiceOptions::keepaliveTimeMs() const noexcept
    {
        return keepalive_time_ms_;
    }

    void AuthRpcServiceOptions::keepaliveTimeMs(const int32_t value) noexcept
    {
        keepalive_time_ms_ = value;
    }

    int32_t AuthRpcServiceOptions::keepaliveTimeoutMs() const noexcept
    {
        return keepalive_timeout_ms_;
    }

    void AuthRpcServiceOptions::keepaliveTimeoutMs(const int32_t value) noexcept
    {
        keepalive_timeout_ms_ = value;
    }

    int32_t AuthRpcServiceOptions::keepalivePermitWithoutCalls() const noexcept
    {
        return keepalive_permit_without_calls_;
    }

    void AuthRpcServiceOptions::keepalivePermitWithoutCalls(const int32_t value) noexcept
    {
        keepalive_permit_without_calls_ = value;
    }

    // ReSharper disable once CppDFAConstantFunctionResult
    const std::string& AuthRpcServiceOptions::serverAddress() const noexcept
    {
        return server_address_;
    }

    void AuthRpcServiceOptions::serverAddress(const std::string& value)
    {
        server_address_ = value;
    }

    void AuthRpcServiceOptions::deserializedFromYamlFile(const std::filesystem::path& path)
    {
        if (!std::filesystem::exists(path))
        {
            const std::string error_msg = fmt::format("Configuration file does not exist: {}", path.string());
            LOG(ERROR) << error_msg;
            throw std::runtime_error(error_msg);
        }

        try
        {
            const YAML::Node root = common::filesystem::YamlToolkit::read(path.string());
            const YAML::Node grpcNode = common::filesystem::YamlToolkit::getNodeOrRoot(root, "grpc");

            // Table-driven configuration loading for gRPC parameters
            const std::vector<std::pair<std::string, std::function<void()>>> config_handlers = {
                {
                    "maxConnectionIdleMs", [&]()
                    {
                        max_connection_idle_ms_ = grpcNode["maxConnectionIdleMs"].as<int32_t>();
                    }
                },
                {
                    "maxConnectionAgeMs", [&]()
                    {
                        max_connection_age_ms_ = grpcNode["maxConnectionAgeMs"].as<int32_t>();
                    }
                },
                {
                    "maxConnectionAgeGraceMs", [&]()
                    {
                        max_connection_age_grace_ms_ = grpcNode["maxConnectionAgeGraceMs"].as<int32_t>();
                    }
                },
                {
                    "keepaliveTimeMs", [&]()
                    {
                        keepalive_time_ms_ = grpcNode["keepaliveTimeMs"].as<int32_t>();
                    }
                },
                {
                    "keepaliveTimeoutMs", [&]()
                    {
                        keepalive_timeout_ms_ = grpcNode["keepaliveTimeoutMs"].as<int32_t>();
                    }
                },
                {
                    "keepalivePermitWithoutCalls", [&]()
                    {
                        keepalive_permit_without_calls_ = grpcNode["keepalivePermitWithoutCalls"].as<int32_t>();
                    }
                },
                {
                    "serverAddress", [&]()
                    {
                        server_address_ = grpcNode["serverAddress"].as<std::string>();
                    }
                }
            };

            for (const auto& [key, handler] : config_handlers)
            {
                if (grpcNode[key])
                {
                    handler();
                }
            }
        }
        catch (const YAML::Exception& e)
        {
            const std::string error_msg = fmt::format("Failed to parse YAML file '{}': {}", path.string(), e.what());
            LOG(ERROR) << error_msg;
            throw std::runtime_error(std::move(error_msg));
        }
        catch (const std::exception& e)
        {
            const std::string error_msg = fmt::format("Error processing configuration file '{}': {}", path.string(), e.what());
            LOG(ERROR) << error_msg;
            throw std::runtime_error(std::move(error_msg));
        }

        validateParameters();
    }

    void AuthRpcServiceOptions::validateParameters() const
    {
        // Table-driven validation for numeric parameter checks
        const std::vector<std::tuple<bool, std::string, const char*>> numeric_validations = {
            std::make_tuple(max_connection_idle_ms_ <= 0, fmt::format("Invalid max connection idle time: {}ms. Value must be greater than 0.", max_connection_idle_ms_), "max_connection_idle_ms_"), std::make_tuple(max_connection_age_ms_ <= 0, fmt::format("Invalid max connection age: {}ms. Value must be greater than 0.", max_connection_age_ms_), "max_connection_age_ms_"), std::make_tuple(max_connection_age_grace_ms_ < 0, fmt::format("Invalid max connection age grace period: {}ms. Value must be greater than or equal to 0.", max_connection_age_grace_ms_), "max_connection_age_grace_ms_"),
            std::make_tuple(keepalive_time_ms_ <= 0, fmt::format("Invalid keepalive time: {}ms. Value must be greater than 0.", keepalive_time_ms_), "keepalive_time_ms_"), std::make_tuple(keepalive_timeout_ms_ <= 0, fmt::format("Invalid keepalive timeout: {}ms. Value must be greater than 0.", keepalive_timeout_ms_), "keepalive_timeout_ms_"), std::make_tuple(keepalive_permit_without_calls_ != 0 && keepalive_permit_without_calls_ != 1, fmt::format("Invalid keepalive permit without calls: {}. Valid values are 0 or 1.", keepalive_permit_without_calls_), "keepalive_permit_without_calls_"),
            std::make_tuple(server_address_.empty(), fmt::format("Server address is empty."), "server_address_")
        };

        // Execute numeric validations
        for (const auto& [condition, error_message, param_name] : numeric_validations)
        {
            if (condition)
            {
                LOG(ERROR) << error_message;
                throw std::invalid_argument(error_message);
            }
        }

        // Table-driven validation for warning conditions
        const std::vector<std::tuple<bool, std::string>> warning_checks = {
            std::make_tuple(max_connection_idle_ms_ > 0 && max_connection_idle_ms_ < 1000, fmt::format("Max connection idle time is set to a very short interval ({}ms). This may cause excessive connection churn.", max_connection_idle_ms_)), std::make_tuple(keepalive_time_ms_ > 0 && keepalive_time_ms_ < 1000, fmt::format("Keepalive time is set to a very short interval ({}ms). This may cause excessive network traffic.", keepalive_time_ms_)),
            std::make_tuple(keepalive_timeout_ms_ > 0 && keepalive_timeout_ms_ > keepalive_time_ms_, fmt::format("Keepalive timeout ({}ms) is greater than keepalive time ({}ms). This may lead to unexpected connection issues.", keepalive_timeout_ms_, keepalive_time_ms_)), std::make_tuple(max_connection_age_ms_ > 0 && max_connection_idle_ms_ > 0 && max_connection_age_ms_ < max_connection_idle_ms_, fmt::format("Max connection age ({}ms) is less than max connection idle time ({}ms). This may lead to unexpected connection behavior.", max_connection_age_ms_, max_connection_idle_ms_))
        };

        // Execute warning checks
        for (const auto& [condition, warning_message] : warning_checks)
        {
            if (condition)
            {
                LOG(WARNING) << warning_message;
            }
        }
    }

    AuthRpcServiceOptions::Builder& AuthRpcServiceOptions::Builder::maxConnectionIdleMs(const int32_t value) noexcept
    {
        max_connection_idle_ms_ = value;
        return *this;
    }

    AuthRpcServiceOptions::Builder& AuthRpcServiceOptions::Builder::maxConnectionAgeMs(const int32_t value) noexcept
    {
        max_connection_age_ms_ = value;
        return *this;
    }

    AuthRpcServiceOptions::Builder& AuthRpcServiceOptions::Builder::maxConnectionAgeGraceMs(const int32_t value) noexcept
    {
        max_connection_age_grace_ms_ = value;
        return *this;
    }

    AuthRpcServiceOptions::Builder& AuthRpcServiceOptions::Builder::keepaliveTimeMs(const int32_t value) noexcept
    {
        keepalive_time_ms_ = value;
        return *this;
    }

    AuthRpcServiceOptions::Builder& AuthRpcServiceOptions::Builder::keepaliveTimeoutMs(const int32_t value) noexcept
    {
        keepalive_timeout_ms_ = value;
        return *this;
    }

    AuthRpcServiceOptions::Builder& AuthRpcServiceOptions::Builder::keepalivePermitWithoutCalls(const int32_t value) noexcept
    {
        keepalive_permit_without_calls_ = value;
        return *this;
    }

    AuthRpcServiceOptions::Builder& AuthRpcServiceOptions::Builder::serverAddress(const std::string& value)
    {
        server_address_ = value;
        return *this;
    }

    AuthRpcServiceOptions AuthRpcServiceOptions::Builder::build() const
    {
        AuthRpcServiceOptions options{max_connection_idle_ms_, max_connection_age_ms_, max_connection_age_grace_ms_, keepalive_time_ms_, keepalive_timeout_ms_, keepalive_permit_without_calls_, server_address_};
        options.validateParameters();
        return options;
    }

    AuthRpcServiceOptions::Builder AuthRpcServiceOptions::builder()
    {
        return Builder{};
    }
}

bool YAML::convert<app_server::auth::AuthRpcServiceOptions>::decode(const Node& node, app_server::auth::AuthRpcServiceOptions& rhs)
{
    const std::vector<std::pair<std::string, std::function<void()>>> config_handlers = {
        {
            "maxConnectionIdleMs", [&]()
            {
                rhs.maxConnectionIdleMs(node["maxConnectionIdleMs"].as<int32_t>());
            }
        },
        {
            "maxConnectionAgeMs", [&]()
            {
                rhs.maxConnectionAgeMs(node["maxConnectionAgeMs"].as<int32_t>());
            }
        },
        {
            "maxConnectionAgeGraceMs", [&]()
            {
                rhs.maxConnectionAgeGraceMs(node["maxConnectionAgeGraceMs"].as<int32_t>());
            }
        },
        {
            "keepaliveTimeMs", [&]()
            {
                rhs.keepaliveTimeMs(node["keepaliveTimeMs"].as<int32_t>());
            }
        },
        {
            "keepaliveTimeoutMs", [&]()
            {
                rhs.keepaliveTimeoutMs(node["keepaliveTimeoutMs"].as<int32_t>());
            }
        },
        {
            "keepalivePermitWithoutCalls", [&]()
            {
                rhs.keepalivePermitWithoutCalls(node["keepalivePermitWithoutCalls"].as<int32_t>());
            }
        },
        {
            "serverAddress", [&]()
            {
                rhs.serverAddress(node["serverAddress"].as<std::string>());
            }
        }
    };

    for (const auto& [key, handler] : config_handlers)
    {
        if (node[key])
        {
            handler();
        }
    }
    return true;
}

Node YAML::convert<app_server::auth::AuthRpcServiceOptions>::encode(const app_server::auth::AuthRpcServiceOptions& rhs)
{
    Node node;
    node["maxConnectionIdleMs"] = rhs.maxConnectionIdleMs();
    node["maxConnectionAgeMs"] = rhs.maxConnectionAgeMs();
    node["maxConnectionAgeGraceMs"] = rhs.maxConnectionAgeGraceMs();
    node["keepaliveTimeMs"] = rhs.keepaliveTimeMs();
    node["keepaliveTimeoutMs"] = rhs.keepaliveTimeoutMs();
    node["keepalivePermitWithoutCalls"] = rhs.keepalivePermitWithoutCalls();
    node["serverAddress"] = rhs.serverAddress();
    return node;
}
