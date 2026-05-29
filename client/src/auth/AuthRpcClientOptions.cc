/**
 * @file AuthRpcClientOptions.cc
 * @brief Implementation of authentication RPC client configuration options
 * @details This file contains the implementation of AuthRpcClientOptions class methods,
 *          including builder pattern, YAML deserialization, and parameter validation.
 */

#include "AuthRpcClientOptions.hpp"

#include <glog/logging.h>
#include <chrono>  // C++20

namespace client_app::auth
{
    AuthRpcClientOptions::AuthRpcClientOptions(const int32_t keepalive_time_ms, const int32_t keepalive_timeout_ms, const int32_t keepalive_permit_without_calls, std::string server_address) noexcept : keepalive_time_ms_(keepalive_time_ms), keepalive_timeout_ms_(keepalive_timeout_ms), keepalive_permit_without_calls_(keepalive_permit_without_calls), server_address_(std::move(server_address))
    {
        validate(); // Validate parameters after construction
    }

    AuthRpcClientOptions::Builder& AuthRpcClientOptions::Builder::keepaliveTimeMs(const int32_t value) noexcept
    {
        keepalive_time_ms_ = value;
        return *this;
    }

    AuthRpcClientOptions::Builder& AuthRpcClientOptions::Builder::keepaliveTimeoutMs(const int32_t value) noexcept
    {
        keepalive_timeout_ms_ = value;
        return *this;
    }

    AuthRpcClientOptions::Builder& AuthRpcClientOptions::Builder::keepalivePermitWithoutCalls(const int32_t value) noexcept
    {
        keepalive_permit_without_calls_ = value;
        return *this;
    }

    AuthRpcClientOptions::Builder& AuthRpcClientOptions::Builder::serverAddress(const std::string& value) noexcept
    {
        server_address_ = value;
        return *this;
    }

    AuthRpcClientOptions AuthRpcClientOptions::Builder::build() const
    {
        return AuthRpcClientOptions{keepalive_time_ms_, keepalive_timeout_ms_, keepalive_permit_without_calls_, server_address_};
    }

    AuthRpcClientOptions::Builder AuthRpcClientOptions::builder()
    {
        return {};
    }

    int32_t AuthRpcClientOptions::keepaliveTimeMs() const noexcept
    {
        return keepalive_time_ms_;
    }

    void AuthRpcClientOptions::keepaliveTimeMs(const int32_t value) noexcept
    {
        keepalive_time_ms_ = value;
        validate(); // Validate after setting new value
    }

    int32_t AuthRpcClientOptions::keepaliveTimeoutMs() const noexcept
    {
        return keepalive_timeout_ms_;
    }

    void AuthRpcClientOptions::keepaliveTimeoutMs(const int32_t value) noexcept
    {
        keepalive_timeout_ms_ = value;
        validate(); // Validate after setting new value
    }

    int32_t AuthRpcClientOptions::keepalivePermitWithoutCalls() const noexcept
    {
        return keepalive_permit_without_calls_;
    }

    void AuthRpcClientOptions::keepalivePermitWithoutCalls(const int32_t value) noexcept
    {
        keepalive_permit_without_calls_ = value;
        validate(); // Validate after setting new value
    }

    // ReSharper disable once CppDFAConstantFunctionResult
    const std::string& AuthRpcClientOptions::serverAddress() const noexcept
    {
        return server_address_;
    }

    void AuthRpcClientOptions::serverAddress(const std::string& value) noexcept
    {
        server_address_ = value;
        validate(); // Validate after setting new value
    }

