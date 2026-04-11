#pragma once

namespace common::service::interfaces {
    /// @brief Interface for configurable services.
    /// @details This interface provides a contract for services that require configuration.
    ///          Implementing classes must provide a configuration mechanism through doConfig().
    class IConfigurable {
    public:
        virtual ~IConfigurable() = default;

        /// @brief Public interface for configuring the service.
        /// @return true if configuration is successful, false otherwise.
        /// @throws std::runtime_error if configuration fails with details about the failure
        [[nodiscard]] auto config() -> bool {
            return doConfig();
        }

    protected:
        /// @brief Configure the service.
        /// @return true if configuration is successful, false otherwise.
        /// @throws std::runtime_error if configuration fails with details about the failure
        [[nodiscard]] virtual auto doConfig() -> bool = 0;
    };
}
