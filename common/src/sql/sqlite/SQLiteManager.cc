/**
 * @file SQLiteManager.cc
 * @brief SQLiteManager implementation â€?sqlite3 C API wrapper
 * @details Implements SQLite database operations: open/close via sqlite3_open,
 *          query execution via sqlite3_exec and sqlite3_prepare, and
 *          transaction control via BEGIN/COMMIT/ROLLBACK SQL commands.
 */

#include "SQLiteManager.hpp"

#include <fmt/format.h>
#include <glog/logging.h>
#include <stdexcept>

namespace cppforge::sql::sqlite
{
    SQLiteManager::SQLiteManager() : db_(nullptr, &sqlite3_close)
    {
    }

    SQLiteManager::SQLiteManager(const std::string& db_path) : db_(nullptr, &sqlite3_close)
    {
        createDatabase(db_path);
    }

    SQLiteManager::~SQLiteManager()
    {
        closeDatabase();
    }

    void SQLiteManager::createDatabase(const std::string& db_path)
    {
        if (db_path.empty())
        {
            DLOG(WARNING) << "SQLiteManager createDatabase: Database path is empty";
            throw std::invalid_argument("SQLiteManager::createDatabase: Database path cannot be empty");
        }

        if (db_)
        {
            closeDatabase();
        }

        sqlite3* raw_db;
        if (sqlite3_open(db_path.c_str(), &raw_db) != SQLITE_OK)
        {
            const std::string error_msg = "SQLiteManager::createDatabase: Database open failed for path '" + db_path + "': " + std::string(sqlite3_errmsg(raw_db));
            DLOG(WARNING) << error_msg;
            sqlite3_close(raw_db); // Clean up the failed connection
            throw std::runtime_error(error_msg);
        }

        db_.reset(raw_db);
    }

    void SQLiteManager::closeDatabase()
    {
        if (db_)
        {
            db_.reset(nullptr);
        }
    }

    int SQLiteManager::exec(const std::string& sql, const std::vector<std::string>& params) const
    {
        if (!db_)
        {
            throw std::runtime_error("SQLiteManager::exec: Database not open");
        }

        if (sql.empty())
        {
            throw std::invalid_argument("SQLiteManager::exec: SQL statement cannot be empty");
        }

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db_.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("SQLiteManager::exec: SQL prepare failed: " + std::string(sqlite3_errmsg(db_.get())));
        }

        // Bind parameters using helper function
        bindParameters(stmt, params, "exec");

        // Execute statement
        const int rc = sqlite3_step(stmt);
        const int affected = sqlite3_changes(db_.get());

        // Cleanup
        sqlite3_finalize(stmt);

        if (rc != SQLITE_DONE)
        {
            throw std::runtime_error("SQLiteManager::exec: SQL execution failed: " + std::string(sqlite3_errmsg(db_.get())));
        }

        return affected;
    }

    std::vector<std::vector<std::string>> SQLiteManager::query(const std::string& sql, const std::vector<std::string>& params) const
    {
        if (!db_)
        {
            throw std::runtime_error("SQLiteManager::query: Database not open");
        }

        if (sql.empty())
        {
            throw std::invalid_argument("SQLiteManager::query: SQL statement cannot be empty");
        }

        std::vector<std::vector<std::string>> results;
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db_.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("SQLiteManager::query: Query preparation failed: " + std::string(sqlite3_errmsg(db_.get())));
        }

        // Bind parameters using helper function
        bindParameters(stmt, params, "query");

        // Process results
        int step_rc;
        while ((step_rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            std::vector<std::string> row;
            const int cols = sqlite3_column_count(stmt);

            for (int i = 0; i < cols; ++i)
            {
                const unsigned char* col = sqlite3_column_text(stmt, i);
                row.emplace_back(col ? reinterpret_cast<const char*>(col) : "NULL");
            }
            results.push_back(row);
        }

        sqlite3_finalize(stmt);

        if (step_rc != SQLITE_DONE)
        {
            throw std::runtime_error("SQLiteManager::query: Query execution failed: " + std::string(sqlite3_errmsg(db_.get())));
        }

        return results;
    }

    bool SQLiteManager::isOpen() const
    {
        return db_ != nullptr;
    }

    void SQLiteManager::bindParameters(sqlite3_stmt* stmt, const std::vector<std::string>& params, const std::string_view method_name)
    {
        for (size_t i = 0; i < params.size(); ++i)
        {
            if (sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_STATIC) != SQLITE_OK)
            {
                sqlite3_finalize(stmt);
                throw std::runtime_error(fmt::format("SQLiteManager::{}: Parameter binding failed at index {}", method_name, i));
            }
        }
    }
} // common
