/**
 * @file UserCredentials.hpp
 * @brief UserCredentials class declaration
 * @details This header defines the UserCredentials class that provides functionality for Authentication and authorization utilities.
 */

#pragma once
#include <chrono>
#include <string>

namespace common::auth
{
    /// @brief User credentials storage with security features
    class UserCredentials
    {
    public:
        /// @brief Constructor for new user credentials
        /// @param username User identifier
        /// @param hashed_password Hashed password value
        /// @param salt Salt used for password hashing
        explicit UserCredentials(std::string username, std::string hashed_password, std::string salt) noexcept;

        /// @brief Get username
        /// @return Username string
        [[nodiscard]] auto get_username() const noexcept -> const std::string&;

        /// @brief Get hashed password
        /// @return Hashed password string
        [[nodiscard]] auto get_hashed_password() const noexcept -> const std::string&;

        /// @brief Get salt value
        /// @return Salt string used for hashing
        [[nodiscard]] auto get_salt() const noexcept -> const std::string&;

        /// @brief Get number of failed login attempts
        /// @return Count of failed attempts
        [[nodiscard]] auto get_failed_attempts() const noexcept -> size_t;

        /// @brief Increment failed login attempt counter
        auto increment_failed_attempts() noexcept -> void;

        /// @brief Reset failed attempt counter
        auto reset_failed_attempts() noexcept -> void;

        /// @brief Check if account is locked due to excessive failed attempts
        /// @return true if account is locked, false otherwise
        [[nodiscard]] auto is_locked() const noexcept -> bool;

        /// @brief Check if account lock will expire after a specific duration
        /// @param lockout_duration Duration in minutes after which lockout expires
        /// @return true if account is locked, false otherwise
        [[nodiscard]] auto is_locked(std::chrono::minutes lockout_duration, size_t max_attempts) const noexcept -> bool;

    private:
        std::string username_;
        std::string hashed_password_;
        std::string salt_;
        size_t failed_attempts_;
        std::chrono::system_clock::time_point last_failed_attempt_;

        static constexpr size_t DEFAULT_MAX_ATTEMPTS = 5;
        static constexpr std::chrono::minutes DEFAULT_LOCKOUT_DURATION{5};
    };
} // common
