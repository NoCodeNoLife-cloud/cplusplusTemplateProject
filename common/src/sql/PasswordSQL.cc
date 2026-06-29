/**
 * @file PasswordSQL.cc
 * @brief PasswordSQL implementation â€?parameterised query template building
 * @details Implements SQL query templates for password operations, using
 *          parameterised statements to prevent SQL injection.
 */

#include "PasswordSQL.hpp"

#include <stdexcept>

#include <glog/logging.h>

namespace cppforge::sql
{
    bool PasswordSQL::HasAnyEmpty(const std::initializer_list<std::string>& params)
    {
        for (const auto& p : params)
        {
            if (p.empty())
            {
                return true;
            }
        }
        return false;
    }

    PasswordSQL::PasswordSQL(const std::string& db_path): sqlite_manager_{db_path}
    {
        constexpr auto create_table_sql = R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT UNIQUE NOT NULL,
                password TEXT NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
        )";

        if (const auto result = sqlite_manager_.exec(create_table_sql); result < 0)
        {
            throw std::runtime_error("Failed to initialize users table in database: " + db_path);
        }
    }

    bool PasswordSQL::RegisterUser(const std::string& username, const std::string& password) const
    {
        if (username.empty() || password.empty())
        {
            DLOG(WARNING) << "RegisterUser failed: empty username or password";
            return false;
        }

        try
        {
            constexpr auto insert_sql = R"(
                INSERT INTO users (username, password) VALUES (?, ?);
            )";

            if (const auto result = sqlite_manager_.exec(insert_sql, {username, password}); result > 0)
            {
                DLOG(INFO) << "User registered: " << username;
                return true;
            }
            DLOG(WARNING) << "RegisterUser failed: INSERT returned 0 rows for: " << username;
            return false;
        }
        catch (const std::exception& e)
        {
            DLOG(ERROR) << "RegisterUser exception for " << username << ": " << e.what();
            return false;
        }
    }

    bool PasswordSQL::AuthenticateUser(const std::string& username, const std::string& password) const
    {
        if (username.empty() || password.empty())
        {
            return false;
        }

        try
        {
            constexpr auto select_sql = R"(
                SELECT 1 FROM users WHERE username = ? AND password = ?;
            )";

            const auto result = sqlite_manager_.query(select_sql, {username, password});
            return !result.empty();
        }
        catch (const std::exception& e)
        {
            DLOG(ERROR) << "AuthenticateUser exception for " << username << ": " << e.what();
            return false;
        }
    }

    bool PasswordSQL::ChangePassword(const std::string& username, const std::string& old_password, const std::string& new_password) const
    {
        if (HasAnyEmpty({username, old_password, new_password}))
        {
            return false;
        }

        try
        {
            constexpr auto update_sql = R"(
                UPDATE users SET password = ? WHERE username = ? AND password = ?;
            )";

            if (const auto affected_rows = sqlite_manager_.exec(update_sql, {new_password, username, old_password}); affected_rows > 0)
            {
                DLOG(INFO) << "Password changed for: " << username;
                return true;
            }
            DLOG(WARNING) << "ChangePassword failed for: " << username;
            return false;
        }
        catch (const std::exception& e)
        {
            DLOG(ERROR) << "ChangePassword exception for " << username << ": " << e.what();
            return false;
        }
    }

    bool PasswordSQL::ResetPassword(const std::string& username, const std::string& new_password) const
    {
        if (username.empty() || new_password.empty())
        {
            return false;
        }

        try
        {
            constexpr auto update_sql = R"(
                UPDATE users SET password = ? WHERE username = ?;
            )";

            if (const auto affected_rows = sqlite_manager_.exec(update_sql, {new_password, username}); affected_rows > 0)
            {
                DLOG(INFO) << "Password reset for: " << username;
                return true;
            }
            DLOG(WARNING) << "ResetPassword failed for: " << username;
            return false;
        }
        catch (const std::exception& e)
        {
            DLOG(ERROR) << "ResetPassword exception for " << username << ": " << e.what();
            return false;
        }
    }

    bool PasswordSQL::DeleteUser(const std::string& username) const
    {
        if (username.empty())
        {
            return false;
        }

        try
        {
            constexpr auto delete_sql = R"(
                DELETE FROM users WHERE username = ?;
            )";

            if (const auto affected_rows = sqlite_manager_.exec(delete_sql, {username}); affected_rows > 0)
            {
                DLOG(INFO) << "User deleted: " << username;
                return true;
            }
            DLOG(WARNING) << "DeleteUser failed for: " << username;
            return false;
        }
        catch (const std::exception& e)
        {
            DLOG(ERROR) << "DeleteUser exception for " << username << ": " << e.what();
            return false;
        }
    }

    bool PasswordSQL::UserExists(const std::string& username) const
    {
        if (username.empty())
        {
            return false;
        }

        try
        {
            constexpr auto select_sql = R"(
                SELECT 1 FROM users WHERE username = ?;
            )";

            const auto result = sqlite_manager_.query(select_sql, {username});
            return !result.empty();
        }
        catch (const std::exception& e)
        {
            DLOG(ERROR) << "UserExists exception for " << username << ": " << e.what();
            return false;
        }
    }

    std::string PasswordSQL::GetUser(const std::string& username) const
    {
        if (username.empty())
        {
            return {};
        }

        try
        {
            constexpr auto select_sql = R"(
                SELECT username FROM users WHERE username = ?;
            )";

            if (const auto result = sqlite_manager_.query(select_sql, {username}); !result.empty() && !result[0].empty())
            {
                return result[0][0];
            }

            return {};
        }
        catch (const std::exception& e)
        {
            DLOG(ERROR) << "GetUser exception for " << username << ": " << e.what();
            return {};
        }
    }

    std::vector<std::string> PasswordSQL::GetAllUsers() const
    {
        try
        {
            constexpr auto select_sql = R"(
                SELECT username FROM users ORDER BY username;
            )";

            const auto result = sqlite_manager_.query(select_sql);
            std::vector<std::string> users;
            users.reserve(result.size());

            for (const auto& row : result)
            {
                if (!row.empty())
                {
                    users.push_back(row[0]);
                }
            }

            return users;
        }
        catch (const std::exception& e)
        {
            DLOG(ERROR) << "GetAllUsers exception: " << e.what();
            return {};
        }
    }
}
