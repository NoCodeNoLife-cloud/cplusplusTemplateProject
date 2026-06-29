/**
 * @file MySqlExecutor.hpp
 * @brief MySQL query executor with connection pooling and prepared statements
 * @description Executes SQL queries against a MySQL database using the MySQL
 *          C API (libmysqlclient).  Supports connection pooling, prepared
 *          statements with parameter binding, and result set iteration.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 */

#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

// MySQL Connector/C++ 8.0+ X DevAPI headers
#include <mysqlx/xdevapi.h>

namespace cppforge::sql::mysql
{
    /// @brief Represents a single value in a query result that can be NULL, integer, double, or string
    using QueryValue = std::variant<std::monostate, int64_t, double, std::string>;

    /// @brief Represents a single row in a query result with column names
    struct QueryRow
    {
        /// @brief Column data mapped by column name
        std::unordered_map<std::string, QueryValue> columns;

        /// @brief Get column value by name
        /// @param column_name Name of the column
        /// @return Optional value (nullopt if column doesn't exist)
        [[nodiscard]] std::optional<QueryValue> getColumn(const std::string& column_name) const
        {
            const auto it = columns.find(column_name);
            return it != columns.end() ? std::make_optional(it->second) : std::nullopt;
        }

        /// @brief Get column value as string by name
        /// @param column_name Name of the column
        /// @return String representation (empty string if NULL or not found)
        [[nodiscard]] std::string getString(const std::string& column_name) const
        {
            auto value = getColumn(column_name);
            if (!value.has_value())
            {
                return "";
            }

            return std::visit([]<typename T0>(const T0& arg) -> std::string
            {
                using T = std::decay_t<T0>;
                if constexpr (std::is_same_v<T, std::monostate>)
                {
                    return ""; // NULL value
                }
                else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, double>)
                {
                    return std::to_string(arg);
                }
                else if constexpr (std::is_same_v<T, std::string>)
                {
                    return arg;
                }
                return "";
            }, value.value());
        }

