/**
 * @file IConfigurable.hpp
 * @brief IConfigurable class declaration
 * @details This header defines the IConfigurable class that provides functionality for Common interface definitions for the framework.
 */

#pragma once

namespace common::interfaces
{
    /// @brief Interface for configurable services.
    /// @details This interface provides a contract for services that require configuration.
    ///          Implementing classes must provide a configuration mechanism through doConfig().
    class IConfigurable
    {
    public:
        virtual ~IConfigurable() = default;

        /// @brief Public interface for configuring the service.
        /// @return true if configuration is successful, false otherwise.
        /// @throws std::runtime_error if configuration fails with details about the failure
        [[nodiscard]] bool config()
        {
            return doConfig();
        }

    protected:
        /// @brief Configure the service.
        /// @return true if configuration is successful, false otherwise.
        /// @throws std::runtime_error if configuration fails with details about the failure
        [[nodiscard]] virtual bool doConfig() = 0;
    };
}
