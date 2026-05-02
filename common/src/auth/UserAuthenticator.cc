#include "UserAuthenticator.hpp"

#include <fmt/format.h>
#include "src/exception/AuthenticationException.hpp"
#include <sstream>

#include "crypto/CryptoToolKit.hpp"

namespace common::auth {
    UserAuthenticator::UserAuthenticator(const std::string &db_path, const PasswordPolicy &policy) : password_policy_(policy), password_sql_(db_path) {
    }

    /// @brief Format credentials data (salt:hashed_password format)
    /// @param salt Salt string
    /// @param hashed_password Hashed password string
    /// @return Formatted credentials string
    auto UserAuthenticator::format_credentials_data(const std::string &salt, const std::string &hashed_password) -> std::string {
        std::ostringstream credential_stream;
        credential_stream << salt << ":" << hashed_password;
        return credential_stream.str();
    }

    bool UserAuthenticator::register_user(const std::string &username, const std::string &password) {
        std::lock_guard lock(users_mutex_);

        // Validate username format
        if (!validate_username(username)) {
            throw exception::AuthenticationException(std::string("Invalid username format. Use alphanumeric characters, underscores, or hyphens (3-20 characters)."));
        }

        // Check if username already exists
        if (users_.contains(username) || password_sql_.UserExists(username)) {
            throw exception::AuthenticationException(std::string("Username already exists"));
        }

        // Validate password against policy
        if (!password_policy_.validate(password)) {
            throw exception::AuthenticationException(std::string("Password does not meet security requirements"));
        }

        // Generate salt and hash password
        auto salt = crypto::CryptoToolKit::generate_salt();
        auto hashed_password = crypto::CryptoToolKit::hash_password(password, salt);

        // Store user credentials in database
        const std::string credential_data = format_credentials_data(salt, hashed_password);
        if (!password_sql_.RegisterUser(username, credential_data)) {
            throw exception::AuthenticationException(std::string("Failed to register user in database"));
        }

        // Store user credentials in memory cache
        users_[username] = std::make_unique<UserCredentials>(username, hashed_password, salt);
        return true;
    }

    bool UserAuthenticator::authenticate(const std::string &username, const std::string &password) {
        std::lock_guard lock(users_mutex_);

        // Try to get user from memory cache first
        auto it = users_.find(username);
        if (it == users_.end()) {
            // If not in cache, try to load from database
            auto user_opt = load_user_from_db(username);
            if (user_opt.has_value()) {
                // Add to cache
                users_[username] = std::make_unique<UserCredentials>(user_opt.value());
                it = users_.find(username);
            }
        }

        if (it == users_.end()) {
            throw exception::AuthenticationException(std::string("User not found"));
        }

        const auto &user = it->second;

        // Check if account is locked
        if (user->is_locked()) {
            throw exception::AuthenticationException(std::string("Account is locked due to too many failed attempts. Please try again later."));
        }

        // Verify password
        if (const auto hashed_input = crypto::CryptoToolKit::hash_password(password, user->get_salt()); crypto::CryptoToolKit::secure_compare(hashed_input, user->get_hashed_password())) {
            user->reset_failed_attempts();
            return true;
        }
        user->increment_failed_attempts();
        throw exception::AuthenticationException(std::string("Invalid password"));
    }

    bool UserAuthenticator::change_password(const std::string &username, const std::string &current_password, const std::string &new_password) {
        // First verify current password
        try {
            authenticate(username, current_password);
        } catch (const exception::AuthenticationException &) {
            throw exception::AuthenticationException(std::string("Current password is incorrect"));
        }

        std::lock_guard lock(users_mutex_);
        const auto it = users_.find(username);
        if (it == users_.end()) {
            throw exception::AuthenticationException(std::string("User not found"));
        }

        // Validate new password
        if (!password_policy_.validate(new_password)) {
            throw exception::AuthenticationException(std::string("New password does not meet security requirements"));
        }

        // Generate new salt and hash
        auto salt = crypto::CryptoToolKit::generate_salt();
        auto hashed_password = crypto::CryptoToolKit::hash_password(new_password, salt);

        // Update credentials in database
        const std::string credential_data = format_credentials_data(salt, hashed_password);
        if (!password_sql_.ResetPassword(username, credential_data)) {
            throw exception::AuthenticationException(std::string("Failed to update password in database"));
        }

        // Update credentials in memory cache
        auto new_credentials = std::make_unique<UserCredentials>(username, hashed_password, salt);
        it->second = std::move(new_credentials);
        return true;
    }

