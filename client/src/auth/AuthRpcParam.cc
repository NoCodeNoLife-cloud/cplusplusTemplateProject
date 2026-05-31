/**
 * @file AuthRpcParam.cc
 * @brief Implementation of authentication RPC client configuration options
 * @details This file contains the implementation of AuthRpcClientOptions class methods,
 *          including builder pattern, YAML deserialization, and parameter validation.
 */

#include "AuthRpcParam.hpp"

#include <glog/logging.h>
#include <chrono>  // C++20

namespace client_app::auth
{
    AuthRpcParam::AuthRpcParam(const int32_t keepalive_time_ms, const int32_t keepalive_timeout_ms, const int32_t keepalive_permit_without_calls, std::string server_address) : keepalive_time_ms_(keepalive_time_ms), keepalive_timeout_ms_(keepalive_timeout_ms), keepalive_permit_without_calls_(keepalive_permit_without_calls), server_address_(std::move(server_address))
    {
    }

    int32_t AuthRpcParam::keepaliveTimeMs() const
    {
        return keepalive_time_ms_;
    }

    void AuthRpcParam::keepaliveTimeMs(const int32_t value)
    {
        keepalive_time_ms_ = value;
        validate(); // Validate after setting new value
    }

    int32_t AuthRpcParam::keepaliveTimeoutMs() const
    {
        return keepalive_timeout_ms_;
    }

    void AuthRpcParam::keepaliveTimeoutMs(const int32_t value)
    {
        keepalive_timeout_ms_ = value;
        validate(); // Validate after setting new value
    }

    int32_t AuthRpcParam::keepalivePermitWithoutCalls() const
    {
        return keepalive_permit_without_calls_;
    }

    void AuthRpcParam::keepalivePermitWithoutCalls(const int32_t value)
    {
        keepalive_permit_without_calls_ = value;
        validate(); // Validate after setting new value
    }

    // ReSharper disable once CppDFAConstantFunctionResult
    const std::string& AuthRpcParam::serverAddress() const
    {
        return server_address_;
    }

    void AuthRpcParam::serverAddress(const std::string& value)
    {
        server_address_ = value;
        validate(); // Validate after setting new value
    }

    void AuthRpcParam::deserializedFromYamlFile(const std::filesystem::path& path)
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

    void AuthRpcParam::validate() const
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

bool YAML::convert<client_app::auth::AuthRpcParam>::decode(const Node& node, client_app::auth::AuthRpcParam& rhs)
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

YAML::Node YAML::convert<client_app::auth::AuthRpcParam>::encode(const client_app::auth::AuthRpcParam& rhs)
{
    Node node;
    node["keepaliveTimeMs"] = rhs.keepaliveTimeMs();
    node["keepaliveTimeoutMs"] = rhs.keepaliveTimeoutMs();
    node["keepalivePermitWithoutCalls"] = rhs.keepalivePermitWithoutCalls();
    node["serverAddress"] = rhs.serverAddress();
    return node;
}
