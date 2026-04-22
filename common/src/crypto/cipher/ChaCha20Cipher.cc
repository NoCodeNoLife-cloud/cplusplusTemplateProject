#include "ChaCha20Cipher.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <stdexcept>
#include <algorithm>
#include <openssl/err.h>

namespace common::crypto::cipher {
    ChaCha20Cipher::ChaCha20Cipher() noexcept : ctx_(nullptr), key_{}, nonce_{}, initialized_(false) {
    }

    ChaCha20Cipher::~ChaCha20Cipher() noexcept {
        cleanup();
    }

    ChaCha20Cipher::ChaCha20Cipher(ChaCha20Cipher &&other) noexcept : ctx_(other.ctx_),
                                                                      key_(other.key_),
                                                                      nonce_(other.nonce_),
                                                                      initialized_(other.initialized_) {
        // Transfer ownership - other no longer owns the context
        other.ctx_ = nullptr;
        other.initialized_ = false;
    }

    auto ChaCha20Cipher::operator=(ChaCha20Cipher &&other) noexcept -> ChaCha20Cipher & {
        if (this != &other) {
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

    void ChaCha20Cipher::initialize(const std::vector<uint8_t> &key,
                                    const std::vector<uint8_t> &nonce) {
        // Validate key size
        if (key.size() != KEY_SIZE) {
            DLOG(ERROR) << fmt::format("ChaCha20 initialization failed - invalid key size: {} bytes (expected {})", key.size(), KEY_SIZE);
            throw std::invalid_argument(
                "ChaCha20 requires a 32-byte (256-bit) key. Got: " +
                std::to_string(key.size()) + " bytes."
            );
        }

        // Validate nonce size
        if (nonce.size() != NONCE_SIZE) {
            DLOG(ERROR) << fmt::format("ChaCha20 initialization failed - invalid nonce size: {} bytes (expected {})", nonce.size(), NONCE_SIZE);
            throw std::invalid_argument(
                "ChaCha20-IETF requires a 12-byte (96-bit) nonce. Got: " +
                std::to_string(nonce.size()) + " bytes."
            );
        }

        // Cleanup existing context if any
        cleanup();

        // Create new cipher context
        ctx_ = EVP_CIPHER_CTX_new();
        if (!ctx_) {
            DLOG(ERROR) << "ChaCha20 initialization failed - cannot create OpenSSL context";
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
        if (EVP_EncryptInit_ex(ctx_, EVP_chacha20(), nullptr, key_.data(), nonce_.data()) != 1) {
            cleanup();
            DLOG(ERROR) << "ChaCha20 initialization failed - OpenSSL EncryptInit error";
            throw std::runtime_error(
                "Failed to initialize ChaCha20 cipher with key and nonce: " +
                std::string(ERR_error_string(ERR_get_error(), nullptr))
            );
        }

        initialized_ = true;
        DLOG(INFO) << "ChaCha20 cipher initialized successfully with 256-bit key and 96-bit nonce";
    }

    auto ChaCha20Cipher::encrypt(const std::vector<uint8_t> &plaintext) -> std::vector<uint8_t> {
        if (!initialized_) {
            DLOG(ERROR) << "ChaCha20 encrypt failed - cipher not initialized";
            throw std::runtime_error("ChaCha20 cipher not initialized. Call initialize() first.");
        }
        DLOG(INFO) << fmt::format("ChaCha20 encrypt - input size: {} bytes", plaintext.size());
        return process(plaintext);
    }

    auto ChaCha20Cipher::decrypt(const std::vector<uint8_t> &ciphertext) -> std::vector<uint8_t> {
        if (!initialized_) {
            DLOG(ERROR) << "ChaCha20 decrypt failed - cipher not initialized";
            throw std::runtime_error("ChaCha20 cipher not initialized. Call initialize() first.");
        }
        DLOG(INFO) << fmt::format("ChaCha20 decrypt - input size: {} bytes", ciphertext.size());
        // For stream ciphers, decryption is identical to encryption
        return process(ciphertext);
    }

    auto ChaCha20Cipher::generateKeystream(size_t length) -> std::vector<uint8_t> {
        if (!initialized_) {
            throw std::runtime_error("ChaCha20 cipher not initialized. Call initialize() first.");
        }

        // Generate keystream by encrypting zeros
        std::vector<uint8_t> zeros(length, 0);
        return process(zeros);
    }

    void ChaCha20Cipher::reset() {
        if (!initialized_) {
            DLOG(WARNING) << "ChaCha20 reset called on uninitialized cipher";
            return; // Nothing to reset
        }

        // Re-initialize with the same key and nonce
        // This resets the counter to INITIAL_COUNTER
        if (EVP_EncryptInit_ex(ctx_, nullptr, nullptr, key_.data(), nonce_.data()) != 1) {
            DLOG(ERROR) << "ChaCha20 reset failed - OpenSSL EncryptInit error";
            throw std::runtime_error(
                "Failed to reset ChaCha20 cipher: " +
                std::string(ERR_error_string(ERR_get_error(), nullptr))
            );
        }
        DLOG(INFO) << "ChaCha20 cipher reset successfully - counter reset to initial value";
    }

    auto ChaCha20Cipher::getAlgorithmName() const noexcept -> std::string {
        return "ChaCha20";
    }

    auto ChaCha20Cipher::isInitialized() const noexcept -> bool {
        return initialized_;
    }

    auto ChaCha20Cipher::process(const std::vector<uint8_t> &input) const -> std::vector<uint8_t> {
        if (!ctx_) {
            throw std::runtime_error("Invalid cipher context.");
        }

        // Output buffer (same size as input for stream cipher)
        std::vector<uint8_t> output(input.size());
        int output_len = 0;
        int final_len = 0;

        // Encrypt/decrypt the data
        if (EVP_EncryptUpdate(ctx_, output.data(), &output_len,
                              input.data(), static_cast<int>(input.size())) != 1) {
            throw std::runtime_error(
                "ChaCha20 encryption/decryption failed: " +
                std::string(ERR_error_string(ERR_get_error(), nullptr))
            );
        }

        // Finalize (for stream ciphers, this typically doesn't add padding)
        if (EVP_EncryptFinal_ex(ctx_, output.data() + output_len, &final_len) != 1) {
            throw std::runtime_error(
                "ChaCha20 finalization failed: " +
                std::string(ERR_error_string(ERR_get_error(), nullptr))
            );
        }

        // Resize output to actual size
        output.resize(static_cast<size_t>(output_len + final_len));
        return output;
    }

    void ChaCha20Cipher::cleanup() noexcept {
        if (ctx_) {
            EVP_CIPHER_CTX_free(ctx_);
            ctx_ = nullptr;
        }
        initialized_ = false;
    }
} // namespace common::crypto::cipher
