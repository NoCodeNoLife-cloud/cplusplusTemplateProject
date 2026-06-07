/**
 * @file MySqlExecutor.cc
 * @brief MySqlExecutor class implementation
 * @details This file contains the implementation of the MySqlExecutor class methods for SQL database utilities.
 */

#include "MySqlExecutor.hpp"

#include <fmt/format.h>
#include <glog/logging.h>

namespace common::sql::mysql
{
    MySqlExecutor::MySqlExecutor() : session_(nullptr),
                                     port_(33060)
    {
        // X Protocol default port
    }

    MySqlExecutor::MySqlExecutor(const std::string& host,
                                 const unsigned int port,
                                 const std::string& user,
                                 const std::string& password,
                                 const std::string& database) : session_(nullptr),
                                                                host_(host),
                                                                port_(port),
                                                                user_(user),
                                                                password_(password),
                                                                database_(database)
    {
        connect(host, port, user, password, database);
    }

    MySqlExecutor::~MySqlExecutor()
    {
        disconnect();
    }

    MySqlExecutor::MySqlExecutor(MySqlExecutor&& other) noexcept : session_(std::move(other.session_)),
                                                                  connected_(other.connected_),
                                                                  last_error_(std::move(other.last_error_)),
                                                                  host_(std::move(other.host_)),
                                                                  port_(other.port_),
                                                                  user_(std::move(other.user_)),
                                                                  password_(std::move(other.password_)),
                                                                  database_(std::move(other.database_))
    {
        other.connected_ = false;
        other.port_ = 33060;
    }

    auto MySqlExecutor::operator=(MySqlExecutor&& other) noexcept -> MySqlExecutor&
    {
        if (this != &other)
        {
            disconnect();

            session_ = std::move(other.session_);
            connected_ = other.connected_;
            last_error_ = std::move(other.last_error_);
            host_ = std::move(other.host_);
            port_ = other.port_;
            user_ = std::move(other.user_);
            password_ = std::move(other.password_);
            database_ = std::move(other.database_);

            other.connected_ = false;
            other.port_ = 33060;
        }
        return *this;
    }

    void MySqlExecutor::connect(const std::string& host,
                                unsigned int port,
                                const std::string& user,
                                const std::string& password,
                                const std::string& database)
    {
        host_ = host;
        port_ = port;
        user_ = user;
        password_ = password;
        database_ = database;

        if (session_)
        {
            disconnect();
        }

        try
        {
            session_ = std::make_unique<mysqlx::Session>(
                mysqlx::SessionOption::HOST, host,
                mysqlx::SessionOption::PORT, port,
                mysqlx::SessionOption::USER, user,
                mysqlx::SessionOption::PWD, password,
                mysqlx::SessionOption::DB, database
            );

            connected_ = true;
        }
        catch (const mysqlx::Error& e)
        {
            last_error_ = fmt::format("MySqlExecutor::connect: Connection failed - {}", e.what());
            DLOG(WARNING) << last_error_;
            disconnect();
            throw std::runtime_error(last_error_);
        }
    }

    void MySqlExecutor::disconnect()
    {
        if (session_)
        {
            try
            {
                session_->close();
            }
            catch (const mysqlx::Error&)
            {
                // Ignore exceptions during disconnect
            }
            session_.reset(nullptr);
        }
        connected_ = false;
    }

    int MySqlExecutor::execute(const std::string& sql)
    {
        if (!session_)
        {
            last_error_ = "MySqlExecutor::execute: Database not connected";
            DLOG(WARNING) << last_error_;
            throw std::runtime_error(last_error_);
        }

        if (sql.empty())
        {
            last_error_ = "MySqlExecutor::execute: SQL statement cannot be empty";
            DLOG(WARNING) << last_error_;
            throw std::invalid_argument(last_error_);
        }

        try
        {
            const auto result = session_->sql(sql).execute();
            const auto affected_rows = result.getAffectedItemsCount();
            return static_cast<int>(affected_rows);
        }
        catch (const mysqlx::Error& e)
        {
            last_error_ = fmt::format("MySqlExecutor::execute: Query execution failed - {}", e.what());
            throw std::runtime_error(last_error_);
        }
    }

    std::vector<std::vector<std::string>> MySqlExecutor::query(const std::string& sql)
    {
        if (!session_)
        {
            last_error_ = "MySqlExecutor::query: Database not connected";
            throw std::runtime_error(last_error_);
        }

        if (sql.empty())
        {
            last_error_ = "MySqlExecutor::query: SQL statement cannot be empty";
            throw std::invalid_argument(last_error_);
        }

        try
        {
            auto result = session_->sql(sql).execute();
            return processQueryResult(result);
        }
        catch (const mysqlx::Error& e)
        {
            last_error_ = fmt::format("MySqlExecutor::query: Query execution failed - {}", e.what());
            throw std::runtime_error(last_error_);
        }
    }

    std::vector<std::vector<std::string>> MySqlExecutor::queryWithParams(const std::string& sql,
                                                                         const std::vector<std::string>& params)
    {
        if (!session_)
        {
            last_error_ = "MySqlExecutor::queryWithParams: Database not connected";
            throw std::runtime_error(last_error_);
        }

        if (sql.empty())
        {
            last_error_ = "MySqlExecutor::queryWithParams: SQL statement cannot be empty";
            throw std::invalid_argument(last_error_);
        }

        try
        {
            auto stmt = session_->sql(sql);
            for (const auto& param : params)
            {
                stmt.bind(mysqlx::Value(param));
            }
            auto result = stmt.execute();
            return processQueryResult(result);
        }
        catch (const mysqlx::Error& e)
        {
            last_error_ = fmt::format("MySqlExecutor::queryWithParams: Query execution failed - {}", e.what());
            throw std::runtime_error(last_error_);
        }
    }

