#pragma once
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <optional>
#include <unordered_map>
#include <variant>

// MySQL Connector/C++ 8.0+ X DevAPI headers
#include <mysqlx/xdevapi.h>

namespace common::sql::mysql {
    /// @brief Represents a single value in a query result that can be NULL, integer, double, or string
    using QueryValue = std::variant<std::monostate, int64_t, double, std::string>;

    /// @brief Represents a single row in a query result with column names
    struct QueryRow {
        /// @brief Column data mapped by column name
        std::unordered_map<std::string, QueryValue> columns;

        /// @brief Get column value by name
        /// @param column_name Name of the column
        /// @return Optional value (nullopt if column doesn't exist)
        [[nodiscard]] auto getColumn(const std::string &column_name) const -> std::optional<QueryValue> {
            const auto it = columns.find(column_name);
            return (it != columns.end()) ? std::make_optional(it->second) : std::nullopt;
        }

        /// @brief Get column value as string by name
        /// @param column_name Name of the column
        /// @return String representation (empty string if NULL or not found)
        [[nodiscard]] auto getString(const std::string &column_name) const -> std::string {
            auto value = getColumn(column_name);
            if (!value.has_value()) {
                return "";
            }

            return std::visit([]<typename T0>(const T0 &arg) -> std::string {
                using T = std::decay_t<T0>;
                if constexpr (std::is_same_v<T, std::monostate>) {
                    return ""; // NULL value
                } else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, double>) {
                    return std::to_string(arg);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    return arg;
                }
                return "";
            }, value.value());
        }

        /// @brief Check if a column exists and is not NULL
        /// @param column_name Name of the column
        /// @return true if column exists and has a non-NULL value
        [[nodiscard]] auto hasColumn(const std::string &column_name) const -> bool {
            const auto value = getColumn(column_name);
            if (!value.has_value()) {
                return false;
            }
            return !std::holds_alternative<std::monostate>(value.value());
        }
    };

    /// @brief Complete query result with metadata
    struct QueryResult {
        /// @brief All rows in the result
        std::vector<QueryRow> rows;

        /// @brief Column names in order
        std::vector<std::string> column_names;

        /// @brief Number of rows
        [[nodiscard]] auto rowCount() const -> size_t { return rows.size(); }

        /// @brief Number of columns
        [[nodiscard]] auto columnCount() const -> size_t { return column_names.size(); }

        /// @brief Check if result is empty
        [[nodiscard]] auto isEmpty() const -> bool { return rows.empty(); }
    };

    /// @brief MySQL database executor with RAII management using MySQL X DevAPI
    class MySqlExecutor {
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
        explicit MySqlExecutor(const std::string &host,
                               unsigned int port,
                               const std::string &user,
                               const std::string &password,
                               const std::string &database);

        /// @brief Destructor that automatically closes the database connection
        ~MySqlExecutor();

        /// @brief Copy constructor (deleted - database connections should not be copied)
        MySqlExecutor(const MySqlExecutor &) = delete;

        /// @brief Copy assignment operator (deleted - database connections should not be copied)
        auto operator=(const MySqlExecutor &) -> MySqlExecutor & = delete;

        /// @brief Move constructor
        /// @param other Another MySqlExecutor instance to move from
        MySqlExecutor(MySqlExecutor &&other) noexcept;

        /// @brief Move assignment operator
        /// @param other Another MySqlExecutor instance to move from
        /// @return Reference to this instance
        auto operator=(MySqlExecutor &&other) noexcept -> MySqlExecutor &;

        /// @brief Connect to MySQL database
        /// @param host MySQL server host
        /// @param port MySQL X Protocol port (typically 33060)
        /// @param user MySQL username
        /// @param password MySQL password
        /// @param database Database name
        /// @throws std::runtime_error if connection fails
        void connect(const std::string &host,
                     unsigned int port,
                     const std::string &user,
                     const std::string &password,
                     const std::string &database);

        /// @brief Close database session
        void disconnect();

        /// @brief Executes non-query SQL statements (INSERT/UPDATE/DELETE)
        /// @param sql SQL statement to execute
        /// @return Number of affected rows
        /// @throws std::runtime_error if execution fails
        [[nodiscard]] auto execute(const std::string &sql) const -> int;

        /// @brief Executes a query and returns results as a 2D string vector (legacy API)
        /// @param sql SQL query to execute
        /// @return Query results in format [rows][columns]
        /// @throws std::runtime_error if query fails
        /// @deprecated Use queryStructured() for better type safety and column name support
        [[nodiscard]] auto query(const std::string &sql) const -> std::vector<std::vector<std::string> >;

        /// @brief Executes a parameterized query and returns results as a 2D string vector (legacy API)
        /// @param sql SQL query with placeholders (?)
        /// @param params Parameter values
        /// @return Query results in format [rows][columns]
        /// @throws std::runtime_error if query fails
        /// @deprecated Use queryWithParamsStructured() for better type safety and column name support
        [[nodiscard]] auto queryWithParams(const std::string &sql,
                                           const std::vector<std::string> &params) const -> std::vector<std::vector<std::string> >;

        /// @brief Executes a query and returns structured results with column names and typed values
        /// @param sql SQL query to execute
        /// @return Structured query result with metadata
        /// @throws std::runtime_error if query fails
        [[nodiscard]] auto queryStructured(const std::string &sql) const -> QueryResult;

        /// @brief Executes a parameterized query and returns structured results with column names and typed values
        /// @param sql SQL query with placeholders (?)
        /// @param params Parameter values
        /// @return Structured query result with metadata
        /// @throws std::runtime_error if query fails
        [[nodiscard]] auto queryWithParamsStructured(const std::string &sql,
                                                     const std::vector<std::string> &params) const -> QueryResult;

        /// @brief Check if database session is valid
        /// @return true if connected, false otherwise
        [[nodiscard]] auto isConnected() const -> bool;

        /// @brief Get last error message
        /// @return Error message string
        [[nodiscard]] auto getLastError() const -> std::string;

    private:
        /// @brief MySQL session handle
        std::unique_ptr<mysqlx::Session> session_;

        /// @brief Connection state flag for efficient isConnected() check
        bool connected_ = false;

        /// @brief Last error message
        mutable std::string last_error_;

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

        /// @brief Process query result and convert to 2D string vector (legacy)
        /// @param result MySQL query result
        /// @return Query results in format [rows][columns]
        [[nodiscard]] static auto processQueryResult(mysqlx::SqlResult &result) -> std::vector<std::vector<std::string> >;

        /// @brief Process query result into structured format with column names and typed values
        /// @param result MySQL query result
        /// @return Structured query result
        [[nodiscard]] static auto processQueryResultStructured(mysqlx::SqlResult &result) -> QueryResult;
    };
}
