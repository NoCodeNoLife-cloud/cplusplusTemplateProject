#pragma once
#include "StreamCipher.hpp"

#include <vector>
#include <cstdint>
#include <algorithm>
#include <memory>

namespace common::crypto::cipher {
    /**
     * @class XorBitCipher
     * @brief A stream cipher implementation using XOR operations on bit sequences.
     *
     * This class provides cryptographic operations for bit sequences using the XOR algorithm.
     * Encryption and decryption are symmetric operations (XOR with the same key stream).
     * Supports both byte-aligned and arbitrary bit-length operations.
     *
     * @note This is a basic stream cipher. For production use, ensure key stream is generated
     *       by a cryptographically secure pseudo-random number generator (CSPRNG).
     * @implements StreamCipher
     */
    class XorBitCipher final : public StreamCipher {
    public:
        /**
         * @brief Default constructor. Initializes cipher with empty key.
         * @note Must call initialize() before encrypt/decrypt operations.
         */
        XorBitCipher() noexcept = default;

        /**
         * @brief Constructor with key initialization.
         * @param key The initial key stream as byte vector.
         */
        explicit XorBitCipher(std::vector<uint8_t> key) noexcept 
            : key_stream_(std::move(key)), key_pos_(0), bit_pos_(0) {
        }

        /// @brief Destructor
        ~XorBitCipher() noexcept override = default;

        /**
         * @brief Initialize the cipher with key.
         * @param key The encryption/decryption key as byte vector.
         * @param nonce Unused parameter (kept for interface compatibility).
         * @throws std::invalid_argument if key is empty.
         * @note For XorBitCipher, nonce is ignored. Only key is used.
         */
        void initialize(const std::vector<uint8_t>& key, 
                       const std::vector<uint8_t>& nonce) override;

        /**
         * @brief Encrypt data using XOR with key stream.
         * @param plaintext Data to encrypt.
         * @return Encrypted ciphertext.
         * @throws std::runtime_error if cipher not initialized.
         *
         * @note After encryption, the key stream position advances. For decryption,
         *       create a NEW cipher instance with the same key instead of
         *       calling reset() on the same instance.
         */
        [[nodiscard]] auto encrypt(const std::vector<uint8_t>& plaintext) -> std::vector<uint8_t> override;

        /**
         * @brief Decrypt data using XOR with key stream.
         * @param ciphertext Data to decrypt.
         * @return Decrypted plaintext.
         * @throws std::runtime_error if cipher not initialized.
         *
         * @note Use a FRESH cipher instance initialized with the same key
         *       that was used for encryption. Do NOT reuse the encryption instance.
         */
        [[nodiscard]] auto decrypt(const std::vector<uint8_t>& ciphertext) -> std::vector<uint8_t> override;

        /**
         * @brief Generate keystream of specified length.
         * @param length Number of bytes to generate.
         * @return Generated keystream.
         * @throws std::runtime_error if cipher not initialized.
         */
        [[nodiscard]] auto generateKeystream(size_t length) -> std::vector<uint8_t> override;

        /**
         * @brief Reset the key stream position to the beginning.
         * @note Resets key_pos_ and bit_pos_ to 0, allowing the cipher to restart
         *       from the beginning of the key stream. Useful for re-encrypting data
         *       with the same key. NOT recommended for encryption/decryption
         *       roundtrip - use separate instances instead.
         */
        void reset() override;

        /**
         * @brief Get algorithm name.
         * @return "XorBitCipher"
         */
        [[nodiscard]] auto getAlgorithmName() const noexcept -> std::string override;

        /**
         * @brief Check if cipher is initialized (has key).
         * @return true if key is set, false otherwise.
         */
        [[nodiscard]] auto isInitialized() const noexcept -> bool override;

        /**
         * @brief Process (encrypt/decrypt) data using XOR with key stream.
         * @param data Input data to be processed.
         * @return Processed data after XOR operation.
         * @throw std::invalid_argument if key is empty.
         *
         * @note This method processes full bytes. Use processBits() for arbitrary bit lengths.
         */
        [[nodiscard]] auto process(const std::vector<uint8_t> &data) const -> std::vector<uint8_t>;

        /**
         * @brief Process data in-place for memory efficiency.
         * @param data Reference to data buffer to be processed.
         * @throw std::invalid_argument if key is empty.
         */
        auto processInPlace(std::vector<uint8_t> &data) const -> void;

        /**
         * @brief Process arbitrary bit sequence.
         * @param bits Input bit sequence as vector of bool.
         * @return Processed bit sequence.
         * @throw std::invalid_argument if key is empty.
         *
         * @note Bit-level processing is slower than byte-level but supports arbitrary lengths.
         */
        [[nodiscard]] auto processBits(const std::vector<bool> &bits) const -> std::vector<bool>;

        /**
         * @brief Generate keystream segment using internal state.
         * @param length Number of bytes to generate.
         * @return Generated keystream segment.
         * @throw std::invalid_argument if key is empty.
         */
        [[nodiscard]] auto generateKeyStream(size_t length) const -> std::vector<uint8_t>;

        /**
         * @brief Get current key stream position.
         * @return Current byte position in key stream.
         */
        [[nodiscard]] auto getCurrentPosition() const noexcept -> size_t;

        /**
         * @brief Check if key is set.
         * @return True if key stream is not empty, false otherwise.
         */
        [[nodiscard]] auto hasKey() const noexcept -> bool;

        /**
         * @brief Create a new cipher instance with auto-generated random key.
         * @param key_length Length of the key in bytes.
         * @return New XorBitCipher instance with generated key.
         * @note This is a factory method for convenience testing only.
         *       Not cryptographically secure without proper entropy source.
         */
        [[nodiscard]] static auto createWithRandomKey(size_t key_length) -> XorBitCipher;

    private:
        /** @brief The key stream (keystream) for XOR operations */
        std::vector<uint8_t> key_stream_;

        /** @brief Current position in key stream (byte index) */
        mutable size_t key_pos_{0};

        /** @brief Current bit position within current byte (0-7) */
        mutable uint8_t bit_pos_{0};

        /**
         * @brief Validate that cipher is initialized.
         * @throws std::runtime_error if not initialized.
         */
        void validateInitialized() const;

        /**
         * @brief Get next key byte and advance position.
         * @return Next key byte for XOR operation.
         * @throw std::invalid_argument if key is empty.
         */
        [[nodiscard]] auto nextKeyByte() const -> uint8_t;

        /**
         * @brief Get next key bit and advance position.
         * @return Next key bit (0 or 1) for XOR operation.
         * @throw std::invalid_argument if key is empty.
         */
        [[nodiscard]] auto nextKeyBit() const -> bool;
    };
} // namespace common::crypto::cipher
