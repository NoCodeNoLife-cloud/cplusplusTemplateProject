/**
 * @file AuthRpcParam.hpp
 * @brief gRPC service configuration options
 * @details This header defines the AuthRpcServiceOptions class that encapsulates
 *          gRPC server configuration parameters including keepalive settings,
 *          connection management, and YAML-based configuration loading with builder pattern.
 */

#pragma once
#include <filesystem>
#include <string>
#include <yaml-cpp/node/node.h>

#include "interface/serialize/IYamlConfigurable.hpp"

namespace server_app::auth
{
    /// @brief A class that holds gRPC configuration options
    /// @details This class encapsulates all the gRPC configuration parameters
    /// that can be used to customize the behavior of gRPC channels and connections.
    /// The configuration parameters can be loaded from a YAML configuration file.
    ///
    /// Example usage:
    /// @code
    /// auto options = AuthRpcServiceOptions::builder()
    ///     .maxConnectionIdleMs(3600000)
    ///     .maxConnectionAgeMs(7200000)
    ///     .maxConnectionAgeGraceMs(300000)
    ///     .keepaliveTimeMs(30000)
    ///     .keepaliveTimeoutMs(5000)
    ///     .keepalivePermitWithoutCalls(1)
    ///     .serverAddress("0.0.0.0:50051")
    ///     .build();
    /// @endcode
    class AuthRpcParam final : public common::interface::serialize::IYamlConfigurable
    {
    public:
        AuthRpcParam();

        /// @brief Constructor with all parameters
        AuthRpcParam(int32_t max_connection_idle_ms, int32_t max_connection_age_ms, int32_t max_connection_age_grace_ms, int32_t keepalive_time_ms, int32_t keepalive_timeout_ms, int32_t keepalive_permit_without_calls, std::string server_address);

        /// @brief Get the maximum connection idle time in milliseconds
        /// @return The maximum connection idle time in milliseconds
        /// @details This parameter controls how long a connection can remain idle
        /// before the server closes it. Helps manage server resources by cleaning
        /// up unused connections.
        [[nodiscard]] int32_t maxConnectionIdleMs() const;

        /// @brief Set the maximum connection idle time in milliseconds
        /// @param value The maximum connection idle time in milliseconds
        /// @details This parameter controls how long a connection can remain idle
        /// before the server closes it. Setting this too low might disconnect
        /// clients that have legitimate periods of inactivity.
        void maxConnectionIdleMs(int32_t value);

        /// @brief Get the maximum connection age in milliseconds
        /// @return The maximum connection age in milliseconds
        /// @details This parameter controls the maximum age of a connection before
        /// it is gracefully closed. Helps with connection rotation and resource management.
        [[nodiscard]] int32_t maxConnectionAgeMs() const;

        /// @brief Set the maximum connection age in milliseconds
        /// @param value The maximum connection age in milliseconds
        /// @details This parameter controls the maximum age of a connection before
        /// it is gracefully closed. Helps with connection rotation and resource management.
        /// Setting this too low might cause frequent reconnects.
        void maxConnectionAgeMs(int32_t value);

        /// @brief Get the maximum connection age grace period in milliseconds
        /// @return The maximum connection age grace period in milliseconds
        /// @details This parameter controls the grace period after max connection age
        /// during which pending RPCs can complete before the connection is forcibly closed.
        [[nodiscard]] int32_t maxConnectionAgeGraceMs() const;

        /// @brief Set the maximum connection age grace period in milliseconds
        /// @param value The maximum connection age grace period in milliseconds
        /// @details This parameter controls the grace period after max connection age
        /// during which pending RPCs can complete before the connection is forcibly closed.
        void maxConnectionAgeGraceMs(int32_t value);

        /// @brief Get the keepalive time interval in milliseconds
        /// @return The keepalive time interval in milliseconds
        /// @details This parameter controls how often the server sends keepalive pings
        /// to the client to ensure the connection is still alive.
        [[nodiscard]] int32_t keepaliveTimeMs() const;

        /// @brief Set the keepalive time interval in milliseconds
        /// @param value The keepalive time interval in milliseconds
        /// @details This parameter controls how often the server sends keepalive pings
        /// to the client to ensure the connection is still alive. Setting this to a lower
        /// value will detect connection failures faster but consume more network resources.
        void keepaliveTimeMs(int32_t value);

