/**
 * @file SQLiteManager.hpp
 * @brief SQLite database manager — connection, query, transaction control
 * @description Manages a SQLite database connection.  Provides open/close,
 *          DDL execution (CREATE TABLE, etc.), DML (INSERT, SELECT), and
 *          transaction management (BEGIN, COMMIT, ROLLBACK).  Wraps the
 *          sqlite3 C API.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  SQLite in single-thread mode requires
 * external synchronisation for concurrent access.
 */

#pragma once
#include <memory>
#include <sqlite3.h>
#include <string>
#include <string_view>
#include <vector>

namespace common::sql::sqlite
{
    /// @brief SQLite database executor with RAII management and parameterized queries
    class SQLiteManager
    {
    public:
        /// @brief Default constructor
        SQLiteManager();

        /// @brief Constructor that opens the database file (creates it if not exists)
        /// @param db_path Path to the SQLite database file
        /// @throws std::runtime_error if database cannot be opened
        explicit SQLiteManager(const std::string& db_path);

        /// @brief Destructor that automatically closes the database connection
        ~SQLiteManager();

        /// @brief Move constructor
        SQLiteManager(SQLiteManager&& other) noexcept = default;

        /// @brief Move assignment operator
        auto operator=(SQLiteManager&& other) noexcept -> SQLiteManager& = default;

        /// @brief Copy constructor (deleted)
        SQLiteManager(const SQLiteManager&) = delete;

        /// @brief Copy assignment (deleted)
        auto operator=(const SQLiteManager&) -> SQLiteManager& = delete;

        /// @brief Creates/open database connection
        /// @param db_path Path to the SQLite database file
        /// @throws std::runtime_error if database cannot be opened
        void createDatabase(const std::string& db_path);

        /// @brief Closes database connection
        void closeDatabase();

        /// @brief Executes non-query SQL statements (INSERT/UPDATE/DELETE)
        /// @param sql SQL statement to execute
        /// @param params Parameter values for prepared statement
        /// @return Number of affected rows
        /// @throws std::runtime_error if execution fails
        [[nodiscard]] int exec(const std::string& sql, const std::vector<std::string>& params = {}) const;

        /// @brief Executes a query and returns results as a 2D string vector
        /// @param sql SQL query to execute
        /// @param params Parameter values for prepared statement
        /// @return Query results in format [rows][columns]
        /// @throws std::runtime_error if query fails
        [[nodiscard]] std::vector<std::vector<std::string>> query(const std::string& sql, const std::vector<std::string>& params = {}) const;

        /// @brief Check if database is open
        /// @return true if database is open, false otherwise
        [[nodiscard]] bool isOpen() const;

    private:
        /// @brief Helper function to bind parameters to a prepared statement
        /// @param stmt SQLite statement to bind parameters to
        /// @param params Vector of parameter values to bind
        /// @param method_name Name of the calling method for error reporting
        /// @throws std::runtime_error if parameter binding fails
        static void bindParameters(sqlite3_stmt* stmt, const std::vector<std::string>& params, std::string_view method_name);

        std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db_;
    };
} // common