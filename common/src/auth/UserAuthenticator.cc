/**
 * @file UserAuthenticator.cc
 * @brief UserAuthenticator implementation â€?login, session, credential store
 * @details Implements user authentication: credential validation against store,
 *          session token generation (UUID-based), and login/logout lifecycle.
 */

#include "UserAuthenticator.hpp"

#include <regex>
#include <sstream>
#include <fmt/format.h>
#include <glog/logging.h>

#include "crypto/CryptoToolKit.hpp"
#include "auth/AuthenticationException.hpp"

namespace
{
    bool validate_username(const std::string& username)
    {
        // Allow letters, numbers, underscores, hyphens; 3-20 characters
        const std::regex username_pattern("^[a-zA-Z0-9_-]{3,20}$");
        return std::regex_match(username, username_pattern);
    }

    std::optional<std::pair<std::string, std::string>> parse_credentials_data(const std::string& credentials_data)
    {
        const size_t delimiter_pos = credentials_data.find(':');
        if (delimiter_pos == std::string::npos)
        {
            return std::nullopt;
        }

        const std::string salt = credentials_data.substr(0, delimiter_pos);
        const std::string hashed_password = credentials_data.substr(delimiter_pos + 1);
        return std::make_pair(salt, hashed_password);
    }

    std::string format_credentials_data(const std::string& salt, const std::string& hashed_password)
    {
        std::ostringstream credential_stream;
        credential_stream << salt << ":" << hashed_password;
        return credential_stream.str();
    }
}

namespace cppforge::auth
{
    UserAuthenticator::UserAuthenticator(const std::string& db_path, const PasswordPolicy& policy) : password_policy_(policy), password_sql_(db_path)
    {
    }

    bool UserAuthenticator::register_user(const std::string& username, const std::string& password)
    {
        DLOG(INFO) << "Registering new user: " << username;
        std::lock_guard lock(users_mutex_);

        // Validate username format
        if (!validate_username(username))
        {
            DLOG(WARNING) << "Registration failed: invalid username format for: " << username;
            throw AuthenticationException(std::string("Invalid username format. Use alphanumeric characters, underscores, or hyphens (3-20 characters)."));
        }

        // Check if username already exists
        if (users_.contains(username) || password_sql_.UserExists(username))
        {
            DLOG(WARNING) << "Registration failed: username already exists: " << username;
            throw AuthenticationException(std::string("Username already exists"));
        }

        // Validate password against policy
        if (!password_policy_.validate(password))
        {
            DLOG(WARNING) << "Registration failed: password does not meet security requirements for user: " << username;
            throw AuthenticationException(std::string("Password does not meet security requirements"));
        }

        // Generate salt and hash password
        auto salt = crypto::CryptoToolKit::generate_salt();
        auto hashed_password = crypto::CryptoToolKit::hash_password(password, salt);

        // Store user credentials in database
        const std::string credential_data = format_credentials_data(salt, hashed_password);
        if (!password_sql_.RegisterUser(username, credential_data))
        {
            throw AuthenticationException(std::string("Failed to register user in database"));
        }

        // Store user credentials in memory cache
        users_[username] = std::make_unique<UserCredentials>(username, hashed_password, salt);
        DLOG(INFO) << "User registered successfully: " << username;
        return true;
    }

    AuthResult UserAuthenticator::authenticate(const std::string& username, const std::string& password)
    {
        DLOG(INFO) << "Authenticating user: " << username;
        std::lock_guard lock(users_mutex_);

        // Try to get user from memory cache first
        auto it = users_.find(username);
        if (it == users_.end())
        {
            DLOG(INFO) << "User not in cache, loading from database: " << username;
            // If not in cache, try to load from database
            auto user_opt = load_user_from_db(username);
            if (user_opt.has_value())
            {
                // Add to cache
                users_[username] = std::make_unique<UserCredentials>(user_opt.value());
                it = users_.find(username);
            }
        }

        if (it == users_.end())
        {
            DLOG(INFO) << "Authentication failed: user not found: " << username;
            return AuthResult::failure_result("User not found");
        }

        const auto& user = it->second;

        // Check if account is locked
        if (user->is_locked())
        {
            DLOG(WARNING) << "Authentication failed: account is locked: " << username;
            return AuthResult::failure_result("Account is locked due to too many failed attempts. Please try again later.");
        }

        // Verify password
        const auto hashed_input = crypto::CryptoToolKit::hash_password(password, user->get_salt());
        if (crypto::CryptoToolKit::secure_compare(hashed_input, user->get_hashed_password()))
        {
            user->reset_failed_attempts();
            DLOG(INFO) << "Authentication successful for user: " << username;
            return AuthResult::success_result(user.get());
        }

        user->increment_failed_attempts();
        DLOG(INFO) << "Authentication failed: invalid password for user: " << username
            << " (failed attempts: " << user->get_failed_attempts() << ")";
        return AuthResult::failure_result("Invalid password");
    }

