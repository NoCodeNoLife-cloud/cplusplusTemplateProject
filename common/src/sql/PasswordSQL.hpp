#pragma once
#include <string>
#include <vector>

#include "sql/sqlite/SQLiteManager.hpp"

namespace common::sql {
/// @brief Manages user authentication and password operations using SQLite database
class PasswordSQL {
public:
    /// @brief Default constructor deleted to prevent uninitialized instances
    PasswordSQL() = delete;

    /// @brief Constructs PasswordSQL and initializes database connection
    /// @param db_path Path to the SQLite database file
    /// @throws std::runtime_error if database initialization fails
    explicit PasswordSQL(const std::string& db_path) noexcept(false);

    /// @brief Copy constructor deleted to prevent copying
    PasswordSQL(const PasswordSQL&) = delete;

    /// @brief Copy assignment operator deleted to prevent copying
    auto operator=(const PasswordSQL&) -> PasswordSQL& = delete;

    /// @brief Move constructor
    PasswordSQL(PasswordSQL&&) = delete;

    /// @brief Move assignment operator
    auto operator=(PasswordSQL&&) -> PasswordSQL& = delete;

    /// @brief Default destructor
    ~PasswordSQL() = default;

    /// @brief Registers a new user with username and password
    /// @param username Username to register
    /// @param password Password for the user
    /// @return true if registration successful, false otherwise
    [[nodiscard]] auto RegisterUser(const std::string& username, const std::string& password) const noexcept -> bool;

    /// @brief Authenticates a user with provided credentials
    /// @param username Username to authenticate
    /// @param password Password for the user
    /// @return true if authentication successful, false otherwise
    [[nodiscard]] auto AuthenticateUser(const std::string& username, const std::string& password) const noexcept -> bool;

    /// @brief Changes password for a user after validating old password
    /// @param username Username whose password needs to be changed
    /// @param old_password Current password for validation
    /// @param new_password New password to set
    /// @return true if password changed successfully, false otherwise
    [[nodiscard]] auto ChangePassword(const std::string& username, const std::string& old_password, const std::string& new_password) const noexcept -> bool;

    /// @brief Resets password for a user without validating old password
    /// @param username Username whose password needs to be reset
    /// @param new_password New password to set
    /// @return true if password reset successfully, false otherwise
    [[nodiscard]] auto ResetPassword(const std::string& username, const std::string& new_password) const noexcept -> bool;

    /// @brief Deletes a user from the database
    /// @param username Username to delete
    /// @return true if user deleted successfully, false otherwise
    [[nodiscard]] auto DeleteUser(const std::string& username) const noexcept -> bool;

    /// @brief Checks if a user exists in the database
    /// @param username Username to check
    /// @return true if user exists, false otherwise
    [[nodiscard]] auto UserExists(const std::string& username) const noexcept -> bool;

    /// @brief Retrieves a user's username from the database
    /// @param username Username to retrieve
    /// @return Username if found, empty string otherwise
    [[nodiscard]] auto GetUser(const std::string& username) const noexcept -> std::string;

    /// @brief Retrieves all usernames from the database
    /// @return Vector containing all usernames
    [[nodiscard]] auto GetAllUsers() const noexcept -> std::vector<std::string>;

private:
    /// @brief SQLite manager instance for database operations
    sqlite::SQLiteManager sqlite_manager_;
};
}
