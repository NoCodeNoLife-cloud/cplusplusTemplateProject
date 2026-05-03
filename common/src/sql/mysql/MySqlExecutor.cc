#include "MySqlExecutor.hpp"

#include <fmt/format.h>

namespace common::sql::mysql {
    MySqlExecutor::MySqlExecutor() : session_(nullptr),
                                     connected_(false),
                                     port_(33060) {  // X Protocol default port
    }

    MySqlExecutor::MySqlExecutor(const std::string &host,
                                 const unsigned int port,
                                 const std::string &user,
                                 const std::string &password,
                                 const std::string &database) : session_(nullptr),
                                                                connected_(false),
                                                                host_(host),
                                                                port_(port),
                                                                user_(user),
                                                                password_(password),
                                                                database_(database) {
        connect(host, port, user, password, database);
    }

    MySqlExecutor::~MySqlExecutor() {
        disconnect();
    }

    MySqlExecutor::MySqlExecutor(MySqlExecutor &&other) noexcept : session_(std::move(other.session_)),
                                                                   connected_(other.connected_),
                                                                   last_error_(std::move(other.last_error_)),
                                                                   host_(std::move(other.host_)),
                                                                   port_(other.port_),
                                                                   user_(std::move(other.user_)),
                                                                   password_(std::move(other.password_)),
                                                                   database_(std::move(other.database_)) {
        // Reset other's state since it's been moved
        other.connected_ = false;
        other.port_ = 33060;  // X Protocol default port
    }

    auto MySqlExecutor::operator=(MySqlExecutor &&other) noexcept -> MySqlExecutor & {
        if (this != &other) {
            // Close current connection if exists
            disconnect();

            // Move all members
            session_ = std::move(other.session_);
            connected_ = other.connected_;
            last_error_ = std::move(other.last_error_);
            host_ = std::move(other.host_);
            port_ = other.port_;
            user_ = std::move(other.user_);
            password_ = std::move(other.password_);
            database_ = std::move(other.database_);

            // Reset other's state since it's been moved
            other.connected_ = false;
            other.port_ = 33060;  // X Protocol default port
        }
        return *this;
    }

    void MySqlExecutor::connect(const std::string &host,
                                unsigned int port,
                                const std::string &user,
                                const std::string &password,
                                const std::string &database) {
        // Store connection parameters
        host_ = host;
        port_ = port;
        user_ = user;
        password_ = password;
        database_ = database;

        if (session_) {
            disconnect();
        }

        try {
            // Create session using X DevAPI
            session_ = std::make_unique<mysqlx::Session>(
                mysqlx::SessionOption::HOST, host,
                mysqlx::SessionOption::PORT, port,
                mysqlx::SessionOption::USER, user,
                mysqlx::SessionOption::PWD, password,
                mysqlx::SessionOption::DB, database
            );

            // Mark as connected after successful connection
            connected_ = true;
        } catch (const mysqlx::Error &e) {
            last_error_ = fmt::format("MySqlExecutor::connect: Connection failed - {}", e.what());
            disconnect();
            throw std::runtime_error(last_error_);
        }
    }

    void MySqlExecutor::disconnect() {
        if (session_) {
            try {
                session_->close();
            } catch (const mysqlx::Error &) {
                // Ignore exceptions during disconnect
            }
            session_.reset(nullptr);
        }
        // Mark as disconnected
        connected_ = false;
    }

    auto MySqlExecutor::execute(const std::string &sql) const -> int {
        if (!session_) {
            last_error_ = "MySqlExecutor::execute: Database not connected";
            throw std::runtime_error(last_error_);
        }

        if (sql.empty()) {
            last_error_ = "MySqlExecutor::execute: SQL statement cannot be empty";
            throw std::invalid_argument(last_error_);
        }

        try {
            // Execute SQL statement
            const auto result = session_->sql(sql).execute();

            // Get number of affected rows
            const auto affected_rows = result.getAffectedItemsCount();

            return static_cast<int>(affected_rows);
        } catch (const mysqlx::Error &e) {
            last_error_ = fmt::format("MySqlExecutor::execute: Query execution failed - {}", e.what());
            throw std::runtime_error(last_error_);
        }
    }

    auto MySqlExecutor::query(const std::string &sql) const -> std::vector<std::vector<std::string> > {
        if (!session_) {
            last_error_ = "MySqlExecutor::query: Database not connected";
            throw std::runtime_error(last_error_);
        }

        if (sql.empty()) {
            last_error_ = "MySqlExecutor::query: SQL statement cannot be empty";
            throw std::invalid_argument(last_error_);
        }

        try {
            // Execute SQL query
            auto result = session_->sql(sql).execute();

            // Process results using common method
            return processQueryResult(result);
        } catch (const mysqlx::Error &e) {
            last_error_ = fmt::format("MySqlExecutor::query: Query execution failed - {}", e.what());
            throw std::runtime_error(last_error_);
        }
    }

