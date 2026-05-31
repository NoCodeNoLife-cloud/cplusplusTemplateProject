/**
 * @file UserAuthenticator.hpp
 * @brief UserAuthenticator class declaration
 * @details This header defines the UserAuthenticator class that provides functionality for Authentication and authorization utilities.
 */

#pragma once
#include <mutex>
#include <unordered_map>
#include <optional>
#include <memory>

#include "PasswordPolicy.hpp"
#include "UserCredentials.hpp"
#include "src/sql/PasswordSQL.hpp"

namespace common::auth
{
    /// @brief Result of an authentication attempt
    struct AuthResult
    {
        bool success; ///< Whether authentication was successful
        std::string error_message; ///< Error message if authentication failed
        UserCredentials* user = nullptr; ///< Pointer to user credentials (only valid if success is true)

        /// @brief Create a successful auth result
        static AuthResult success_result(UserCredentials* user)
        {
            return AuthResult{true, "", user};
        }

        /// @brief Create a failed auth result
        static AuthResult failure_result(const std::string& error)
        {
            return AuthResult{false, error, nullptr};
        }

        /// @brief Check if authentication was successful
        [[nodiscard]] bool is_success() const
        {
            return success;
        }
    };

    /// @brief Main authentication class providing user management and verification
    class UserAuthenticator
    {
    public:
        /// @brief Constructor with database path and optional custom password policy
        /// @param db_path Path to SQLite database file
        /// @param policy Custom password policy (default: standard policy)
        explicit UserAuthenticator(const std::string& db_path, const PasswordPolicy& policy = PasswordPolicy());

        /// @brief Register new user with username and password
        /// @param username User identifier to register
        /// @param password Plaintext password for new account
        /// @return true if registration successful
        /// @throws AuthenticationException if username invalid, exists, or password fails policy
        [[nodiscard]] bool register_user(const std::string& username, const std::string& password);

        /// @brief Authenticate user with username and password
        /// @param username User identifier
        /// @param password Plaintext password to verify
        /// @return AuthResult containing success status, error message (if any), and user pointer (if successful)
        /// @note This method does NOT throw exceptions for authentication failures. Use the returned AuthResult to check status.
        [[nodiscard]] AuthResult authenticate(const std::string& username, const std::string& password);

        /// @brief Change user password after verifying current password
        /// @param username User identifier
        /// @param current_password Current plaintext password for verification
        /// @param new_password New plaintext password to set
        /// @return true if password changed successfully
        /// @throws AuthenticationException if current password incorrect or new password fails policy
        [[nodiscard]] bool change_password(const std::string& username, const std::string& current_password, const std::string& new_password);

        /// @brief Reset user password (administrative function)
        /// @param username User identifier
        /// @param new_password New plaintext password to set
        /// @return true if password reset successful
        /// @throws AuthenticationException if user not found or new password fails policy
        [[nodiscard]] bool reset_password(const std::string& username, const std::string& new_password);

        /// @brief Check if user exists in the system
        /// @param username User identifier to check
        /// @return true if user exists, false otherwise
        [[nodiscard]] bool user_exists(const std::string& username) const;

        /// @brief Delete user from the system
        /// @param username User identifier to delete
        /// @return true if user deleted successfully
        [[nodiscard]] bool delete_user(const std::string& username);

        /// @brief Set custom password policy
        /// @param policy New password policy configuration
        void set_password_policy(const PasswordPolicy& policy);

        /// @brief Get reference to the users map for administrative operations
        /// @return Reference to the users map
        std::unordered_map<std::string, std::unique_ptr<UserCredentials>>& get_users();

        /// @brief Get reference to the mutex protecting the users map
        /// @return Reference to the users mutex
        std::mutex& get_users_mutex() const;

    private:
        /// @brief Validate username format against security requirements
        /// @param username Username string to validate
        /// @return true if username format is valid, false otherwise
        static bool validate_username(const std::string& username) noexcept;

        /// @brief Load user credentials from database
        /// @param username User identifier to load
        /// @return User credentials if found, nullopt otherwise
        std::optional<UserCredentials> load_user_from_db(const std::string& username) const;

        /// @brief Parse credentials data (salt:hashed_password format)
        /// @param credentials_data Raw credentials data from database
        /// @return Parsed salt and hashed password pair, nullopt if invalid format
        static std::optional<std::pair<std::string, std::string>> parse_credentials_data(const std::string& credentials_data);

        /// @brief Format credentials data (salt:hashed_password format)
        /// @param salt Salt string
        /// @param hashed_password Hashed password string
        /// @return Formatted credentials string
        static std::string format_credentials_data(const std::string& salt, const std::string& hashed_password);

        PasswordPolicy password_policy_;
        std::unordered_map<std::string, std::unique_ptr<UserCredentials>> users_;
        mutable std::mutex users_mutex_;
        sql::PasswordSQL password_sql_;
    };
}