    bool UserAuthenticator::reset_password(const std::string &username, const std::string &new_password) {
        std::lock_guard lock(users_mutex_);

        // Validate new password
        if (!password_policy_.validate(new_password)) {
            throw exception::AuthenticationException(std::string("New password does not meet security requirements"));
        }

        // Generate new credentials
        auto salt = crypto::CryptoToolKit::generate_salt();
        auto hashed_password = crypto::CryptoToolKit::hash_password(new_password, salt);

        // Update credentials in database
        const std::string credential_data = format_credentials_data(salt, hashed_password);
        if (!password_sql_.ResetPassword(username, credential_data)) {
            throw exception::AuthenticationException(std::string("Failed to reset password in database"));
        }

        // Update credentials in memory cache or add if not exists
        auto new_credentials = std::make_unique<UserCredentials>(username, hashed_password, salt);
        users_[username] = std::move(new_credentials);
        return true;
    }

    bool UserAuthenticator::delete_user(const std::string &username) {
        std::lock_guard lock(users_mutex_);

        // Delete from database
        if (!password_sql_.DeleteUser(username)) {
            return false;
        }

        // Delete from memory cache
        users_.erase(username);
        return true;
    }

    bool UserAuthenticator::user_exists(const std::string &username) const {
        std::lock_guard lock(users_mutex_);
        // Check in memory cache first
        if (users_.contains(username)) {
            return true;
        }
        // Check in database
        return password_sql_.UserExists(username);
    }

    void UserAuthenticator::set_password_policy(const PasswordPolicy &policy) {
        password_policy_ = policy;
    }

    auto UserAuthenticator::validate_username(const std::string &username) noexcept -> bool {
        // Allow letters, numbers, underscores, hyphens; 3-20 characters
        const std::regex username_pattern("^[a-zA-Z0-9_-]{3,20}$");
        return std::regex_match(username, username_pattern);
    }

    /// @brief Parse credentials data (salt:hashed_password format)
    /// @param credentials_data Raw credentials data from database
    /// @return Parsed salt and hashed password pair, nullopt if invalid format
    auto UserAuthenticator::parse_credentials_data(const std::string &credentials_data) -> std::optional<std::pair<std::string, std::string> > {
        const size_t delimiter_pos = credentials_data.find(':');
        if (delimiter_pos == std::string::npos) {
            return std::nullopt;
        }

        const std::string salt = credentials_data.substr(0, delimiter_pos);
        const std::string hashed_password = credentials_data.substr(delimiter_pos + 1);
        return std::make_pair(salt, hashed_password);
    }

    auto UserAuthenticator::load_user_from_db(const std::string &username) const -> std::optional<UserCredentials> {
        if (!password_sql_.UserExists(username)) {
            return std::nullopt;
        }

        const std::string credentials_data = password_sql_.GetUser(username);
        if (credentials_data.empty()) {
            return std::nullopt;
        }

        // Parse credentials data (salt:hashed_password)
        const auto parsed_credentials = parse_credentials_data(credentials_data);
        if (!parsed_credentials.has_value()) {
            return std::nullopt;
        }

        const auto &[salt, hashed_password] = parsed_credentials.value();
        return UserCredentials(username, hashed_password, salt);
    }

    auto UserAuthenticator::get_users() -> std::unordered_map<std::string, std::unique_ptr<UserCredentials> > & {
        return users_;
    }

    auto UserAuthenticator::get_users_mutex() const -> std::mutex & {
        return users_mutex_;
    }
} // common