    void AuthRpcClientOptions::deserializedFromYamlFile(const std::filesystem::path& path)
    {
        if (!std::filesystem::exists(path))
        {
            throw std::runtime_error("Configuration file does not exist: " + path.string());
        }

        try
        {
            const YAML::Node root = common::filesystem::YamlToolkit::read(path.string());
            const YAML::Node grpcNode = common::filesystem::YamlToolkit::getNodeOrRoot(root, "grpc");

            if (const auto keepaliveTimeMsNode = grpcNode["keepaliveTimeMs"]; keepaliveTimeMsNode)
            {
                keepalive_time_ms_ = keepaliveTimeMsNode.as<int32_t>();
            }
            if (const auto keepaliveTimeoutMsNode = grpcNode["keepaliveTimeoutMs"]; keepaliveTimeoutMsNode)
            {
                keepalive_timeout_ms_ = keepaliveTimeoutMsNode.as<int32_t>();
            }
            if (const auto keepalivePermitWithoutCallsNode = grpcNode["keepalivePermitWithoutCalls"]; keepalivePermitWithoutCallsNode)
            {
                keepalive_permit_without_calls_ = keepalivePermitWithoutCallsNode.as<int32_t>();
            }
            if (const auto serverAddressNode = grpcNode["serverAddress"]; serverAddressNode)
            {
                server_address_ = serverAddressNode.as<std::string>();
            }
        }
        catch (const YAML::Exception& e)
        {
            throw std::runtime_error("Failed to parse YAML file '" + path.string() + "': " + e.what());
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("Error processing configuration file '" + path.string() + "': " + e.what());
        }

        validate(); // Validate after loading from YAML
    }

    void AuthRpcClientOptions::validate() const noexcept
    {
        // Validate keepalive time (should be positive)
        LOG_IF(WARNING, keepalive_time_ms_ <= 0) << "Invalid keepalive time: " << keepalive_time_ms_ << "ms. Using default value of 30000ms.";

        // Validate keepalive timeout (should be positive)
        LOG_IF(WARNING, keepalive_timeout_ms_ <= 0) << "Invalid keepalive timeout: " << keepalive_timeout_ms_ << "ms. Using default value of 5000ms.";

        // Validate keepalive permit without calls (should be 0 or 1)
        LOG_IF(WARNING, keepalive_permit_without_calls_ != 0 && keepalive_permit_without_calls_ != 1) << "Invalid keepalive permit without calls: " << keepalive_permit_without_calls_ << ". Valid values are 0 or 1. Using default value of 1.";

        // Check for potentially problematic combinations
        LOG_IF(WARNING, keepalive_time_ms_ > 0 && keepalive_time_ms_ < 1000) << "Keepalive time is set to a very short interval (" << keepalive_time_ms_ << "ms). This may cause excessive network traffic.";

        LOG_IF(WARNING, keepalive_timeout_ms_ > 0 && keepalive_timeout_ms_ > keepalive_time_ms_) << "Keepalive timeout (" << keepalive_timeout_ms_ << "ms) is greater than keepalive time (" << keepalive_time_ms_ << "ms). This may lead to unexpected connection issues.";

        // Validate server address
        LOG_IF(WARNING, server_address_.empty()) << "Server address is empty. Using default value localhost:50051.";
    }
}

bool YAML::convert<client_app::auth::AuthRpcClientOptions>::decode(const YAML::Node& node, client_app::auth::AuthRpcClientOptions& rhs)
{
    if (const auto keepaliveTimeMsNode = node["keepaliveTimeMs"]; keepaliveTimeMsNode)
    {
        rhs.keepaliveTimeMs(keepaliveTimeMsNode.as<int32_t>());
    }
    if (const auto keepaliveTimeoutMsNode = node["keepaliveTimeoutMs"]; keepaliveTimeoutMsNode)
    {
        rhs.keepaliveTimeoutMs(keepaliveTimeoutMsNode.as<int32_t>());
    }
    if (const auto keepalivePermitWithoutCallsNode = node["keepalivePermitWithoutCalls"]; keepalivePermitWithoutCallsNode)
    {
        rhs.keepalivePermitWithoutCalls(keepalivePermitWithoutCallsNode.as<int32_t>());
    }
    if (const auto serverAddressNode = node["serverAddress"]; serverAddressNode)
    {
        rhs.serverAddress(serverAddressNode.as<std::string>());
    }
    return true;
}

YAML::Node YAML::convert<client_app::auth::AuthRpcClientOptions>::encode(const client_app::auth::AuthRpcClientOptions& rhs)
{
    YAML::Node node;
    node["keepaliveTimeMs"] = rhs.keepaliveTimeMs();
    node["keepaliveTimeoutMs"] = rhs.keepaliveTimeoutMs();
    node["keepalivePermitWithoutCalls"] = rhs.keepalivePermitWithoutCalls();
    node["serverAddress"] = rhs.serverAddress();
    return node;
}
