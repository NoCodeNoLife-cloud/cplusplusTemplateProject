/**
* @file CryptoToolKit.hpp
 * @brief CryptoToolKit class declaration
 * @details This header defines the CryptoToolKit class that provides functionality for Cryptographic utilities and toolkit.
 */

#pragma once
#include <string>

#include "auth/AuthenticationException.hpp"

namespace common::crypto
{
    /// @brief Cryptographic utilities for password hashing and secure comparisons
    class CryptoToolKit
    {
    public:
        static constexpr size_t SALT_SIZE = 16; /// @brief Size of cryptographic salt in bytes
        static constexpr size_t HASH_SIZE = 32; /// @brief Size of SHA256 hash output in bytes

        /// @brief Generate cryptographically secure random salt
        /// @return Random salt string of SALT_SIZE bytes
        /// @throws AuthenticationException if secure random generation fails
        [[nodiscard]] static std::string generate_salt();

        /// @brief Hash password using PBKDF2-HMAC-SHA256 with configurable iterations
        /// @param password Plaintext password to hash
        /// @param salt Salt value for hashing
        /// @param iterations Number of PBKDF2 iterations (default: 600000, NIST recommended)
        /// @return Hashed password string
        /// @throws AuthenticationException if hashing operation fails
        [[nodiscard]] static std::string hash_password(const std::string& password, const std::string& salt, size_t iterations = 600000);

        /// @brief Constant-time string comparison to prevent timing attacks
        /// @param a First string to compare
        /// @param b Second string to compare
        /// @return true if strings are equal, false otherwise
        [[nodiscard]] static bool secure_compare(const std::string& a, const std::string& b) ;
    };
}
