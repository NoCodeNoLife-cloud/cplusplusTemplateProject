/**
 * @file StreamCipherFactory.hpp
 * @brief StreamCipherFactory class declaration
 * @details This header defines the StreamCipherFactory class that provides functionality for Cryptographic utilities and toolkit.
 */

#pragma once
#include <memory>
#include <string>

#include "StreamCipher.hpp"

namespace common::crypto::cipher
{
    /**
     * @class StreamCipherFactory
     * @brief Factory class for creating stream cipher instances.
     *
     * This factory provides a centralized way to create different stream cipher
     * implementations using the Strategy pattern. It supports runtime selection
     * of encryption algorithms.
     *
     * @example
     * @code
     * // Create ChaCha20 cipher
     * auto cipher = StreamCipherFactory::create(StreamCipherFactory::Algorithm::CHACHA20);
     *
     * // Or use convenience method
     * auto cipher2 = StreamCipherFactory::createChaCha20();
     * @endcode
     */
    class StreamCipherFactory
    {
    public:
        /**
         * @enum Algorithm
         * @brief Supported stream cipher algorithms.
         */
        enum class Algorithm
        {
            CHACHA20,
            ///< ChaCha20 (RFC 8439, IETF variant with 96-bit nonce)
        };

        /// @brief Deleted constructor - this is a factory class with only static methods
        StreamCipherFactory() = delete;

        /**
         * @brief Create a stream cipher instance.
         * @param algo The algorithm to instantiate.
         * @return Unique pointer to the cipher implementation.
         * @throws std::invalid_argument if algorithm is not supported.
         * @throws std::runtime_error if cipher initialization fails.
         */
        [[nodiscard]] static std::unique_ptr<StreamCipher> create(Algorithm algo);

        /**
         * @brief Create a ChaCha20 cipher instance (convenience method).
         * @return Unique pointer to ChaCha20Cipher.
         * @throws std::runtime_error if cipher creation fails.
         */
        [[nodiscard]] static std::unique_ptr<StreamCipher> createChaCha20();

        /**
         * @brief Get list of supported algorithm names.
         * @return Vector of algorithm name strings.
         */
        [[nodiscard]] static std::vector<std::string> getSupportedAlgorithms();

        /**
         * @brief Check if an algorithm is supported.
         * @param algo The algorithm to check.
         * @return true if supported, false otherwise.
         */
        [[nodiscard]] static bool isSupported(Algorithm algo) ;

    private:
        /**
         * @brief Convert algorithm enum to string.
         * @param algo The algorithm enum value.
         * @return String representation.
         */
        [[nodiscard]] static std::string algorithmToString(Algorithm algo);
    };
}
