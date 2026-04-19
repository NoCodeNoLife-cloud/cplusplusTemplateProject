#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <memory>

namespace common::crypto::cipher {
    /**
     * @class StreamCipher
     * @brief Abstract base class for stream cipher implementations.
     *
     * This class defines the interface for all stream cipher algorithms.
     * Stream ciphers encrypt data by combining plaintext with a pseudorandom
     * keystream, typically using XOR operations.
     *
     * @note All derived classes must implement thread-safe operations or
     *       clearly document their thread safety guarantees.
     *
     * @warning IMPORTANT: For encryption/decryption roundtrip, use SEPARATE cipher instances.
     *          Stream ciphers maintain internal state (counter/position). After encryption,
     *          the state advances. Calling reset() and then decrypt() on the SAME instance
     *          may produce incorrect results. Instead:
     *          @code
     *          // Correct approach: separate instances
     *          auto encrypt_cipher = factory.create(algo);
     *          encrypt_cipher->initialize(key, nonce);
     *          auto ciphertext = encrypt_cipher->encrypt(plaintext);
     *          
     *          auto decrypt_cipher = factory.create(algo);  // New instance
     *          decrypt_cipher->initialize(key, nonce);      // Same key/nonce
     *          auto decrypted = decrypt_cipher->decrypt(ciphertext);
     *          @endcode
     *
     * @example
     * @code
     * auto cipher = StreamCipherFactory::create(StreamCipherFactory::Algorithm::CHACHA20);
     * std::vector<uint8_t> key(32);   // 256-bit key
     * std::vector<uint8_t> nonce(12); // 96-bit nonce
     * 
     * // Initialize with secure random values (use RandomGenerator in production)
     * cipher->initialize(key, nonce);
     * auto ciphertext = cipher->encrypt(plaintext);
     * // For decryption, create a NEW cipher instance with same key/nonce
     * @endcode
     */
    class StreamCipher {
    public:
        /// @brief Virtual destructor for proper cleanup in inheritance hierarchy
        virtual ~StreamCipher() noexcept = default;

        /**
         * @brief Initialize the cipher with key and nonce.
         * @param key The encryption/decryption key (algorithm-specific length).
         * @param nonce The nonce/IV value (algorithm-specific length).
         * @throws std::invalid_argument if key or nonce length is invalid.
         * @throws std::runtime_error if initialization fails.
         *
         * @note This method must be called before encrypt/decrypt operations.
         *       Calling initialize() again will reset the internal state.
         */
        virtual void initialize(const std::vector<uint8_t>& key, 
                               const std::vector<uint8_t>& nonce) = 0;

        /**
         * @brief Encrypt plaintext data.
         * @param plaintext The data to encrypt.
         * @return Encrypted ciphertext.
         * @throws std::runtime_error if cipher is not initialized or encryption fails.
         *
         * @note For stream ciphers, encryption and decryption are symmetric operations.
         */
        [[nodiscard]] virtual auto encrypt(const std::vector<uint8_t>& plaintext) -> std::vector<uint8_t> = 0;

        /**
         * @brief Decrypt ciphertext data.
         * @param ciphertext The data to decrypt.
         * @return Decrypted plaintext.
         * @throws std::runtime_error if cipher is not initialized or decryption fails.
         *
         * @note For stream ciphers, this is identical to encrypt() operation.
         */
        [[nodiscard]] virtual auto decrypt(const std::vector<uint8_t>& ciphertext) -> std::vector<uint8_t> = 0;

        /**
         * @brief Generate keystream of specified length.
         * @param length Number of bytes to generate.
         * @return Generated keystream bytes.
         * @throws std::runtime_error if cipher is not initialized.
         *
         * @note Useful for custom encryption schemes or testing.
         */
        [[nodiscard]] virtual auto generateKeystream(size_t length) -> std::vector<uint8_t> = 0;

        /**
         * @brief Reset the cipher state to initial configuration.
         * @throws std::runtime_error if reset operation fails.
         *
         * @note After reset, the cipher can be reused with the same key/nonce.
         */
        virtual void reset() = 0;

        /**
         * @brief Get the algorithm name.
         * @return Human-readable algorithm name (e.g., "ChaCha20").
         */
        [[nodiscard]] virtual auto getAlgorithmName() const noexcept -> std::string = 0;

        /**
         * @brief Check if the cipher has been initialized.
         * @return true if initialized, false otherwise.
         */
        [[nodiscard]] virtual auto isInitialized() const noexcept -> bool = 0;
    };
} // namespace common::crypto::cipher