        /// @brief Get the keepalive timeout in milliseconds
        /// @return The keepalive timeout in milliseconds
        /// @details This parameter controls how long the server waits for an acknowledgment
        /// of a keepalive ping from the client before considering the connection dead.
        [[nodiscard]] int32_t keepaliveTimeoutMs() const;

        /// @brief Set the keepalive timeout in milliseconds
        /// @param value The keepalive timeout in milliseconds
        /// @details This parameter controls how long the server waits for an acknowledgment
        /// of a keepalive ping from the client before considering the connection dead.
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
        /// in the format "host:port". Using "0.0.0.0" binds to all available interfaces.
        [[nodiscard]] const std::string& serverAddress() const;

        /// @brief Set the server address
        /// @param value The server address as a string
        /// @details This parameter specifies the address and port of the gRPC server
        /// in the format "host:port". Using "0.0.0.0" binds to all available interfaces.
        void serverAddress(const std::string& value);

        /// @brief Deserialize object configuration from a YAML file
        /// @param path The file path to the YAML configuration file
        /// @return true if successful, false otherwise
        /// @throws std::runtime_error If the file cannot be read or parsed
        /// @details This method loads the gRPC configuration from a YAML file. The expected
        /// YAML structure should contain keys matching the configuration parameters:
        /// @code
        /// grpc:
        ///   max-connection-idle-ms: 3600000
        ///   max-connection-age-ms: 7200000
        ///   max-connection-age-grace-ms: 300000
        ///   keepalive-time-ms: 30000
        ///   keepalive-timeout-ms: 5000
        ///   keepalive-permit-without-calls: 1
        ///   server-address: "0.0.0.0:50051"
        /// @endcode
        void deserializeFromYamlFile(const std::filesystem::path& path) override;

        /// @brief Builder class for constructing AuthRpcServiceOptions instances
        /// @details Implements the Builder pattern to allow for flexible construction
        /// of AuthRpcServiceOptions objects with default values and selective parameter setting.
        class Builder
        {
        public:
            /// @brief Set the maximum connection idle time in milliseconds
            /// @param value The maximum connection idle time in milliseconds
            /// @return Reference to this builder for method chaining
            [[nodiscard]] Builder& maxConnectionIdleMs(int32_t value);

            /// @brief Set the maximum connection age in milliseconds
            /// @param value The maximum connection age in milliseconds
            /// @return Reference to this builder for method chaining
            [[nodiscard]] Builder& maxConnectionAgeMs(int32_t value);

            /// @brief Set the maximum connection age grace period in milliseconds
            /// @param value The maximum connection age grace period in milliseconds
            /// @return Reference to this builder for method chaining
            [[nodiscard]] Builder& maxConnectionAgeGraceMs(int32_t value);

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

            /// @brief Build the AuthRpcServiceOptions instance with the configured parameters
            /// @return A new AuthRpcServiceOptions instance with the configured values
            [[nodiscard]] AuthRpcParam build() const;

        private:
            /// @brief Maximum time a connection can remain idle before being closed (in milliseconds)
            /// @details Default value is 1 hour (60 * 60 * 1000 ms).
            int32_t max_connection_idle_ms_{60 * 60 * 1000};

            /// @brief Maximum age of a connection before it is gracefully closed (in milliseconds)
            /// @details Default value is 2 hours (2 * 60 * 60 * 1000 ms).
            int32_t max_connection_age_ms_{2 * 60 * 60 * 1000};

            /// @brief Grace period after max connection age before force closing (in milliseconds)
            /// @details Default value is 5 minutes (5 * 60 * 1000 ms).
            int32_t max_connection_age_grace_ms_{5 * 60 * 1000};

            /// @brief Time interval between keepalive pings (in milliseconds)
            /// @details This parameter controls how often the server sends keepalive pings
            /// to the client to ensure the connection is still alive.
            /// Default value is 30 seconds (30000 ms).
            int32_t keepalive_time_ms_{30 * 1000};

