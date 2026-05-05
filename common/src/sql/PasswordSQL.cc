#include "PasswordSQL.hpp"
#include <stdexcept>
#include <string_view>

namespace common::sql {
PasswordSQL::PasswordSQL(const std::string& db_path) noexcept(false) : sqlite_manager_{db_path} {
    /// @brief Create users table if not exists during initialization
    constexpr std::string_view create_table_sql = R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT UNIQUE NOT NULL,
                password TEXT NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
        )";

    if (const auto result = sqlite_manager_.exec(create_table_sql.data()); result < 0) {
        throw std::runtime_error("Failed to initialize users table in database: " + db_path);
    }
}

auto PasswordSQL::RegisterUser(const std::string& username, const std::string& password) const noexcept -> bool {
    /// @brief Validate input parameters
    if (username.empty() || password.empty()) {
        return false;
    }

    try {
        constexpr std::string_view insert_sql = R"(
                INSERT INTO users (username, password) VALUES (?, ?);
            )";

        if (const auto result = sqlite_manager_.exec(insert_sql.data(), {username, password}); result > 0) {
            return true;
        } else {
            return false;
        }
    } catch (...) {
        return false;
    }
}

auto PasswordSQL::AuthenticateUser(const std::string& username, const std::string& password) const noexcept -> bool {
    /// @brief Validate input parameters
    if (username.empty() || password.empty()) {
        return false;
    }

    try {
        constexpr std::string_view select_sql = R"(
                SELECT 1 FROM users WHERE username = ? AND password = ?;
            )";

        const auto result = sqlite_manager_.query(select_sql.data(), {username, password});
        return !result.empty();
    } catch (...) {
        return false;
    }
}

auto PasswordSQL::ChangePassword(const std::string& username, const std::string& old_password, const std::string& new_password) const noexcept -> bool {
    /// @brief Validate input parameters
    if (username.empty() || old_password.empty() || new_password.empty()) {
        return false;
    }

    try {
        /// @brief First check if the old credentials are correct
        if (!AuthenticateUser(username, old_password)) {
            return false;
        }

        constexpr std::string_view update_sql = R"(
                UPDATE users SET password = ? WHERE username = ?;
            )";

        if (const auto affected_rows = sqlite_manager_.exec(update_sql.data(), {new_password, username}); affected_rows > 0) {
            return true;
        }

        return false;
    } catch (...) {
        return false;
    }
}

auto PasswordSQL::ResetPassword(const std::string& username, const std::string& new_password) const noexcept -> bool {
    /// @brief Validate input parameters
    if (username.empty() || new_password.empty()) {
        return false;
    }

    try {
        constexpr std::string_view update_sql = R"(
                UPDATE users SET password = ? WHERE username = ?;
            )";

        if (const auto affected_rows = sqlite_manager_.exec(update_sql.data(), {new_password, username}); affected_rows > 0) {
            return true;
        }

        return false;
    } catch (...) {
        return false;
    }
}

auto PasswordSQL::DeleteUser(const std::string& username) const noexcept -> bool {
    /// @brief Validate input parameters
    if (username.empty()) {
        return false;
    }

    try {
        constexpr std::string_view delete_sql = R"(
                DELETE FROM users WHERE username = ?;
            )";

        if (const auto affected_rows = sqlite_manager_.exec(delete_sql.data(), {username}); affected_rows > 0) {
            return true;
        }

        return false;
    } catch (...) {
        return false;
    }
}

auto PasswordSQL::UserExists(const std::string& username) const noexcept -> bool {
    /// @brief Validate input parameters
    if (username.empty()) {
        return false;
    }

    try {
        constexpr std::string_view select_sql = R"(
                SELECT 1 FROM users WHERE username = ?;
            )";

        const auto result = sqlite_manager_.query(select_sql.data(), {username});
        return !result.empty();
    } catch (...) {
        return false;
    }
}

auto PasswordSQL::GetUser(const std::string& username) const noexcept -> std::string {
    /// @brief Validate input parameters
    if (username.empty()) {
        return {};
    }

    try {
        constexpr std::string_view select_sql = R"(
                SELECT username FROM users WHERE username = ?;
            )";

        if (const auto result = sqlite_manager_.query(select_sql.data(), {username}); !result.empty() && !result[0].empty()) {
            return result[0][0];
        }

        return {};
    } catch (...) {
        return {};
    }
}

auto PasswordSQL::GetAllUsers() const noexcept -> std::vector<std::string> {
    try {
        constexpr std::string_view select_sql = R"(
                SELECT username FROM users ORDER BY username;
            )";

        const auto result = sqlite_manager_.query(select_sql.data());
        std::vector<std::string> users;
        users.reserve(result.size()); // Reserve space for efficiency

        for (const auto& row : result) {
            if (!row.empty()) {
                users.push_back(row[0]);
            }
        }

        return users;
    } catch (...) {
        return {};
    }
}
}
