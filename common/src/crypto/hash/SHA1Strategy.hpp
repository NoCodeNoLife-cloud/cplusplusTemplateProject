/**
 * @file SHA1Strategy.hpp
 * @brief SHA-1 hash strategy — 160-bit digest via OpenSSL
 * @details Implements the HashStrategy interface using the SHA-1 algorithm
 *          (160-bit output).  Wraps OpenSSL's EVP_Digest API.  SHA-1 is
 *          considered cryptographically broken for collision resistance;
 *          use SHA256Strategy for security-sensitive contexts.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Reference
 * FIPS PUB 180-4, "Secure Hash Standard (SHS)".
 */

#pragma once
#include <memory>
#include <openssl/evp.h>

#include "HashStrategy.hpp"

namespace common::crypto::hash
{
    /**
     * @brief Concrete strategy implementation for SHA-1 hash algorithm.
     * 
     * This class implements the HashStrategy interface using OpenSSL's EVP API
     * to provide SHA-1 hashing functionality.
     * 
     * @warning SHA-1 is considered cryptographically broken and should not be used
     * for security-sensitive applications. Use SHA-256 or stronger algorithms instead.
     */
    class SHA1Strategy final : public HashStrategy
    {
    public:
        /**
         * @brief Size of SHA-1 digest output in bytes.
         */
        static constexpr size_t DIGEST_SIZE = 20;
        /**
         * @brief Size of SHA-1 digest output in hex characters.
         */
        static constexpr size_t HEX_DIGEST_SIZE = 40;

        /**
         * @brief Constructs a new SHA1Strategy and initializes the digest context.
         * @throws std::runtime_error If OpenSSL context initialization fails.
         */
        [[nodiscard]] SHA1Strategy();

        /**
         * @brief Deleted copy constructor to prevent sharing of internal state.
         */
        SHA1Strategy(const SHA1Strategy&) = delete;

        /**
         * @brief Deleted copy assignment to prevent sharing of internal state.
         */
        SHA1Strategy& operator=(const SHA1Strategy&) = delete;

        /**
         * @brief Move constructor.
         */
        [[nodiscard]] SHA1Strategy(SHA1Strategy&&) noexcept;

        /**
         * @brief Move assignment operator.
         */
        SHA1Strategy& operator=(SHA1Strategy&&) noexcept;

        /**
         * @brief Default destructor.
         */
        ~SHA1Strategy() override;

        /**
         * @brief Gets the digest size in bytes (20 for SHA-1).
         */
        [[nodiscard]] size_t getDigestSize() const override;

        /**
         * @brief Gets the hex digest size in characters (40 for SHA-1).
         */
        [[nodiscard]] size_t getHexDigestSize() const override;

        /**
         * @brief Updates the hash computation with additional data.
         *
         * @param data Pointer to the data buffer.
         * @param length Length of the data in bytes.
         * @return true if update succeeded, false otherwise.
         */
        [[nodiscard]] bool update(const void* data, size_t length) override;

        using HashStrategy::update;

        /**
         * @brief Finalizes the hash computation and returns the digest.
         *
         * @return Optional containing the 20-byte digest, or nullopt on failure.
         */
        [[nodiscard]] std::optional<std::vector<uint8_t>> finalize() override;

        /**
         * @brief Resets the toolkit for a new hashing operation.
         *
         * @return true if reset succeeded, false otherwise.
         */
        [[nodiscard]] bool reset() override;

    private:
        std::unique_ptr<EVP_MD_CTX, void(*)(EVP_MD_CTX*)> ctx_;
        bool finalized_{false};

    };
}