            /// @brief Timeout for keepalive ping acknowledgment (in milliseconds)
            /// @details This parameter controls how long the server waits for an acknowledgment
            /// of a keepalive ping from the client before considering the connection dead.
            /// Default value is 5 seconds (5000 ms).
            int32_t keepalive_timeout_ms_{5 * 1000};

            /// @brief Whether to permit keepalive pings when there are no active calls (1 = true, 0 = false)
            /// @details When set to true, keepalive pings are allowed even when there are no active RPC calls.
            /// When set to false, keepalive pings are only sent when there are active calls.
            /// Default value is true (1).
            int32_t keepalive_permit_without_calls_{1};

            /// @brief The server address to listen on
            /// @details This parameter specifies the address and port of the gRPC server
            /// Default value is 0.0.0.0:50051
            std::string server_address_{"0.0.0.0:50051"};
        };

        /// @brief Create a new Builder instance for constructing AuthRpcServiceOptions
        /// @return A new Builder instance with default values
        static Builder builder();

    private:
        /// @brief Validate gRPC parameters for correctness
        /// @details This function checks that the gRPC parameters are within reasonable ranges
        /// and logs warnings for potentially problematic configurations
        void validateParameters() const;

        /// @brief Maximum time a connection can remain idle before being closed (in milliseconds)
        /// @details Default value is 1 hour (60 * 60 * 1000 ms).
        int32_t max_connection_idle_ms_{60 * 60 * 1000};

        /// @brief Maximum age of a connection before it is gracefully closed (in milliseconds)
        /// @details Default value is 2 hours (2 * 60 * 60 * 1000 ms).
        int32_t max_connection_age_ms_{2 * 60 * 60 * 1000};

        /// @brief Grace period after max connection age before force closing (in milliseconds)
        /// @details Default value is 5 minutes (5 * 60 * 1000 ms).
        int32_t max_connection_age_grace_ms_{5 * 60 * 1000};

        /// @brief Time interval between keepalive pings (in milliseconds)
        /// @details This parameter controls how often the server sends keepalive pings
        /// to the client to ensure the connection is still alive.
        /// Default value is 30 seconds (30000 ms).
        int32_t keepalive_time_ms_{30 * 1000};

        /// @brief Timeout for keepalive ping acknowledgment (in milliseconds)
        /// @details This parameter controls how long the server waits for an acknowledgment
        /// of a keepalive ping from the client before considering the connection dead.
        /// Default value is 5 seconds (5000 ms).
        int32_t keepalive_timeout_ms_{5 * 1000};

        /// @brief Whether to permit keepalive pings when there are no active calls (1 = true, 0 = false)
        /// @details When set to true, keepalive pings are allowed even when there are no active RPC calls.
        /// When set to false, keepalive pings are only sent when there are active calls.
        /// Default value is true (1).
        int32_t keepalive_permit_without_calls_{1};

        /// @brief The server address to listen on
        /// @details This parameter specifies the address and port of the gRPC server
        /// Default value is 0.0.0.0:50051
        std::string server_address_{"0.0.0.0:50051"};
    };
}

/// @brief YAML serialization specialization for AuthRpcServiceOptions.
/// Provides methods to encode and decode AuthRpcServiceOptions to/from YAML nodes.
/// @details This template specialization allows the YAML library to automatically
/// serialize and deserialize AuthRpcServiceOptions objects to and from YAML format.
template <>
struct YAML::convert<server_app::auth::AuthRpcParam>
{
    /// @brief Decode a YAML node into a AuthRpcServiceOptions object.
    /// @param node The YAML node containing the configuration data.
    /// @param rhs The AuthRpcServiceOptions object to populate.
    /// @return True if decoding was successful.
    /// @details Extracts configuration values from the YAML node and sets them
    /// in the AuthRpcServiceOptions object. Missing values will retain their default values.
    static bool decode(const Node& node, server_app::auth::AuthRpcParam& rhs);

    /// @brief Encode a AuthRpcServiceOptions object into a YAML node.
    /// @param rhs The AuthRpcServiceOptions object to encode.
    /// @return A YAML node containing the configuration data.
    /// @details Converts the AuthRpcServiceOptions object's configuration values into
    /// a YAML node representation that can be serialized to a file or string.
    static Node encode(const server_app::auth::AuthRpcParam& rhs);
};
