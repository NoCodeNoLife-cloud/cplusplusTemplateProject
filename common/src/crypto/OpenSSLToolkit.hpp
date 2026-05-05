#pragma once

#include <array>
#include <string>
#include <vector>

namespace common::crypto {
/// @brief Utility class for OpenSSL cryptographic operations.
/// This class provides methods for key derivation, encryption, and decryption
/// using OpenSSL's AES-256-CBC algorithm with PBKDF2 key derivation.
class OpenSSLToolkit {
public:
    OpenSSLToolkit() = delete;

    /// @brief Derives a key from the given password and salt using PBKDF2.
    /// @param password The password to derive the key from.
    /// @param key The output key buffer (32 bytes for AES-256).
    /// @param salt The salt to use in key derivation (16 bytes).
    static auto deriveKey(const std::string& password, std::array<unsigned char, 32>& key, const std::array<unsigned char, 16>& salt) noexcept -> void;

    /// @brief Encrypts the given plaintext using AES-256-CBC with a key derived from the password.
    /// @param plaintext The plaintext to encrypt.
    /// @param password The password used to derive the encryption key.
    /// @return The encrypted ciphertext as a vector of bytes (includes IV and salt).
    /// @throws std::runtime_error If encryption fails.
    [[nodiscard]] static auto encryptAES256CBC(const std::string& plaintext, const std::string& password) -> std::vector<unsigned char>;

    /// @brief Decrypts the given ciphertext using AES-256-CBC with a key derived from the password.
    /// @param ciphertext The ciphertext to decrypt (includes IV and salt).
    /// @param password The password used to derive the decryption key.
    /// @return The decrypted plaintext as a string.
    /// @throws std::runtime_error If decryption fails or ciphertext is invalid.
    [[nodiscard]] static auto decryptAES256CBC(const std::vector<unsigned char>& ciphertext, const std::string& password) -> std::string;
};
}