    auto MySqlExecutor::queryWithParams(const std::string &sql,
                                        const std::vector<std::string> &params) const -> std::vector<std::vector<std::string> > {
        if (!session_) {
            last_error_ = "MySqlExecutor::queryWithParams: Database not connected";
            throw std::runtime_error(last_error_);
        }

        if (sql.empty()) {
            last_error_ = "MySqlExecutor::queryWithParams: SQL statement cannot be empty";
            throw std::invalid_argument(last_error_);
        }

        try {
            // X DevAPI C++ doesn't support dynamic parameter binding well
            // Workaround: Manually replace ? placeholders with properly escaped values
            std::string processed_sql = sql;
            
            for (const auto& param : params) {
                // Escape single quotes by doubling them (SQL standard)
                std::string escaped_param;
                escaped_param.reserve(param.size() + 10);
                for (char c : param) {
                    if (c == '\'') {
                        escaped_param += "''";  // Escape single quote
                    } else if (c == '\\') {
                        escaped_param += "\\\\";  // Escape backslash
                    } else {
                        escaped_param += c;
                    }
                }
                
                // Find and replace the first ? placeholder
                auto qmark_pos = processed_sql.find('?');
                if (qmark_pos != std::string::npos) {
                    processed_sql.replace(qmark_pos, 1, "'" + escaped_param + "'");
                }
            }
            
            // Execute the processed query
            auto result = session_->sql(processed_sql).execute();

            // Process results using common method
            return processQueryResult(result);
        } catch (const mysqlx::Error &e) {
            last_error_ = fmt::format("MySqlExecutor::queryWithParams: Query execution failed - {}", e.what());
            throw std::runtime_error(last_error_);
        }
    }

    auto MySqlExecutor::queryStructured(const std::string &sql) const -> QueryResult {
        if (!session_) {
            last_error_ = "MySqlExecutor::queryStructured: Database not connected";
            throw std::runtime_error(last_error_);
        }

        if (sql.empty()) {
            last_error_ = "MySqlExecutor::queryStructured: SQL statement cannot be empty";
            throw std::invalid_argument(last_error_);
        }

        try {
            // Execute SQL query
            auto result = session_->sql(sql).execute();

            // Process results into structured format
            return processQueryResultStructured(result);
        } catch (const mysqlx::Error &e) {
            last_error_ = fmt::format("MySqlExecutor::queryStructured: Query execution failed - {}", e.what());
            throw std::runtime_error(last_error_);
        }
    }

    auto MySqlExecutor::queryWithParamsStructured(const std::string &sql,
                                                  const std::vector<std::string> &params) const -> QueryResult {
        if (!session_) {
            last_error_ = "MySqlExecutor::queryWithParamsStructured: Database not connected";
            throw std::runtime_error(last_error_);
        }

        if (sql.empty()) {
            last_error_ = "MySqlExecutor::queryWithParamsStructured: SQL statement cannot be empty";
            throw std::invalid_argument(last_error_);
        }

        try {
            // X DevAPI C++ doesn't support dynamic parameter binding well
            // Workaround: Manually replace ? placeholders with properly escaped values
            std::string processed_sql = sql;
            
            for (const auto& param : params) {
                // Escape single quotes by doubling them (SQL standard)
                std::string escaped_param;
                escaped_param.reserve(param.size() + 10);
                for (char c : param) {
                    if (c == '\'') {
                        escaped_param += "''";  // Escape single quote
                    } else if (c == '\\') {
                        escaped_param += "\\\\";  // Escape backslash
                    } else {
                        escaped_param += c;
                    }
                }
                
                // Find and replace the first ? placeholder
                auto qmark_pos = processed_sql.find('?');
                if (qmark_pos != std::string::npos) {
                    processed_sql.replace(qmark_pos, 1, "'" + escaped_param + "'");
                }
            }
            
            // Execute the processed query
            auto result = session_->sql(processed_sql).execute();

            // Process results into structured format
            return processQueryResultStructured(result);
        } catch (const mysqlx::Error &e) {
            last_error_ = fmt::format("MySqlExecutor::queryWithParamsStructured: Query execution failed - {}", e.what());
            throw std::runtime_error(last_error_);
        }
    }

    auto MySqlExecutor::isConnected() const -> bool {
        // Fast check using flag - no database query needed
        return connected_ && session_;
    }

