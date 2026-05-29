/**
 * @file ChaCha20Cipher.cc
 * @brief ChaCha20Cipher class implementation
 * @details This file contains the implementation of the ChaCha20Cipher class methods for Cryptographic utilities and toolkit.
 */

#include "ChaCha20Cipher.hpp"

#include <fmt/format.h>
#include <stdexcept>
#include <algorithm>
#include <openssl/err.h>
#include <glog/logging.h>

namespace common::crypto::cipher
{
    ChaCha20Cipher::ChaCha20Cipher() noexcept : ctx_(nullptr), key_{}, nonce_{}, initialized_(false)
    {
    }

    ChaCha20Cipher::~ChaCha20Cipher() noexcept
    {
        cleanup();
    }

    ChaCha20Cipher::ChaCha20Cipher(ChaCha20Cipher&& other) noexcept : ctx_(other.ctx_),
                                                                      key_(other.key_),
                                                                      nonce_(other.nonce_),
                                                                      initialized_(other.initialized_)
    {
        // Transfer ownership - other no longer owns the context
        other.ctx_ = nullptr;
        other.initialized_ = false;
    }

    ChaCha20Cipher& ChaCha20Cipher::operator=(ChaCha20Cipher&& other) noexcept
    {
        if (this != &other)
        {
            cleanup();
            ctx_ = other.ctx_;
            key_ = other.key_;
            nonce_ = other.nonce_;
            initialized_ = other.initialized_;

            // Transfer ownership
            other.ctx_ = nullptr;
            other.initialized_ = false;
        }
        return *this;
    }

    void ChaCha20Cipher::initialize(const std::vector<uint8_t>& key,
                                    const std::vector<uint8_t>& nonce)
    {
        // Validate key size
        if (key.size() != KEY_SIZE)
        {
            DLOG(WARNING) << fmt::format("ChaCha20 invalid key size: expected {} bytes, got {}", KEY_SIZE, key.size());
            throw std::invalid_argument(
                "ChaCha20 requires a 32-byte (256-bit) key. Got: " +
                std::to_string(key.size()) + " bytes."
            );
        }

        // Validate nonce size
        if (nonce.size() != NONCE_SIZE)
        {
            DLOG(WARNING) << fmt::format("ChaCha20 invalid nonce size: expected {} bytes, got {}", NONCE_SIZE, nonce.size());
            throw std::invalid_argument(
                "ChaCha20-IETF requires a 12-byte (96-bit) nonce. Got: " +
                std::to_string(nonce.size()) + " bytes."
            );
        }

        // Cleanup existing context if any
        cleanup();

        // Create new cipher context
        ctx_ = EVP_CIPHER_CTX_new();
        if (!ctx_)
        {
            DLOG(WARNING) << "Failed to create OpenSSL cipher context for ChaCha20";
            throw std::runtime_error(
                "Failed to create OpenSSL cipher context: " +
                std::string(ERR_error_string(ERR_get_error(), nullptr))
            );
        }

        // Copy key and nonce
        std::ranges::copy(key, key_.begin());
        std::ranges::copy(nonce, nonce_.begin());

        // Initialize encryption operation with ChaCha20
        // EVP_chacha20() uses 256-bit key and accepts variable IV length
        // For IETF variant (RFC 8439), we use 12-byte nonce
        if (EVP_EncryptInit_ex(ctx_, EVP_chacha20(), nullptr, key_.data(), nonce_.data()) != 1)
        {
            cleanup();
            throw std::runtime_error(
                "Failed to initialize ChaCha20 cipher with key and nonce: " +
                std::string(ERR_error_string(ERR_get_error(), nullptr))
            );
        }

        initialized_ = true;
    }

    std::vector<uint8_t> ChaCha20Cipher::encrypt(const std::vector<uint8_t>& plaintext)
    {
        if (!initialized_)
        {
            DLOG(WARNING) << "ChaCha20 encrypt called without initialization";
            throw std::runtime_error("ChaCha20 cipher not initialized. Call initialize() first.");
        }
        return process(plaintext);
    }

    std::vector<uint8_t> ChaCha20Cipher::decrypt(const std::vector<uint8_t>& ciphertext)
    {
        if (!initialized_)
        {
            DLOG(WARNING) << "ChaCha20 decrypt called without initialization";
            throw std::runtime_error("ChaCha20 cipher not initialized. Call initialize() first.");
        }
        // For stream ciphers, decryption is identical to encryption
        return process(ciphertext);
    }

    std::vector<uint8_t> ChaCha20Cipher::generateKeystream(size_t length)
    {
        if (!initialized_)
        {
            throw std::runtime_error("ChaCha20 cipher not initialized. Call initialize() first.");
        }

        // Generate keystream by encrypting zeros
        const std::vector<uint8_t> zeros(length, 0);
        return process(zeros);
    }

    void ChaCha20Cipher::reset()
    {
        if (!initialized_)
        {
            return; // Nothing to reset
        }

        // Re-initialize with the same key and nonce
        // This resets the counter to INITIAL_COUNTER
        if (EVP_EncryptInit_ex(ctx_, nullptr, nullptr, key_.data(), nonce_.data()) != 1)
        {
            throw std::runtime_error(
                "Failed to reset ChaCha20 cipher: " +
                std::string(ERR_error_string(ERR_get_error(), nullptr))
            );
        }
    }

    std::string ChaCha20Cipher::getAlgorithmName() const noexcept
    {
        return "ChaCha20";
    }

    bool ChaCha20Cipher::isInitialized() const noexcept
    {
        return initialized_;
    }

    std::vector<uint8_t> ChaCha20Cipher::process(const std::vector<uint8_t>& input) const
    {
        if (!ctx_)
        {
            throw std::runtime_error("Invalid cipher context.");
        }

        // Output buffer (same size as input for stream cipher)
        std::vector<uint8_t> output(input.size());
        int output_len = 0;
        int final_len = 0;

        // Encrypt/decrypt the data
        if (EVP_EncryptUpdate(ctx_, output.data(), &output_len,
                              input.data(), static_cast<int>(input.size())) != 1)
        {
            throw std::runtime_error(
                "ChaCha20 encryption/decryption failed: " +
                std::string(ERR_error_string(ERR_get_error(), nullptr))
            );
        }

        // Finalize (for stream ciphers, this typically doesn't add padding)
        if (EVP_EncryptFinal_ex(ctx_, output.data() + output_len, &final_len) != 1)
        {
            throw std::runtime_error(
                "ChaCha20 finalization failed: " +
                std::string(ERR_error_string(ERR_get_error(), nullptr))
            );
        }

        // Resize output to actual size
        output.resize(static_cast<size_t>(output_len + final_len));
        return output;
    }

    void ChaCha20Cipher::cleanup() noexcept
    {
        if (ctx_)
        {
            EVP_CIPHER_CTX_free(ctx_);
            ctx_ = nullptr;
        }
        initialized_ = false;
    }
}
