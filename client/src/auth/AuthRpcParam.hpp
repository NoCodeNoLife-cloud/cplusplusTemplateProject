/**
 * @file AuthRpcParam.hpp
 * @brief Configuration options for the authentication RPC client
 * @details This header defines the AuthRpcClientOptions class that encapsulates
 *          gRPC configuration parameters including keepalive settings, server address,
 *          and YAML-based configuration loading with builder pattern support.
 */

#pragma once
#include <chrono>    // C++20
#include <filesystem>
#include <string>
#include <yaml-cpp/node/node.h>

#include "filesystem/type/YamlToolkit.hpp"
#include "interface/serialize/IYamlConfigurable.hpp"

namespace client_app::auth
{
    /// @brief A class that holds gRPC configuration options for the client
    /// @details This class encapsulates all the gRPC configuration parameters
    /// that can be used to customize the behavior of gRPC channels and connections.
    /// The configuration parameters can be loaded from a YAML configuration file.
    ///
    /// Example usage:
    /// @code
    /// auto options = AuthRpcParam::builder()
    ///     .keepaliveTimeMs(30000)
    ///     .serverAddress("localhost:50051")
    ///     .build();
    /// @endcode
    class AuthRpcParam final : public common::interface::serialize::IYamlConfigurable
    {
    public:
        /// @brief Constructor with explicit parameter initialization
        /// @param keepalive_time_ms Time interval between keepalive pings in milliseconds (default: 30000)
        /// @param keepalive_timeout_ms Timeout for keepalive ping acknowledgment in milliseconds (default: 5000)
        /// @param keepalive_permit_without_calls Flag to permit keepalive pings without active calls (1=true, 0=false) (default: 1)
        /// @param server_address The gRPC server address in format "host:port" (default: "localhost:50051")
        explicit AuthRpcParam(int32_t keepalive_time_ms = 30 * 1000, int32_t keepalive_timeout_ms = 5 * 1000, int32_t keepalive_permit_without_calls = 1, std::string server_address = "localhost:50051");

        /// @brief Copy assignment operator deleted to prevent unintended resource duplication
        auto operator=(const AuthRpcParam&) -> AuthRpcParam& = delete;

        /// @brief Get the keepalive time interval in milliseconds
        /// @return The keepalive time interval in milliseconds
        /// @details This parameter controls how often the client sends keepalive pings
        /// to the server to ensure the connection is still alive.
        [[nodiscard]] int32_t keepaliveTimeMs() const;

        /// @brief Set the keepalive time interval in milliseconds
        /// @param value The keepalive time interval in milliseconds
        /// @details This parameter controls how often the client sends keepalive pings
        /// to the server to ensure the connection is still alive. Setting this to a lower
        /// value will detect connection failures faster but consume more network resources.
        void keepaliveTimeMs(int32_t value);

        /// @brief Get the keepalive timeout in milliseconds
        /// @return The keepalive timeout in milliseconds
        /// @details This parameter controls how long the client waits for an acknowledgment
        /// of a keepalive ping from the server before considering the connection dead.
        [[nodiscard]] int32_t keepaliveTimeoutMs() const;

        /// @brief Set the keepalive timeout in milliseconds
        /// @param value The keepalive timeout in milliseconds
        /// @details This parameter controls how long the client waits for an acknowledgment
        /// of a keepalive ping from the server before considering the connection dead.
        /// Setting this too low might cause false positives during temporary network delays.
        void keepaliveTimeoutMs(int32_t value);

        /// @brief Check if keepalive pings are permitted without active calls
        /// @return 1 if permitted, 0 if not permitted
        /// @details When set to true, keepalive pings are allowed even when there are
        /// no active RPC calls. When set to false, keepalive pings are only sent when
        /// there are active calls.
        [[nodiscard]] int32_t keepalivePermitWithoutCalls() const;

        /// @brief Set whether to permit keepalive pings without active calls
        /// @param value 1 to permit, 0 to not permit
        /// @details When set to true, keepalive pings are allowed even when there are
        /// no active RPC calls. When set to false, keepalive pings are only sent when
        /// there are active calls. Setting this to false can reduce unnecessary network traffic.
        void keepalivePermitWithoutCalls(int32_t value);

        /// @brief Get the server address
        /// @return The server address as a string
        /// @details This parameter specifies the address and port of the gRPC server
        /// in the format "host:port". IPv4, IPv6, and hostnames are supported.
        [[nodiscard]] const std::string& serverAddress() const;

        /// @brief Set the server address
        /// @param value The server address as a string
        /// @details This parameter specifies the address and port of the gRPC server
        /// in the format "host:port". IPv4, IPv6, and hostnames are supported.
        void serverAddress(const std::string& value);

