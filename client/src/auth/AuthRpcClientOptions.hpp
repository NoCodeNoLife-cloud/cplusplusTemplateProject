#pragma once
#include <yaml-cpp/node/node.h>
#include <filesystem>
#include <string>
#include <chrono>    // C++20

#include "src/serializer/interface/IYamlConfigurable.hpp"
#include "src/filesystem/type/YamlToolkit.hpp"

namespace client_app::auth {
    /// @brief A class that holds gRPC configuration options for the client
    /// @details This class encapsulates all the gRPC configuration parameters
    /// that can be used to customize the behavior of gRPC channels and connections.
    /// The configuration parameters can be loaded from a YAML configuration file.
    ///
    /// Example usage:
    /// @code
    /// auto options = AuthRpcClientOptions::builder()
    ///     .keepaliveTimeMs(30000)
    ///     .keepaliveTimeoutMs(5000)
    ///     .keepalivePermitWithoutCalls(1)
    ///     .serverAddress("localhost:50051")
    ///     .build();
    /// @endcode
    class AuthRpcClientOptions final : public common::interfaces::IYamlConfigurable {
    public:
        /// @brief Default constructor explicitly deleted to enforce parameterized construction
        AuthRpcClientOptions() = delete;

        /// @brief Constructor with explicit parameter initialization
        /// @param keepalive_time_ms Time interval between keepalive pings in milliseconds
        /// @param keepalive_timeout_ms Timeout for keepalive ping acknowledgment in milliseconds
        /// @param keepalive_permit_without_calls Flag to permit keepalive pings without active calls (1=true, 0=false)
        /// @param server_address The gRPC server address in format "host:port"
        AuthRpcClientOptions(int32_t keepalive_time_ms, int32_t keepalive_timeout_ms, int32_t keepalive_permit_without_calls, std::string server_address) noexcept;

        /// @brief Copy constructor deleted to prevent unintended resource duplication
        AuthRpcClientOptions(const AuthRpcClientOptions &) = delete;

        /// @brief Copy assignment operator deleted to prevent unintended resource duplication
        auto operator=(const AuthRpcClientOptions &) -> AuthRpcClientOptions & = delete;

        /// @brief Get the keepalive time interval in milliseconds
        /// @return The keepalive time interval in milliseconds
        /// @details This parameter controls how often the client sends keepalive pings
        /// to the server to ensure the connection is still alive.
        [[nodiscard]] auto keepaliveTimeMs() const noexcept -> int32_t;

        /// @brief Set the keepalive time interval in milliseconds
        /// @param value The keepalive time interval in milliseconds
        /// @details This parameter controls how often the client sends keepalive pings
        /// to the server to ensure the connection is still alive. Setting this to a lower
        /// value will detect connection failures faster but consume more network resources.
        auto keepaliveTimeMs(int32_t value) noexcept -> void;

        /// @brief Get the keepalive timeout in milliseconds
        /// @return The keepalive timeout in milliseconds
        /// @details This parameter controls how long the client waits for an acknowledgment
        /// of a keepalive ping from the server before considering the connection dead.
        [[nodiscard]] auto keepaliveTimeoutMs() const noexcept -> int32_t;

        /// @brief Set the keepalive timeout in milliseconds
        /// @param value The keepalive timeout in milliseconds
        /// @details This parameter controls how long the client waits for an acknowledgment
        /// of a keepalive ping from the server before considering the connection dead.
        /// Setting this too low might cause false positives during temporary network delays.
        auto keepaliveTimeoutMs(int32_t value) noexcept -> void;

        /// @brief Check if keepalive pings are permitted without active calls
        /// @return 1 if permitted, 0 if not permitted
        /// @details When set to true, keepalive pings are allowed even when there are
        /// no active RPC calls. When set to false, keepalive pings are only sent when
        /// there are active calls.
        [[nodiscard]] auto keepalivePermitWithoutCalls() const noexcept -> int32_t;

