#include "PasswordPolicy.hpp"
#include <cctype>
#include <fmt/format.h>

namespace common::auth {
    PasswordPolicy::PasswordPolicy(const size_t min_length, const size_t max_length, const bool require_uppercase, const bool require_lowercase, const bool require_digits, const bool require_special, const size_t max_login_attempts) : min_length_(min_length), max_length_(max_length), require_uppercase_(require_uppercase), require_lowercase_(require_lowercase), require_digits_(require_digits), require_special_(require_special), max_login_attempts_(max_login_attempts) {
    }

    auto PasswordPolicy::validate(const std::string &password) const noexcept -> bool {
        // Check length requirements
        if (password.length() < min_length_ || password.length() > max_length_) {
            return false;
        }

        // Initialize flags based on requirements (if requirement is false, set to true by default)
        bool has_upper = !require_uppercase_;
        bool has_lower = !require_lowercase_;
        bool has_digit = !require_digits_;
        bool has_special = !require_special_;

        // Process each character in the password
        for (const char c: password) {
            if (!has_upper && is_uppercase_char(c)) {
                has_upper = true;
            } else if (!has_lower && is_lowercase_char(c)) {
                has_lower = true;
            } else if (!has_digit && is_digit_char(c)) {
                has_digit = true;
            } else if (!has_special && is_special_char(c)) {
                has_special = true;
            }

            // Early exit if all requirements are met
            if (has_upper && has_lower && has_digit && has_special) {
                break;
            }
        }

        return has_upper && has_lower && has_digit && has_special;
    }

    auto PasswordPolicy::is_uppercase_char(const char c) noexcept -> bool {
        return std::isupper(static_cast<unsigned char>(c));
    }

    auto PasswordPolicy::is_lowercase_char(const char c) noexcept -> bool {
        return std::islower(static_cast<unsigned char>(c));
    }

    auto PasswordPolicy::is_digit_char(const char c) noexcept -> bool {
        return std::isdigit(static_cast<unsigned char>(c));
    }

    auto PasswordPolicy::is_special_char(const char c) noexcept -> bool {
        return std::ispunct(static_cast<unsigned char>(c)) || std::isspace(static_cast<unsigned char>(c));
    }

    auto PasswordPolicy::set_min_length(const size_t length) noexcept -> void {
        min_length_ = length;
    }

    auto PasswordPolicy::set_max_length(const size_t length) noexcept -> void {
        max_length_ = length;
    }

    auto PasswordPolicy::set_require_uppercase(const bool require) noexcept -> void {
        require_uppercase_ = require;
    }

    auto PasswordPolicy::set_require_lowercase(const bool require) noexcept -> void {
        require_lowercase_ = require;
    }

    auto PasswordPolicy::set_require_digits(const bool require) noexcept -> void {
        require_digits_ = require;
    }

    auto PasswordPolicy::set_require_special(const bool require) noexcept -> void {
        require_special_ = require;
    }
} // common
