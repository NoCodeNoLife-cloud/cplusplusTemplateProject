#include "src/crypto/OpenSSLToolkit.hpp"
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <fmt/format.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <array>

namespace common::crypto {
    auto OpenSSLToolkit::deriveKey(const std::string &password, std::array<unsigned char, 32> &key, const std::array<unsigned char, 16> &salt) noexcept -> void {
EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), salt.data(), reinterpret_cast<const unsigned char *>(password.c_str()), static_cast<int32_t>(password.size()), 1, key.data(), nullptr);
    }

    auto OpenSSLToolkit::encryptAES256CBC(const std::string &plaintext, const std::string &password) -> std::vector<unsigned char> {
std::array<unsigned char, 32> key{};
        std::array<unsigned char, 16> salt{};

        // Generate random salt
        if (RAND_bytes(salt.data(), 16) != 1) {
throw std::runtime_error("Failed to generate random salt for key derivation");
        }

        deriveKey(password, key, salt);

        std::array<unsigned char, AES_BLOCK_SIZE> iv{};
        if (RAND_bytes(iv.data(), AES_BLOCK_SIZE) != 1) {
throw std::runtime_error("Failed to generate random initialization vector (IV)");
        }

        const auto ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
throw std::runtime_error("Failed to create cipher context");
        }

        // Ensure proper cleanup of context
        std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx_guard(ctx, EVP_CIPHER_CTX_free);

        int32_t len = 0;
        int32_t ciphertext_len = 0;
        std::vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_SIZE);

        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
throw std::runtime_error("Failed to initialize AES-256-CBC encryption with provided key and IV");
        }

        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, reinterpret_cast<const unsigned char *>(plaintext.data()), static_cast<int32_t>(plaintext.size())) != 1) {
throw std::runtime_error("Failed to encrypt plaintext data with AES-256-CBC algorithm");
        }
        ciphertext_len = len;

        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
throw std::runtime_error("Failed to finalize AES-256-CBC encryption - padding error likely occurred");
        }
        ciphertext_len += len;

        // Create result that includes salt, IV, and ciphertext
        std::vector<unsigned char> result;
        result.reserve(salt.size() + iv.size() + ciphertext.size());
        result.insert(result.end(), salt.begin(), salt.end());
        result.insert(result.end(), iv.begin(), iv.end());
        result.insert(result.end(), ciphertext.begin(), ciphertext.end());
return result;
    }

    auto OpenSSLToolkit::decryptAES256CBC(const std::vector<unsigned char> &ciphertext, const std::string &password) -> std::string {
        constexpr size_t metadata_size = 16 + AES_BLOCK_SIZE; // salt + iv
        if (ciphertext.size() < metadata_size) {
throw std::runtime_error("Invalid ciphertext length: too short to contain salt and IV");
        }
// Extract salt and IV from the beginning of the ciphertext
        std::array<unsigned char, 16> salt{};
        std::copy_n(ciphertext.begin(), 16, salt.data());

        std::array<unsigned char, AES_BLOCK_SIZE> iv{};
        std::copy_n(ciphertext.begin() + 16, AES_BLOCK_SIZE, iv.data());

        std::array<unsigned char, 32> key{};
        deriveKey(password, key, salt);

        const auto ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
throw std::runtime_error("Failed to create cipher context");
        }

        // Ensure proper cleanup of context
        std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx_guard(ctx, EVP_CIPHER_CTX_free);

        int32_t len = 0;
        int32_t plaintext_len = 0;
        std::vector<unsigned char> plaintext(ciphertext.size());

        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
throw std::runtime_error("Failed to initialize AES-256-CBC decryption with provided key and IV");
        }

        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data() + metadata_size, static_cast<int32_t>(ciphertext.size() - metadata_size)) != 1) {
throw std::runtime_error("Failed to decrypt ciphertext data with AES-256-CBC algorithm");
        }
        plaintext_len = len;

        int32_t final_len = 0;
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &final_len) != 1) {
throw std::runtime_error("Failed to finalize AES-256-CBC decryption - padding error or corrupted data likely occurred");
        }
        plaintext_len += final_len;
return {reinterpret_cast<const char *>(plaintext.data()), static_cast<std::string::size_type>(plaintext_len)};
    }
}