        /// @brief Builder class for constructing AuthRpcParam instances
        /// @details Implements the Builder pattern to allow for flexible construction
        /// of AuthRpcParam objects with default values and selective parameter setting.
        class Builder
        {
        public:
            /// @brief Set the keepalive time interval in milliseconds
            /// @param value The keepalive time interval in milliseconds
            /// @return Reference to this builder for method chaining
            [[nodiscard]] Builder& keepaliveTimeMs(int32_t value);

            /// @brief Set the keepalive timeout in milliseconds
            /// @param value The keepalive timeout in milliseconds
            /// @return Reference to this builder for method chaining
            [[nodiscard]] Builder& keepaliveTimeoutMs(int32_t value);

            /// @brief Set whether to permit keepalive pings without active calls
            /// @param value 1 to permit, 0 to not permit
            /// @return Reference to this builder for method chaining
            [[nodiscard]] Builder& keepalivePermitWithoutCalls(int32_t value);

            /// @brief Set the server address
            /// @param value The server address as a string
            /// @return Reference to this builder for method chaining
            [[nodiscard]] Builder& serverAddress(const std::string& value);

            /// @brief Build the AuthRpcParam instance with the configured parameters
            /// @return A new AuthRpcParam instance with the configured values
            [[nodiscard]] AuthRpcParam build() const;

        private:
            int32_t keepalive_time_ms_{30 * 1000};
            int32_t keepalive_timeout_ms_{5 * 1000};
            int32_t keepalive_permit_without_calls_{1};
            std::string server_address_{"localhost:50051"};
        };

        /// @brief Create a new Builder instance for constructing AuthRpcParam
        /// @return A new Builder instance with default values
        static Builder builder();

        /// @brief Deserialize gRPC options from a YAML file
        /// @param path Path to the YAML file containing the configuration
        /// @return true if successful, false otherwise
        /// @throws std::runtime_error If file cannot be opened or decoded
        /// @details This method loads the gRPC configuration from a YAML file. The expected
        /// YAML structure should contain keys matching the configuration parameters:
        /// @code
        /// grpc:
        ///   keepalive-time-ms: 30000
        ///   keepalive-timeout-ms: 5000
        ///   keepalive-permit-without-calls: 1
        ///   server-address: "localhost:50051"
        /// @endcode
        auto deserializeFromYamlFile(const std::filesystem::path& path) -> void override;

    private:
        /// @brief Validate gRPC parameters for correctness
        /// @details This function checks that the gRPC parameters are within reasonable ranges
        /// and logs warnings for potentially problematic configurations
        void validate() const;

        /// @brief Time interval between keepalive pings (in milliseconds)
        /// @details This parameter controls how often the client sends keepalive pings
        /// to the server to ensure the connection is still alive.
        /// Default value is 30 seconds (30000 ms).
        int32_t keepalive_time_ms_{30 * 1000};

        /// @brief Timeout for keepalive ping acknowledgment (in milliseconds)
        /// @details This parameter controls how long the client waits for an acknowledgment
        /// of a keepalive ping from the server before considering the connection dead.
        /// Default value is 5 seconds (5000 ms).
        int32_t keepalive_timeout_ms_{5 * 1000};

        /// @brief Whether to permit keepalive pings when there are no active calls (1 = true, 0 = false)
        /// @details When set to true, keepalive pings are allowed even when there are no active RPC calls.
        /// When set to false, keepalive pings are only sent when there are active calls.
        /// Default value is true (1).
        int32_t keepalive_permit_without_calls_{1};

        /// @brief The server address to connect to
        /// @details This parameter specifies the address and port of the gRPC server
        /// Default value is localhost:50051
        std::string server_address_{"localhost:50051"};
    };
}

/// @brief YAML serialization specialization for AuthRpcParam.
/// Provides methods to encode and decode AuthRpcParam to/from YAML nodes.
/// @details This template specialization allows the YAML library to automatically
/// serialize and deserialize AuthRpcParam objects to and from YAML format.
template <>
struct YAML::convert<client_app::auth::AuthRpcParam>
{
    /// @brief Decode a YAML node into a AuthRpcParam object.
    /// @param node The YAML node containing the configuration data.
    /// @param rhs The AuthRpcParam object to populate.
    /// @return True if decoding was successful.
    /// @details Extracts configuration values from the YAML node and sets them
    /// in the AuthRpcParam object. Missing values will retain their default values.
    static bool decode(const Node& node, client_app::auth::AuthRpcParam& rhs);

    /// @brief Encode a AuthRpcParam object into a YAML node.
    /// @param rhs The AuthRpcParam object to encode.
    /// @return A YAML node containing the configuration data.
    /// @details Converts the AuthRpcParam object's configuration values into
    /// a YAML node representation that can be serialized to a file or string.
    static Node encode(const client_app::auth::AuthRpcParam& rhs);
};
