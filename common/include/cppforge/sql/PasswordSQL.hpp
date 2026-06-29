/**
 * @file PasswordSQL.hpp
 * @brief SQL query builder for password-related database operations
 * @description Builds parameterised SQL queries for password management:
 *          storing password hashes, verifying credentials, updating passwords,
 *          and querying password history.  Prevents SQL injection via
 *          parameterised query templates.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 */

#pragma once
#include <string>
#include <vector>

#include <cppforge/sql/sqlite/SQLiteManager.hpp>

namespace cppforge::sql
{
    /// @brief Manages user authentication and password operations using SQLite database
    class PasswordSQL
    {
    public:
        /// @brief Default constructor deleted to prevent uninitialized instances
        PasswordSQL() = delete;

        /// @brief Constructs PasswordSQL and initializes database connection
        /// @param db_path Path to the SQLite database file
        /// @throws std::runtime_error if database initialization fails
        explicit PasswordSQL(const std::string& db_path);

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
        [[nodiscard]] bool RegisterUser(const std::string& username, const std::string& password) const;

        /// @brief Authenticates a user with provided credentials
        [[nodiscard]] bool AuthenticateUser(const std::string& username, const std::string& password) const;

        /// @brief Changes password for a user after validating old password
        [[nodiscard]] bool ChangePassword(const std::string& username, const std::string& old_password, const std::string& new_password) const;

        /// @brief Resets password for a user without validating old password
        [[nodiscard]] bool ResetPassword(const std::string& username, const std::string& new_password) const;

        /// @brief Deletes a user from the database
        [[nodiscard]] bool DeleteUser(const std::string& username) const;

        /// @brief Checks if a user exists in the database
        [[nodiscard]] bool UserExists(const std::string& username) const;

        /// @brief Retrieves a user's username from the database
        [[nodiscard]] std::string GetUser(const std::string& username) const;

        /// @brief Retrieves all usernames from the database
        [[nodiscard]] std::vector<std::string> GetAllUsers() const;

    private:
        /// @brief Check if any parameter in a list is empty
        static bool HasAnyEmpty(const std::initializer_list<std::string>& params);

        sqlite::SQLiteManager sqlite_manager_;
    };
}
