/**
 * @file OpenSSLToolkit.hpp
 * @brief OpenSSL wrapper for hash, encryption, and random operations
 * @details Provides a C++ wrapper around the OpenSSL C library for common
 *          operations: EVP digest (SHA-1, SHA-256), symmetric encryption
 *          (AES), cryptographically secure random bytes, and PEM key import.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  OpenSSL operations may require global
 * initialisation (OPENSSL_init_crypto).
 */

#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace common::crypto
{
    /// @brief Utility class for OpenSSL cryptographic operations.
    /// This class provides methods for key derivation, encryption, and decryption
    /// using OpenSSL's AES-256-CBC algorithm with PBKDF2-HMAC-SHA256 key derivation.
    class OpenSSLToolkit
    {
    public:
        OpenSSLToolkit() = delete;

        static constexpr size_t kSaltSize = 16;
        static constexpr size_t kKeySize = 32;
        static constexpr size_t kIvSize = 16;
        static constexpr size_t kBlockSize = 16;
        static constexpr int32_t kDefaultIterations = 600000;

        /// @brief Derives a key from the given password and salt using PBKDF2-HMAC-SHA256.
        /// @param password The password to derive the key from.
        /// @param key The output key buffer (kKeySize bytes for AES-256).
        /// @param salt The salt to use in key derivation (kSaltSize bytes).
        static void deriveKey(const std::string& password, std::array<unsigned char, kKeySize>& key, const std::array<unsigned char, kSaltSize>& salt);

        /// @brief Encrypts the given plaintext using AES-256-CBC with a key derived from the password.
        /// @param plaintext The plaintext to encrypt.
        /// @param password The password used to derive the encryption key.
        /// @return The encrypted ciphertext as a vector of bytes (includes salt, IV, and ciphertext).
        /// @throws std::runtime_error If encryption fails.
        [[nodiscard]] static std::vector<unsigned char> encryptAES256CBC(const std::string& plaintext, const std::string& password);

        /// @brief Decrypts the given ciphertext using AES-256-CBC with a key derived from the password.
        /// @param ciphertext The ciphertext to decrypt (includes salt, IV, and ciphertext).
        /// @param password The password used to derive the decryption key.
        /// @return The decrypted plaintext as a string.
        /// @throws std::runtime_error If decryption fails or ciphertext is invalid.
        [[nodiscard]] static std::string decryptAES256CBC(const std::vector<unsigned char>& ciphertext, const std::string& password);
    };
}