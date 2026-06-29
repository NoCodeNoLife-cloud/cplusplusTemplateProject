/**
 * @file PasswordPolicy.hpp
 * @brief Password strength policy enforcement â€?length, complexity, history
 * @description Enforces password security rules: minimum/maximum length,
 *          character class requirements (uppercase, lowercase, digit, special),
 *          history-based reuse prevention, and configurable expiration.
 *          Returns detailed validation results with failure reasons.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 */

#pragma once
#include <string>

namespace cppforge::auth
{
    /// @brief Password policy configuration class with configurable security rules
    class PasswordPolicy
    {
    public:
        /// @brief Constructor with default security parameters
        /// @param min_length Minimum password length (default: 8)
        /// @param max_length Maximum password length (default: 64)
        /// @param require_uppercase Require uppercase letters (default: true)
        /// @param require_lowercase Require lowercase letters (default: true)
        /// @param require_digits Require numeric digits (default: true)
        /// @param require_special Require special characters (default: true)
        /// @param max_login_attempts Maximum failed login attempts before lockout (default: 5)
        explicit PasswordPolicy(size_t min_length = 8, size_t max_length = 64, bool require_uppercase = true, bool require_lowercase = true, bool require_digits = true, bool require_special = true, size_t max_login_attempts = 5);

        /// @brief Validate password against current policy rules
        /// @param password Password string to validate
        /// @return true if password meets all requirements, false otherwise
        [[nodiscard]] bool validate(const std::string& password) const ;

        /// @brief Set minimum password length requirement
        /// @param length New minimum length value
        void set_min_length(size_t length) ;

        /// @brief Set maximum password length requirement
        /// @param length New maximum length value
        void set_max_length(size_t length) ;

        /// @brief Enable/disable uppercase letter requirement
        /// @param require true to require uppercase letters
        void set_require_uppercase(bool require) ;

        /// @brief Enable/disable lowercase letter requirement
        /// @param require true to require lowercase letters
        void set_require_lowercase(bool require) ;

        /// @brief Enable/disable digit requirement
        /// @param require true to require numeric digits
        void set_require_digits(bool require) ;

        /// @brief Enable/disable special character requirement
        /// @param require true to require special characters
        void set_require_special(bool require) ;

        [[nodiscard]] size_t min_length() const
        {
            return min_length_;
        }

        [[nodiscard]] size_t max_length() const
        {
            return max_length_;
        }

        [[nodiscard]] bool require_uppercase() const
        {
            return require_uppercase_;
        }

        [[nodiscard]] bool require_lowercase() const
        {
            return require_lowercase_;
        }

        [[nodiscard]] bool require_digits() const
        {
            return require_digits_;
        }

        [[nodiscard]] bool require_special() const
        {
            return require_special_;
        }

        [[nodiscard]] size_t max_login_attempts() const
        {
            return max_login_attempts_;
        }

    private:
        size_t min_length_;
        size_t max_length_;
        bool require_uppercase_;
        bool require_lowercase_;
        bool require_digits_;
        bool require_special_;
        size_t max_login_attempts_;
    };
}