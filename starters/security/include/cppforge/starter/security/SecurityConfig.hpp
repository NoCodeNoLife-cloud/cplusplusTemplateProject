/**
 * @file SecurityConfig.hpp
 * @brief Configuration options for the security starter
 * @details Defines SecurityConfig struct with JWT secret and expiry settings.
 */

#pragma once
#include <chrono>
#include <string>

namespace cppforge::starter::security
{
    /// @brief Configuration for the security subsystem
    struct SecurityConfig
    {
        /// @brief Secret key used for JWT signing and verification
        std::string jwt_secret = "default_secret_change_me";

        /// @brief Default expiry duration for JWT tokens
        std::chrono::hours jwt_expiry = std::chrono::hours(24);
    };
}
