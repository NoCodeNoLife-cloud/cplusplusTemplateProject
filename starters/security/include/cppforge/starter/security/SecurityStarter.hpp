/**
 * @file SecurityStarter.hpp
 * @brief DI registration for the security starter
 * @details Provides registerServices() to register JwtToken and RbacManager
 *          with the DI container.
 */

#pragma once
#include <cppforge/di/IServiceCollection.hpp>
#include <cppforge/starter/security/SecurityConfig.hpp>

namespace cppforge::starter::security
{
    /// @brief Registers security services with the DI container
    /// @details Registers JwtToken and RbacManager as singletons.
    class SecurityStarter
    {
    public:
        /// @brief Register security services with the DI container
        /// @param collection The service collection to register with
        /// @param config Security configuration parameters
        static void registerServices(di::IServiceCollection& collection, const SecurityConfig& config);

        SecurityStarter() = delete;
    };
}