    auto MySqlExecutor::getLastError() const -> std::string {
        return last_error_;
    }

    auto MySqlExecutor::processQueryResult(mysqlx::SqlResult &result) -> std::vector<std::vector<std::string> > {
        std::vector<std::vector<std::string> > results;

        // Iterate through rows
        mysqlx::Row row;
        while ((row = result.fetchOne())) {
            std::vector<std::string> row_data;
            const auto num_columns = row.colCount();
            row_data.reserve(num_columns);

            for (size_t i = 0; i < num_columns; ++i) {
                // Check if value is NULL
                if (row[i].isNull()) {
                    row_data.emplace_back(QUERY_RESULT_NULL);
                } else {
                    // Convert value to string based on its type
                    try {
                        // Try to get as string first (works for text types)
                        row_data.emplace_back(row[i].get<std::string>());
                    } catch (const mysqlx::Error &) {
                        // If not a string, try numeric types and convert to string
                        try {
                            const auto int_val = row[i].get<int64_t>();
                            row_data.emplace_back(std::to_string(int_val));
                        } catch (const mysqlx::Error &) {
                            try {
                                const auto double_val = row[i].get<double>();
                                row_data.emplace_back(std::to_string(double_val));
                            } catch (const mysqlx::Error &) {
                                // Fallback: use generic conversion
                                row_data.emplace_back("UNKNOWN_TYPE");
                            }
                        }
                    }
                }
            }
            results.push_back(std::move(row_data));
        }

        return results;
    }

    auto MySqlExecutor::processQueryResultStructured(mysqlx::SqlResult &result) -> QueryResult {
        QueryResult query_result;

        // Get column names from the first row's column labels
        // Note: We need to fetch one row to get column information, then prepend it

        if (mysqlx::Row first_row = result.fetchOne()) {
            // Extract column names from the first row
            const auto num_columns = first_row.colCount();
            for (size_t i = 0; i < num_columns; ++i) {
                // MySQL X DevAPI doesn't provide direct column name access in SqlResult
                // We'll use generic column names or extract from SQL if needed
                // For now, use index-based names (can be improved with SQL parsing)
                query_result.column_names.push_back("column_" + std::to_string(i));
            }

            // Process the first row
            QueryRow query_row;
            for (size_t i = 0; i < num_columns; ++i) {
                const std::string &column_name = query_result.column_names[i];

                try {
                    if (first_row[i].isNull()) {
                        query_row.columns[column_name] = std::monostate{};
                    } else {
                        const auto &value = first_row[i];

                        // Try integer first
                        try {
                            const auto int_val = value.get<int64_t>();
                            query_row.columns[column_name] = int_val;
                            continue;
                        } catch (const mysqlx::Error &) {
                            // Not an integer, try next type
                        }

                        // Try double
                        try {
                            const auto double_val = value.get<double>();
                            query_row.columns[column_name] = double_val;
                            continue;
                        } catch (const mysqlx::Error &) {
                            // Not a double, try string
                        }

                        // Default to string
                        query_row.columns[column_name] = value.get<std::string>();
                    }
                } catch (const mysqlx::Error &) {
                    query_row.columns[column_name] = std::monostate{};
                }
            }
            query_result.rows.push_back(std::move(query_row));
        }

        // Process remaining rows
        mysqlx::Row row;
        while ((row = result.fetchOne())) {
            QueryRow query_row;
            const auto num_columns = row.colCount();

            for (size_t i = 0; i < num_columns; ++i) {
                const std::string &column_name = (i < query_result.column_names.size())
                                                     ? query_result.column_names[i]
                                                     : "column_" + std::to_string(i);

                try {
                    if (row[i].isNull()) {
                        query_row.columns[column_name] = std::monostate{};
                    } else {
                        const auto &value = row[i];

                        // Try integer first
                        try {
                            const auto int_val = value.get<int64_t>();
                            query_row.columns[column_name] = int_val;
                            continue;
                        } catch (const mysqlx::Error &) {
                            // Not an integer, try next type
                        }

                        // Try double
                        try {
                            const auto double_val = value.get<double>();
                            query_row.columns[column_name] = double_val;
                            continue;
                        } catch (const mysqlx::Error &) {
                            // Not a double, try string
                        }

                        // Default to string
                        query_row.columns[column_name] = value.get<std::string>();
                    }
                } catch (const mysqlx::Error &) {
                    query_row.columns[column_name] = std::monostate{};
                }
            }

            query_result.rows.push_back(std::move(query_row));
        }

        return query_result;
    }
}
