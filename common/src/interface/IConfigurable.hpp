/**
 * @file IConfigurable.hpp
 * @brief Interface for configurable objects (load/save settings)
 * @details Defines the contract for objects that can be configured from a
 *          property source: loadConfiguration() reads settings from a map or
 *          file, saveConfiguration() writes current settings back.  Usable
 *          with YAML or JSON serialisers.
 */

#pragma once

namespace cppforge::interface::config
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
