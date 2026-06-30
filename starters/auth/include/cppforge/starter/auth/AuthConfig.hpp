/**
 * @file AuthConfig.hpp
 * @brief Unified authentication RPC configuration options
 * @details This header defines the AuthConfig class that encapsulates
 *          gRPC configuration parameters including keepalive settings,
 *          connection management, and YAML-based configuration loading with builder pattern.
 *          Merged from server-side AuthRpcParam (7 params) and client-side AuthRpcParam (4 params).
 */

#pragma once
#include <filesystem>
#include <string>
#include <yaml-cpp/node/node.h>

#include <cppforge/interface/serialization/IYamlConfigurable.hpp>

namespace cppforge::starter::auth
{
    /// @brief A class that holds gRPC configuration options
    /// @details This class encapsulates all the gRPC configuration parameters
    /// that can be used to customize the behavior of gRPC channels and connections.
    /// The configuration parameters can be loaded from a YAML configuration file.
    ///
    /// Example usage:
    /// @code
    /// auto options = AuthConfig::builder()
    ///     .maxConnectionIdleMs(3600000)
    ///     .maxConnectionAgeMs(7200000)
    ///     .maxConnectionAgeGraceMs(300000)
    ///     .keepaliveTimeMs(30000)
    ///     .keepaliveTimeoutMs(5000)
    ///     .keepalivePermitWithoutCalls(1)
    ///     .serverAddress("0.0.0.0:50051")
    ///     .build();
    /// @endcode
    class AuthConfig final : public cppforge::interface::serialization::IYamlConfigurable
    {
    public:
        AuthConfig();

        /// @brief Constructor with all parameters
        AuthConfig(int32_t max_connection_idle_ms, int32_t max_connection_age_ms, int32_t max_connection_age_grace_ms, int32_t keepalive_time_ms, int32_t keepalive_timeout_ms, int32_t keepalive_permit_without_calls, std::string server_address);

        /// @brief Get the maximum connection idle time in milliseconds
        /// @return The maximum connection idle time in milliseconds
        [[nodiscard]] int32_t maxConnectionIdleMs() const;

        /// @brief Set the maximum connection idle time in milliseconds
        /// @param value The maximum connection idle time in milliseconds
        void maxConnectionIdleMs(int32_t value);

        /// @brief Get the maximum connection age in milliseconds
        /// @return The maximum connection age in milliseconds
        [[nodiscard]] int32_t maxConnectionAgeMs() const;

        /// @brief Set the maximum connection age in milliseconds
        /// @param value The maximum connection age in milliseconds
        void maxConnectionAgeMs(int32_t value);

        /// @brief Get the maximum connection age grace period in milliseconds
        /// @return The maximum connection age grace period in milliseconds
        [[nodiscard]] int32_t maxConnectionAgeGraceMs() const;

        /// @brief Set the maximum connection age grace period in milliseconds
        /// @param value The maximum connection age grace period in milliseconds
        void maxConnectionAgeGraceMs(int32_t value);

        /// @brief Get the keepalive time interval in milliseconds
        /// @return The keepalive time interval in milliseconds
        [[nodiscard]] int32_t keepaliveTimeMs() const;

        /// @brief Set the keepalive time interval in milliseconds
        /// @param value The keepalive time interval in milliseconds
        void keepaliveTimeMs(int32_t value);

        /// @brief Get the keepalive timeout in milliseconds
        /// @return The keepalive timeout in milliseconds
        [[nodiscard]] int32_t keepaliveTimeoutMs() const;

        /// @brief Set the keepalive timeout in milliseconds
        /// @param value The keepalive timeout in milliseconds
        void keepaliveTimeoutMs(int32_t value);

        /// @brief Check if keepalive pings are permitted without active calls
        /// @return 1 if permitted, 0 if not permitted
        [[nodiscard]] int32_t keepalivePermitWithoutCalls() const;

        /// @brief Set whether to permit keepalive pings without active calls
        /// @param value 1 to permit, 0 to not permit
        void keepalivePermitWithoutCalls(int32_t value);

        /// @brief Get the server address
        /// @return The server address as a string
        [[nodiscard]] const std::string& serverAddress() const;

        /// @brief Set the server address
        /// @param value The server address as a string
        void serverAddress(const std::string& value);

        /// @brief Deserialize object configuration from a YAML file
        /// @param path The file path to the YAML configuration file
        /// @throws std::runtime_error If the file cannot be read or parsed
        void deserializeFromYamlFile(const std::filesystem::path& path) override;

        /// @brief Builder class for constructing AuthConfig instances
        /// @details Implements the Builder pattern to allow for flexible construction
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

            /// @brief Build the AuthConfig instance with the configured parameters
            /// @return A new AuthConfig instance with the configured values
            [[nodiscard]] AuthConfig build() const;

        private:
            int32_t max_connection_idle_ms_{60 * 60 * 1000};
            int32_t max_connection_age_ms_{2 * 60 * 60 * 1000};
            int32_t max_connection_age_grace_ms_{5 * 60 * 1000};
            int32_t keepalive_time_ms_{30 * 1000};
            int32_t keepalive_timeout_ms_{5 * 1000};
            int32_t keepalive_permit_without_calls_{1};
            std::string server_address_{"0.0.0.0:50051"};
        };

        /// @brief Create a new Builder instance for constructing AuthConfig
        /// @return A new Builder instance with default values
        static Builder builder();

    private:
        /// @brief Validate gRPC parameters for correctness
        void validateParameters() const;

        int32_t max_connection_idle_ms_{60 * 60 * 1000};
        int32_t max_connection_age_ms_{2 * 60 * 60 * 1000};
        int32_t max_connection_age_grace_ms_{5 * 60 * 1000};
        int32_t keepalive_time_ms_{30 * 1000};
        int32_t keepalive_timeout_ms_{5 * 1000};
        int32_t keepalive_permit_without_calls_{1};
        std::string server_address_{"0.0.0.0:50051"};
    };
}

/// @brief YAML serialization specialization for AuthConfig.
template <>
struct YAML::convert<cppforge::starter::auth::AuthConfig>
{
    static bool decode(const Node& node, cppforge::starter::auth::AuthConfig& rhs);
    static Node encode(const cppforge::starter::auth::AuthConfig& rhs);
};