        /// @brief Set whether to permit keepalive pings without active calls
        /// @param value 1 to permit, 0 to not permit
        /// @details When set to true, keepalive pings are allowed even when there are
        /// no active RPC calls. When set to false, keepalive pings are only sent when
        /// there are active calls. Setting this to false can reduce unnecessary network traffic.
        auto keepalivePermitWithoutCalls(int32_t value) noexcept -> void;

        /// @brief Get the server address
        /// @return The server address as a string
        /// @details This parameter specifies the address and port of the gRPC server
        /// in the format "host:port". IPv4, IPv6, and hostnames are supported.
        [[nodiscard]] auto serverAddress() const noexcept -> const std::string &;

        /// @brief Set the server address
        /// @param value The server address as a string
        /// @details This parameter specifies the address and port of the gRPC server
        /// in the format "host:port". IPv4, IPv6, and hostnames are supported.
        auto serverAddress(const std::string &value) noexcept -> void;

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
        auto deserializedFromYamlFile(const std::filesystem::path &path) -> void override;

        /// @brief Builder class for constructing AuthRpcClientOptions instances
        /// @details Implements the Builder pattern to allow for flexible construction
        /// of AuthRpcClientOptions objects with default values and selective parameter setting.
        class Builder {
        public:
            /// @brief Set the keepalive time interval in milliseconds
            /// @param value The keepalive time interval in milliseconds
            /// @return Reference to this builder for method chaining
            [[nodiscard]] auto keepaliveTimeMs(int32_t value) noexcept -> Builder &;

            /// @brief Set the keepalive timeout in milliseconds
            /// @param value The keepalive timeout in milliseconds
            /// @return Reference to this builder for method chaining
            [[nodiscard]] auto keepaliveTimeoutMs(int32_t value) noexcept -> Builder &;

            /// @brief Set whether to permit keepalive pings without active calls
            /// @param value 1 to permit, 0 to not permit
            /// @return Reference to this builder for method chaining
            [[nodiscard]] auto keepalivePermitWithoutCalls(int32_t value) noexcept -> Builder &;

            /// @brief Set the server address
            /// @param value The server address as a string
            /// @return Reference to this builder for method chaining
            [[nodiscard]] auto serverAddress(const std::string &value) noexcept -> Builder &;

            /// @brief Build the AuthRpcClientOptions instance with the configured parameters
            /// @return A new AuthRpcClientOptions instance with the configured values
            [[nodiscard]] auto build() const -> AuthRpcClientOptions;

        private:
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

        /// @brief Create a new Builder instance for constructing AuthRpcClientOptions
        /// @return A new Builder instance with default values
        [[nodiscard]] static auto builder() -> Builder;

        /// @brief Validate gRPC parameters for correctness
        /// @details This function checks that the gRPC parameters are within reasonable ranges
        /// and logs warnings for potentially problematic configurations
        auto validate() const noexcept -> void;

    private:
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

/// @brief YAML serialization specialization for AuthRpcClientOptions.
/// Provides methods to encode and decode AuthRpcClientOptions to/from YAML nodes.
/// @details This template specialization allows the YAML library to automatically
/// serialize and deserialize AuthRpcClientOptions objects to and from YAML format.
template<>
struct YAML::convert<app_client::auth::AuthRpcClientOptions> {
    /// @brief Decode a YAML node into a AuthRpcClientOptions object.
    /// @param node The YAML node containing the configuration data.
    /// @param rhs The AuthRpcClientOptions object to populate.
    /// @return True if decoding was successful.
    /// @details Extracts configuration values from the YAML node and sets them
    /// in the AuthRpcClientOptions object. Missing values will retain their default values.
    static auto decode(const YAML::Node &node, app_client::auth::AuthRpcClientOptions &rhs) -> bool;

    /// @brief Encode a AuthRpcClientOptions object into a YAML node.
    /// @param rhs The AuthRpcClientOptions object to encode.
    /// @return A YAML node containing the configuration data.
    /// @details Converts the AuthRpcClientOptions object's configuration values into
    /// a YAML node representation that can be serialized to a file or string.
    static auto encode(const app_client::auth::AuthRpcClientOptions &rhs) -> YAML::Node;
};
