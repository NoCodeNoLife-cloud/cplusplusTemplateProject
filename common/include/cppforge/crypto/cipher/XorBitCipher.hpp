/**
 * @file XorBitCipher.hpp
 * @brief XOR-based stream cipher with repeating key
 * @details Implements a simple XOR cipher where plaintext is XORed with a
 *          repeating key stream.  Both encryption and decryption use the same
 *          XOR operation (symmetric).  NOT cryptographically secure for
 *          production use â€?suitable for obfuscation and educational purposes.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 */

#pragma once
#include <cstdint>
#include <vector>

#include <cppforge/crypto/cipher/StreamCipher.hpp>

namespace cppforge::crypto::cipher
{
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
    class XorBitCipher final : public StreamCipher
    {
    public:
        /**
         * @brief Default constructor. Initializes cipher with empty key.
         * @note Must call initialize() before encrypt/decrypt operations.
         */
        XorBitCipher()  = default;

        /**
         * @brief Constructor with key initialization.
         * @param key The initial key stream as byte vector.
         */
        explicit XorBitCipher(std::vector<uint8_t> key)  : key_stream_(std::move(key))
        {
        }

        /// @brief Destructor
        ~XorBitCipher()  override = default;

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
        [[nodiscard]] std::vector<uint8_t> encrypt(const std::vector<uint8_t>& plaintext) override;

        /**
         * @brief Decrypt data using XOR with key stream.
         * @param ciphertext Data to decrypt.
         * @return Decrypted plaintext.
         * @throws std::runtime_error if cipher not initialized.
         *
         * @note Use a FRESH cipher instance initialized with the same key
         *       that was used for encryption. Do NOT reuse the encryption instance.
         */
        [[nodiscard]] std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ciphertext) override;

        /**
         * @brief Generate keystream of specified length.
         * @param length Number of bytes to generate.
         * @return Generated keystream.
         * @throws std::runtime_error if cipher not initialized.
         */
        [[nodiscard]] std::vector<uint8_t> generateKeystream(size_t length) override;

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
        [[nodiscard]] std::string getAlgorithmName() const  override;

        /**
         * @brief Check if cipher is initialized (has key).
         * @return true if key is set, false otherwise.
         */
        [[nodiscard]] bool isInitialized() const  override;

        /**
         * @brief Process (encrypt/decrypt) data using XOR with key stream.
         * @param data Input data to be processed.
         * @return Processed data after XOR operation.
         * @throw std::invalid_argument if key is empty.
         *
         * @note This method processes full bytes. Use processBits() for arbitrary bit lengths.
         */
        [[nodiscard]] std::vector<uint8_t> process(const std::vector<uint8_t>& data);

        /**
         * @brief Process data in-place for memory efficiency.
         * @param data Reference to data buffer to be processed.
         * @throw std::invalid_argument if key is empty.
         */
        void processInPlace(std::vector<uint8_t>& data);

        /**
         * @brief Process arbitrary bit sequence.
         * @param bits Input bit sequence as vector of bool.
         * @return Processed bit sequence.
         * @throw std::invalid_argument if key is empty.
         *
         * @note Bit-level processing is slower than byte-level but supports arbitrary lengths.
         */
        [[nodiscard]] std::vector<bool> processBits(const std::vector<bool>& bits);

        /**
         * @brief Get current key stream position.
         * @return Current byte position in key stream.
         */
        [[nodiscard]] size_t getCurrentPosition() const;

        /**
         * @brief Get current bit position within current byte.
         * @return Current bit position (0-7).
         */
        [[nodiscard]] uint8_t getCurrentBitPosition() const;

        /**
         * @brief Check if key is set.
         * @return True if key stream is not empty, false otherwise.
         */
        [[nodiscard]] bool hasKey() const ;

        /**
         * @brief Create a new cipher instance with auto-generated pseudo-random key.
         * @param key_length Length of the key in bytes.
         * @return New XorBitCipher instance with generated key.
         * @note This is a factory method for convenience testing only.
         *       Not cryptographically secure without proper entropy source.
         *       The generated key is deterministic (pseudo-random), not truly random.
         */
        [[nodiscard]] static XorBitCipher createWithPseudoRandomKey(size_t key_length);

    private:
        /// @brief Number of bits in a byte
        static constexpr uint8_t BITS_PER_BYTE = 8;

        /// @brief Most significant bit position (0-indexed from left)
        static constexpr uint8_t MSB_POSITION = 7;

        /** @brief The key stream (keystream) for XOR operations */
        std::vector<uint8_t> key_stream_;

        /** @brief Current position in key stream (byte index) */
        size_t key_pos_{0};

        /** @brief Current bit position within current byte (0-7) */
        uint8_t bit_pos_{0};

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
        [[nodiscard]] uint8_t nextKeyByte();

        /**
         * @brief Get next key bit and advance position.
         * @return Next key bit (0 or 1) for XOR operation.
         * @throw std::invalid_argument if key is empty.
         */
        [[nodiscard]] bool nextKeyBit();
    };
}
