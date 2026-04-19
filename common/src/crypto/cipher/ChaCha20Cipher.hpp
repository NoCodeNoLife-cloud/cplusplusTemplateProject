#pragma once
#include "StreamCipher.hpp"

#include <array>
#include <openssl/evp.h>

namespace common::crypto::cipher {
    /**
     * @class ChaCha20Cipher
     * @brief ChaCha20 stream cipher implementation using OpenSSL EVP API.
     *
     * ChaCha20 is a high-speed stream cipher designed by Daniel J. Bernstein.
     * This implementation follows RFC 8439 (IETF variant with 96-bit nonce).
     *
     * Key features:
     * - 256-bit key length
     * - 96-bit nonce (IETF standard)
     * - 32-bit block counter
     * - No padding required (stream cipher)
     *
     * Security considerations:
     * - NEVER reuse the same key-nonce pair
     * - Use cryptographically secure random number generator for key/nonce
     * - Resistant to timing attacks (OpenSSL implementation)
     *
     * @see RFC 8439: https://datatracker.ietf.org/doc/html/rfc8439
     * @see StreamCipher for interface documentation
     */
    class ChaCha20Cipher final : public StreamCipher {
    public:
        /// @brief Key size in bytes (256 bits)
        static constexpr size_t KEY_SIZE = 32;
        
        /// @brief Nonce size in bytes for IETF variant (96 bits)
        static constexpr size_t NONCE_SIZE = 12;
        
        /// @brief Initial counter value (as per RFC 8439)
        static constexpr uint32_t INITIAL_COUNTER = 1;

        /**
         * @brief Default constructor.
         * @note Cipher must be initialized with initialize() before use.
         */
        ChaCha20Cipher() noexcept;

        /// @brief Destructor - cleans up OpenSSL resources
        ~ChaCha20Cipher() noexcept override;

        // Delete copy operations to prevent resource management issues
        ChaCha20Cipher(const ChaCha20Cipher&) = delete;
        auto operator=(const ChaCha20Cipher&) -> ChaCha20Cipher& = delete;

        // Enable move operations
        ChaCha20Cipher(ChaCha20Cipher&& other) noexcept;
        auto operator=(ChaCha20Cipher&& other) noexcept -> ChaCha20Cipher&;

        /**
         * @brief Initialize the cipher with key and nonce.
         * @param key 32-byte encryption/decryption key.
         * @param nonce 12-byte nonce value (IETF standard).
         * @throws std::invalid_argument if key size != 32 or nonce size != 12.
         * @throws std::runtime_error if OpenSSL initialization fails.
         */
        void initialize(const std::vector<uint8_t>& key, 
                       const std::vector<uint8_t>& nonce) override;

        /**
         * @brief Encrypt plaintext using ChaCha20.
         * @param plaintext Data to encrypt.
         * @return Encrypted ciphertext (same size as plaintext).
         * @throws std::runtime_error if cipher not initialized or encryption fails.
         *
         * @note After encryption, the internal counter advances. For decryption,
         *       create a NEW cipher instance with the same key/nonce instead of
         *       calling reset() on the same instance.
         */
        [[nodiscard]] auto encrypt(const std::vector<uint8_t>& plaintext) -> std::vector<uint8_t> override;

        /**
         * @brief Decrypt ciphertext using ChaCha20.
         * @param ciphertext Data to decrypt.
         * @return Decrypted plaintext (same size as ciphertext).
         * @throws std::runtime_error if cipher not initialized or decryption fails.
         *
         * @note Use a FRESH cipher instance initialized with the same key/nonce
         *       that was used for encryption. Do NOT reuse the encryption instance.
         */
        [[nodiscard]] auto decrypt(const std::vector<uint8_t>& ciphertext) -> std::vector<uint8_t> override;

        /**
         * @brief Generate ChaCha20 keystream.
         * @param length Number of keystream bytes to generate.
         * @return Generated keystream.
         * @throws std::runtime_error if cipher not initialized.
         */
        [[nodiscard]] auto generateKeystream(size_t length) -> std::vector<uint8_t> override;

        /**
         * @brief Reset cipher state to initial configuration.
         * @note Resets the internal counter to INITIAL_COUNTER, allowing the cipher
         *       to be reused with the same key/nonce. Useful for encrypting multiple
         *       independent data blocks with the same key. NOT recommended for
         *       encryption/decryption roundtrip - use separate instances instead.
         */
        void reset() override;

        /**
         * @brief Get algorithm name.
         * @return "ChaCha20"
         */
        [[nodiscard]] auto getAlgorithmName() const noexcept -> std::string override;

        /**
         * @brief Check if cipher is initialized.
         * @return true if initialize() has been called successfully.
         */
        [[nodiscard]] auto isInitialized() const noexcept -> bool override;

    private:
        /** @brief OpenSSL cipher context */
        EVP_CIPHER_CTX* ctx_;
        
        /** @brief Encryption key (32 bytes) */
        std::array<uint8_t, KEY_SIZE> key_;
        
        /** @brief Nonce value (12 bytes for IETF variant) */
        std::array<uint8_t, NONCE_SIZE> nonce_;
        
        /** @brief Initialization flag */
        bool initialized_;

        /**
         * @brief Internal helper to perform encryption/decryption.
         * @param input Input data.
         * @return Processed output data.
         * @throws std::runtime_error if operation fails.
         */
        [[nodiscard]] auto process(const std::vector<uint8_t>& input) const -> std::vector<uint8_t>;

        /**
         * @brief Cleanup OpenSSL resources.
         */
        void cleanup() noexcept;
    };
} // namespace common::crypto::cipher
