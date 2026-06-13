/**
 * @file UserCredentials.hpp
 * @brief User credential value object — username, password hash, roles
 * @description Holds user authentication data: username/email, salted password
 *          hash (not plaintext), assigned roles/permissions, and account
 *          status (active/locked/expired).  Used as the data transfer object
 *          between authentication layers.
 *
 * @par Thread Safety
 * This class is const-only thread-safe (immutable after construction).
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
        explicit UserCredentials(std::string username, std::string hashed_password, std::string salt) ;

        /// @brief Get username
        /// @return Username string
        [[nodiscard]] const std::string& get_username() const ;

        /// @brief Get hashed password
        /// @return Hashed password string
        [[nodiscard]] const std::string& get_hashed_password() const ;

        /// @brief Get salt value
        /// @return Salt string used for hashing
        [[nodiscard]] const std::string& get_salt() const ;

        /// @brief Get number of failed login attempts
        /// @return Count of failed attempts
        [[nodiscard]] size_t get_failed_attempts() const ;

        /// @brief Increment failed login attempt counter
        void increment_failed_attempts() ;

        /// @brief Reset failed attempt counter
        void reset_failed_attempts() ;

        /// @brief Check if account is locked due to excessive failed attempts
        /// @return true if account is locked, false otherwise
        [[nodiscard]] bool is_locked() const ;

        /// @brief Check if account lock will expire after a specific duration
        /// @param lockout_duration Duration in minutes after which lockout expires
        /// @return true if account is locked, false otherwise
        [[nodiscard]] bool is_locked(std::chrono::minutes lockout_duration, size_t max_attempts) const ;

    private:
        std::string username_;
        std::string hashed_password_;
        std::string salt_;
        size_t failed_attempts_;
        std::chrono::system_clock::time_point last_failed_attempt_;

        static constexpr size_t DEFAULT_MAX_ATTEMPTS = 5;
        static constexpr std::chrono::minutes DEFAULT_LOCKOUT_DURATION{5};
    };
}