    bool UserAuthenticator::change_password(const std::string& username, const std::string& current_password, const std::string& new_password)
    {
        DLOG(INFO) << "Changing password for user: " << username;
        // First verify current password
        const auto auth_result = authenticate(username, current_password);
        if (!auth_result.is_success())
        {
            DLOG(WARNING) << "Password change failed: current password incorrect for user: " << username;
            throw AuthenticationException("Current password is incorrect: " + auth_result.error_message);
        }

        std::lock_guard lock(users_mutex_);
        const auto it = users_.find(username);
        if (it == users_.end())
        {
            throw AuthenticationException(std::string("User not found"));
        }

        // Validate new password
        if (!password_policy_.validate(new_password))
        {
            throw AuthenticationException(std::string("New password does not meet security requirements"));
        }

        // Generate new salt and hash
        auto salt = crypto::CryptoToolKit::generate_salt();
        auto hashed_password = crypto::CryptoToolKit::hash_password(new_password, salt);

        // Update credentials in database
        const std::string credential_data = format_credentials_data(salt, hashed_password);
        if (!password_sql_.ResetPassword(username, credential_data))
        {
            throw AuthenticationException(std::string("Failed to update password in database"));
        }

        // Update credentials in memory cache
        auto new_credentials = std::make_unique<UserCredentials>(username, hashed_password, salt);
        it->second = std::move(new_credentials);
        DLOG(INFO) << "Password changed successfully for user: " << username;
        return true;
    }

    bool UserAuthenticator::reset_password(const std::string& username, const std::string& new_password)
    {
        DLOG(INFO) << "Resetting password for user: " << username;
        std::lock_guard lock(users_mutex_);

        // Validate new password
        if (!password_policy_.validate(new_password))
        {
            DLOG(WARNING) << "Password reset failed: new password does not meet requirements for user: " << username;
            throw AuthenticationException(std::string("New password does not meet security requirements"));
        }

        // Generate new credentials
        auto salt = crypto::CryptoToolKit::generate_salt();
        auto hashed_password = crypto::CryptoToolKit::hash_password(new_password, salt);

        // Update credentials in database
        const std::string credential_data = format_credentials_data(salt, hashed_password);
        if (!password_sql_.ResetPassword(username, credential_data))
        {
            throw AuthenticationException(std::string("Failed to reset password in database"));
        }

        // Update credentials in memory cache or add if not exists
        auto new_credentials = std::make_unique<UserCredentials>(username, hashed_password, salt);
        users_[username] = std::move(new_credentials);
        DLOG(INFO) << "Password reset successfully for user: " << username;
        return true;
    }

    bool UserAuthenticator::user_exists(const std::string& username) const
    {
        std::lock_guard lock(users_mutex_);
        // Check in memory cache first
        if (users_.contains(username))
        {
            return true;
        }
        // Check in database
        return password_sql_.UserExists(username);
    }

    bool UserAuthenticator::delete_user(const std::string& username)
    {
        DLOG(INFO) << "Deleting user: " << username;
        std::lock_guard lock(users_mutex_);

        // Delete from database
        if (!password_sql_.DeleteUser(username))
        {
            DLOG(WARNING) << "Failed to delete user from database: " << username;
            return false;
        }

        // Delete from memory cache
        users_.erase(username);
        DLOG(INFO) << "User deleted successfully: " << username;
        return true;
    }

    void UserAuthenticator::set_password_policy(const PasswordPolicy& policy)
    {
        password_policy_ = policy;
    }

    std::unordered_map<std::string, std::unique_ptr<UserCredentials>>& UserAuthenticator::get_users()
    {
        return users_;
    }

    std::mutex& UserAuthenticator::get_users_mutex() const
    {
        return users_mutex_;
    }

    std::optional<UserCredentials> UserAuthenticator::load_user_from_db(const std::string& username) const
    {
        if (!password_sql_.UserExists(username))
        {
            return std::nullopt;
        }

        const std::string credentials_data = password_sql_.GetUser(username);
        if (credentials_data.empty())
        {
            return std::nullopt;
        }

        // Parse credentials data (salt:hashed_password)
        const auto parsed_credentials = parse_credentials_data(credentials_data);
        if (!parsed_credentials.has_value())
        {
            return std::nullopt;
        }

        const auto& [salt, hashed_password] = parsed_credentials.value();
        return UserCredentials(username, hashed_password, salt);
    }
}