        /// @brief Check if a column exists and is not NULL
        /// @param column_name Name of the column
        /// @return true if column exists and has a non-NULL value
        [[nodiscard]] bool hasColumn(const std::string& column_name) const
        {
            const auto value = getColumn(column_name);
            if (!value.has_value())
            {
                return false;
            }
            return !std::holds_alternative<std::monostate>(value.value());
        }
    };

    /// @brief Complete query result with metadata
    struct QueryResult
    {
        /// @brief All rows in the result
        std::vector<QueryRow> rows;

        /// @brief Column names in order
        std::vector<std::string> column_names;

        /// @brief Number of rows
        [[nodiscard]] size_t rowCount() const
        {
            return rows.size();
        }

        /// @brief Number of columns
        [[nodiscard]] size_t columnCount() const
        {
            return column_names.size();
        }

        /// @brief Check if result is empty
        [[nodiscard]] bool isEmpty() const
        {
            return rows.empty();
        }
    };

    /// @brief MySQL database executor with RAII management using MySQL X DevAPI
    class MySqlExecutor
    {
    public:
        /// @brief Default constructor
        MySqlExecutor();

        /// @brief Constructor that connects to MySQL database
        /// @param host MySQL server host (default: localhost)
        /// @param port MySQL X Protocol port (default: 33060)
        /// @param user MySQL username
        /// @param password MySQL password
        /// @param database Database name
        /// @throws std::runtime_error if connection fails
        explicit MySqlExecutor(const std::string& host,
                               unsigned int port,
                               const std::string& user,
                               const std::string& password,
                               const std::string& database);

        /// @brief Destructor that automatically closes the database connection
        ~MySqlExecutor();

        /// @brief Copy constructor (deleted - database connections should not be copied)
        MySqlExecutor(const MySqlExecutor&) = delete;

        /// @brief Copy assignment operator (deleted - database connections should not be copied)
        MySqlExecutor& operator=(const MySqlExecutor&) = delete;

        /// @brief Move constructor
        /// @param other Another MySqlExecutor instance to move from
        MySqlExecutor(MySqlExecutor&& other) noexcept;

        /// @brief Move assignment operator
        /// @param other Another MySqlExecutor instance to move from
        /// @return Reference to this instance
        MySqlExecutor& operator=(MySqlExecutor&& other) noexcept;

        /// @brief Connect to MySQL database
        /// @param host MySQL server host
        /// @param port MySQL X Protocol port (typically 33060)
        /// @param user MySQL username
        /// @param password MySQL password
        /// @param database Database name
        /// @throws std::runtime_error if connection fails
        void connect(const std::string& host,
                     unsigned int port,
                     const std::string& user,
                     const std::string& password,
                     const std::string& database);

        /// @brief Close database session
        void disconnect();

        /// @brief Executes non-query SQL statements (INSERT/UPDATE/DELETE)
        /// @param sql SQL statement to execute
        /// @return Number of affected rows
        /// @throws std::runtime_error if execution fails
        [[nodiscard]] int execute(const std::string& sql);

        /// @brief Executes a query and returns results as a 2D string vector (legacy API)
        /// @param sql SQL query to execute
        /// @return Query results in format [rows][columns]
        /// @throws std::runtime_error if query fails
        /// @deprecated Use queryStructured() for better type safety and column name support
        [[nodiscard]] std::vector<std::vector<std::string>> query(const std::string& sql);

        /// @brief Executes a parameterized query and returns results as a 2D string vector (legacy API)
        /// @param sql SQL query with placeholders (?)
        /// @param params Parameter values (bound via X DevAPI prepared statement)
        /// @return Query results in format [rows][columns]
        /// @throws std::runtime_error if query fails
        /// @note Uses X DevAPI bind() for true server-side parameterization, immune to SQL injection.
        /// @deprecated Use queryWithParamsStructured() for better type safety and column name support
        [[nodiscard]] std::vector<std::vector<std::string>> queryWithParams(const std::string& sql,
                                                                             const std::vector<std::string>& params);

        /// @brief Executes a query and returns structured results with column names and typed values
        /// @param sql SQL query to execute
        /// @return Structured query result with metadata
        /// @throws std::runtime_error if query fails
        [[nodiscard]] QueryResult queryStructured(const std::string& sql);

        /// @brief Executes a parameterized query and returns structured results with column names and typed values
        /// @param sql SQL query with placeholders (?)
        /// @param params Parameter values (bound via X DevAPI prepared statement)
        /// @return Structured query result with metadata
        /// @throws std::runtime_error if query fails
        /// @note Uses X DevAPI bind() for true server-side parameterization, immune to SQL injection.
        [[nodiscard]] QueryResult queryWithParamsStructured(const std::string& sql,
                                                             const std::vector<std::string>& params);

        /// @brief Check if database session is valid
        /// @return true if connected, false otherwise
        [[nodiscard]] bool isConnected() const;

        /// @brief Get last error message
        /// @return Error message string
        [[nodiscard]] std::string getLastError() const;

    private:
        /// @brief MySQL session handle
        std::unique_ptr<mysqlx::Session> session_;

        /// @brief Connection state flag for efficient isConnected() check
        bool connected_ = false;

        /// @brief Last error message
        std::string last_error_;

        /// @brief Connection parameters
        std::string host_;
        unsigned int port_;
        std::string user_;
        std::string password_;
        std::string database_;

        /// @brief Constant for NULL value representation in query results
        static constexpr auto QUERY_RESULT_NULL = "NULL";

        // Note: QUERY_RESULT_ERROR constant removed as exceptions now propagate to caller
        // If needed in future, can be re-added for specific error handling scenarios

        /// @brief Extract a QueryValue from a mysqlx::Value by trying int64_t, double, then string
        /// @param value MySQL X DevAPI value
        /// @return Typed variant value
        [[nodiscard]] static QueryValue toQueryValue(const mysqlx::Value& value);

        /// @brief Process query result and convert to 2D string vector (legacy)
        /// @param result MySQL query result
        /// @return Query results in format [rows][columns]
        [[nodiscard]] static std::vector<std::vector<std::string>> processQueryResult(mysqlx::SqlResult& result);

        /// @brief Process query result into structured format with column names and typed values
        /// @param result MySQL query result
        /// @return Structured query result
        [[nodiscard]] static QueryResult processQueryResultStructured(mysqlx::SqlResult& result);
    };
}