    QueryResult MySqlExecutor::queryStructured(const std::string& sql)
    {
        if (!session_)
        {
            last_error_ = "MySqlExecutor::queryStructured: Database not connected";
            throw std::runtime_error(last_error_);
        }

        if (sql.empty())
        {
            last_error_ = "MySqlExecutor::queryStructured: SQL statement cannot be empty";
            throw std::invalid_argument(last_error_);
        }

        try
        {
            auto result = session_->sql(sql).execute();
            return processQueryResultStructured(result);
        }
        catch (const mysqlx::Error& e)
        {
            last_error_ = fmt::format("MySqlExecutor::queryStructured: Query execution failed - {}", e.what());
            throw std::runtime_error(last_error_);
        }
    }

    QueryResult MySqlExecutor::queryWithParamsStructured(const std::string& sql,
                                                          const std::vector<std::string>& params)
    {
        if (!session_)
        {
            last_error_ = "MySqlExecutor::queryWithParamsStructured: Database not connected";
            throw std::runtime_error(last_error_);
        }

        if (sql.empty())
        {
            last_error_ = "MySqlExecutor::queryWithParamsStructured: SQL statement cannot be empty";
            throw std::invalid_argument(last_error_);
        }

        try
        {
            auto stmt = session_->sql(sql);
            for (const auto& param : params)
            {
                stmt.bind(mysqlx::Value(param));
            }
            auto result = stmt.execute();
            return processQueryResultStructured(result);
        }
        catch (const mysqlx::Error& e)
        {
            last_error_ = fmt::format("MySqlExecutor::queryWithParamsStructured: Query execution failed - {}", e.what());
            throw std::runtime_error(last_error_);
        }
    }

    bool MySqlExecutor::isConnected() const
    {
        return connected_ && (session_ != nullptr);
    }

    std::string MySqlExecutor::getLastError() const
    {
        return last_error_;
    }

    QueryValue MySqlExecutor::toQueryValue(const mysqlx::Value& value)
    {
        if (value.isNull())
        {
            return std::monostate{};
        }

        // Try integer first
        try
        {
            return value.get<int64_t>();
        }
        catch (const mysqlx::Error&)
        {
        }

        // Try double
        try
        {
            return value.get<double>();
        }
        catch (const mysqlx::Error&)
        {
        }

        // Fallback to string (will throw if incompatible, which is correct)
        try
        {
            return value.get<std::string>();
        }
        catch (const mysqlx::Error&)
        {
            return std::monostate{};
        }
    }

    std::vector<std::vector<std::string>> MySqlExecutor::processQueryResult(mysqlx::SqlResult& result)
    {
        std::vector<std::vector<std::string>> results;

        mysqlx::Row row;
        while ((row = result.fetchOne()))
        {
            std::vector<std::string> row_data;
            const auto num_columns = row.colCount();
            row_data.reserve(num_columns);

            for (size_t i = 0; i < num_columns; ++i)
            {
                const auto value = toQueryValue(row[i]);
                if (std::holds_alternative<std::monostate>(value))
                {
                    row_data.emplace_back(QUERY_RESULT_NULL);
                }
                else if (std::holds_alternative<int64_t>(value))
                {
                    row_data.emplace_back(std::to_string(std::get<int64_t>(value)));
                }
                else if (std::holds_alternative<double>(value))
                {
                    row_data.emplace_back(std::to_string(std::get<double>(value)));
                }
                else
                {
                    row_data.emplace_back(std::get<std::string>(value));
                }
            }
            results.push_back(std::move(row_data));
        }

        return results;
    }

    QueryResult MySqlExecutor::processQueryResultStructured(mysqlx::SqlResult& result)
    {
        QueryResult query_result;

        // Derive column metadata from the first row
        if (const auto first_row = result.fetchOne())
        {
            const auto num_columns = first_row.colCount();
            for (size_t i = 0; i < num_columns; ++i)
            {
                query_result.column_names.push_back("column_" + std::to_string(i));
            }

            // Process the first row
            {
                QueryRow query_row;
                for (size_t i = 0; i < num_columns; ++i)
                {
                    const auto& column_name = query_result.column_names[i];
                    try
                    {
                        query_row.columns[column_name] = toQueryValue(first_row[i]);
                    }
                    catch (const mysqlx::Error&)
                    {
                        query_row.columns[column_name] = std::monostate{};
                    }
                }
                query_result.rows.push_back(std::move(query_row));
            }

            // Process remaining rows
            mysqlx::Row row;
            while ((row = result.fetchOne()))
            {
                QueryRow query_row;
                for (size_t i = 0; i < num_columns; ++i)
                {
                    const auto& column_name = query_result.column_names[i];
                    try
                    {
                        query_row.columns[column_name] = toQueryValue(row[i]);
                    }
                    catch (const mysqlx::Error&)
                    {
                        query_row.columns[column_name] = std::monostate{};
                    }
                }
                query_result.rows.push_back(std::move(query_row));
            }
        }

        return query_result;
    }
}
