/**
 * @file SQLiteManager.hpp
 * @brief SQLiteManager class declaration
 * @details This header defines the SQLiteManager class that provides functionality for SQL database utilities.
 */

#pragma once
#include <sqlite3.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>

namespace common::sql::sqlite {
/// @brief SQLite database executor with RAII management and parameterized queries
class SQLiteManager {
public:
    /// @brief Default constructor
    SQLiteManager();

    /// @brief Constructor that opens the database file (creates it if not exists)
    /// @param db_path Path to the SQLite database file
    /// @throws std::runtime_error if database cannot be opened
    explicit SQLiteManager(const std::string& db_path);

    /// @brief Destructor that automatically closes the database connection
    ~SQLiteManager();

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
    [[nodiscard]] auto exec(const std::string& sql, const std::vector<std::string>& params = {}) const -> int;

    /// @brief Executes a query and returns results as a 2D string vector
    /// @param sql SQL query to execute
    /// @param params Parameter values for prepared statement
    /// @return Query results in format [rows][columns]
    /// @throws std::runtime_error if query fails
    [[nodiscard]] auto query(const std::string& sql, const std::vector<std::string>& params = {}) const -> std::vector<std::vector<std::string> >;

    /// @brief Check if database is open
    /// @return true if database is open, false otherwise
    [[nodiscard]] auto isOpen() const -> bool;

private:
    /// @brief Helper function to bind parameters to a prepared statement
    /// @param stmt SQLite statement to bind parameters to
    /// @param params Vector of parameter values to bind
    /// @param method_name Name of the calling method for error reporting
    /// @throws std::runtime_error if parameter binding fails
    static void bindParameters(sqlite3_stmt* stmt, const std::vector<std::string>& params, const std::string& method_name);

    std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db_;
};
} // common