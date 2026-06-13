/**
 * @file OpenSSLToolkit.cc
 * @brief OpenSSLToolkit implementation — EVP digest, AES encrypt, RAND bytes
 * @details Implements the OpenSSL C++ wrapper: EVP_Digest for hash, EVP_CIPHER
 *          for AES, RAND_bytes for secure random, and PEM parsing for keys.
 */

#include "crypto/OpenSSLToolkit.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <glog/logging.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace common::crypto
{
    void OpenSSLToolkit::deriveKey(const std::string& password, std::array<unsigned char, kKeySize>& key, const std::array<unsigned char, kSaltSize>& salt)
    {
        ERR_clear_error();
        if (PKCS5_PBKDF2_HMAC(password.data(), static_cast<int32_t>(password.size()), salt.data(), kSaltSize, kDefaultIterations, EVP_sha256(), kKeySize, key.data()) != 1)
        {
            const auto err = ERR_get_error();
            char err_buf[256];
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            LOG(WARNING) << "PKCS5_PBKDF2_HMAC key derivation failed: " << err_buf;
            throw std::runtime_error("Failed to derive key using PBKDF2-HMAC-SHA256");
        }
    }

    std::vector<unsigned char> OpenSSLToolkit::encryptAES256CBC(const std::string& plaintext, const std::string& password)
    {
        ERR_clear_error();

        std::array<unsigned char, kKeySize> key{};
        std::array<unsigned char, kSaltSize> salt{};

        if (RAND_bytes(salt.data(), kSaltSize) != 1)
        {
            const auto err = ERR_get_error();
            char err_buf[256];
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            LOG(WARNING) << "Failed to generate random salt: " << err_buf;
            throw std::runtime_error("Failed to generate random salt for key derivation");
        }

        deriveKey(password, key, salt);

        std::array<unsigned char, kIvSize> iv{};
        if (RAND_bytes(iv.data(), kIvSize) != 1)
        {
            const auto err = ERR_get_error();
            char err_buf[256];
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            LOG(WARNING) << "Failed to generate random IV: " << err_buf;
            throw std::runtime_error("Failed to generate random initialization vector (IV)");
        }

        const auto ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
        {
            const auto err = ERR_get_error();
            char err_buf[256];
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            LOG(WARNING) << "Failed to create cipher context: " << err_buf;
            throw std::runtime_error("Failed to create cipher context");
        }

        std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx_guard(ctx, EVP_CIPHER_CTX_free);

        int32_t len = 0;
        int32_t ciphertext_len = 0;
        std::vector<unsigned char> ciphertext(plaintext.size() + kBlockSize);

        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1)
        {
            const auto err = ERR_get_error();
            char err_buf[256];
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            LOG(WARNING) << "Failed to initialize AES-256-CBC encryption: " << err_buf;
            throw std::runtime_error("Failed to initialize AES-256-CBC encryption");
        }

        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, reinterpret_cast<const unsigned char*>(plaintext.data()), static_cast<int32_t>(plaintext.size())) != 1)
        {
            const auto err = ERR_get_error();
            char err_buf[256];
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            LOG(WARNING) << "Failed to encrypt plaintext: " << err_buf;
            throw std::runtime_error("Failed to encrypt plaintext data with AES-256-CBC algorithm");
        }
        ciphertext_len = len;

        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1)
        {
            const auto err = ERR_get_error();
            char err_buf[256];
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            LOG(WARNING) << "Failed to finalize encryption: " << err_buf;
            throw std::runtime_error("Failed to finalize AES-256-CBC encryption");
        }
        ciphertext_len += len;

        std::vector<unsigned char> result;
        result.reserve(kSaltSize + kIvSize + ciphertext_len);
        result.insert(result.end(), salt.begin(), salt.end());
        result.insert(result.end(), iv.begin(), iv.end());
        result.insert(result.end(), ciphertext.begin(), ciphertext.begin() + ciphertext_len);
        return result;
    }

    std::string OpenSSLToolkit::decryptAES256CBC(const std::vector<unsigned char>& ciphertext, const std::string& password)
    {
        constexpr size_t metadata_size = kSaltSize + kIvSize;
        if (ciphertext.size() < metadata_size)
        {
            LOG(WARNING) << "Invalid ciphertext length: " << ciphertext.size() << " bytes, expected at least " << metadata_size;
            throw std::runtime_error("Invalid ciphertext length: too short to contain salt and IV");
        }

        std::array<unsigned char, kSaltSize> salt{};
        std::copy_n(ciphertext.begin(), kSaltSize, salt.data());

        std::array<unsigned char, kIvSize> iv{};
        std::copy_n(ciphertext.begin() + kSaltSize, kIvSize, iv.data());

        std::array<unsigned char, kKeySize> key{};
        deriveKey(password, key, salt);

        ERR_clear_error();

        const auto ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
        {
            const auto err = ERR_get_error();
            char err_buf[256];
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            LOG(WARNING) << "Failed to create cipher context: " << err_buf;
            throw std::runtime_error("Failed to create cipher context");
        }

        std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx_guard(ctx, EVP_CIPHER_CTX_free);

        int32_t len = 0;
        int32_t plaintext_len = 0;
        std::vector<unsigned char> plaintext(ciphertext.size() - metadata_size + kBlockSize);

        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1)
        {
            const auto err = ERR_get_error();
            char err_buf[256];
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            LOG(WARNING) << "Failed to initialize AES-256-CBC decryption: " << err_buf;
            throw std::runtime_error("Failed to initialize AES-256-CBC decryption");
        }

        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data() + metadata_size, static_cast<int32_t>(ciphertext.size() - metadata_size)) != 1)
        {
            const auto err = ERR_get_error();
            char err_buf[256];
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            LOG(WARNING) << "Failed to decrypt ciphertext: " << err_buf;
            throw std::runtime_error("Failed to decrypt ciphertext data with AES-256-CBC algorithm");
        }
        plaintext_len = len;

        int32_t final_len = 0;
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &final_len) != 1)
        {
            const auto err = ERR_get_error();
            char err_buf[256];
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            LOG(WARNING) << "Failed to finalize decryption: " << err_buf;
            throw std::runtime_error("Failed to finalize AES-256-CBC decryption - padding error or corrupted data likely occurred");
        }
        plaintext_len += final_len;
        return {reinterpret_cast<const char*>(plaintext.data()), static_cast<std::string::size_type>(plaintext_len)};
    }
}
