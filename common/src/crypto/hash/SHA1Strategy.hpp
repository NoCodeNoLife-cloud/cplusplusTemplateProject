#pragma once

#include "HashStrategy.hpp"
#include <openssl/evp.h>
#include <memory>

namespace common::crypto::hash {
    /**
     * @brief Concrete strategy implementation for SHA-1 hash algorithm.
     * 
     * This class implements the HashStrategy interface using OpenSSL's EVP API
     * to provide SHA-1 hashing functionality.
     * 
     * @warning SHA-1 is considered cryptographically broken and should not be used
     * for security-sensitive applications. Use SHA-256 or stronger algorithms instead.
     */
    class SHA1Strategy final : public HashStrategy {
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
        SHA1Strategy(const SHA1Strategy &) = delete;

        /**
         * @brief Deleted copy assignment to prevent sharing of internal state.
         */
        auto operator=(const SHA1Strategy &) -> SHA1Strategy & = delete;

        /**
         * @brief Move constructor.
         */
        [[nodiscard]] SHA1Strategy(SHA1Strategy &&) noexcept;

        /**
         * @brief Move assignment operator.
         */
        auto operator=(SHA1Strategy &&) noexcept -> SHA1Strategy &;

        /**
         * @brief Default destructor.
         */
        ~SHA1Strategy() override;

        /**
         * @brief Gets the digest size in bytes (20 for SHA-1).
         */
        [[nodiscard]] auto getDigestSize() const noexcept -> size_t override;

        /**
         * @brief Gets the hex digest size in characters (40 for SHA-1).
         */
        [[nodiscard]] auto getHexDigestSize() const noexcept -> size_t override;

        /**
         * @brief Updates the hash computation with additional data.
         * 
         * @param data Pointer to the data buffer.
         * @param length Length of the data in bytes.
         * @return true if update succeeded, false otherwise.
         */
        [[nodiscard]] auto update(const void *data, size_t length) noexcept -> bool override;

        /**
         * @brief Finalizes the hash computation and returns the digest.
         * 
         * @return Optional containing the 20-byte digest, or nullopt on failure.
         */
        [[nodiscard]] auto finalize() noexcept -> std::optional<std::vector<uint8_t>> override;

        /**
         * @brief Resets the toolkit for a new hashing operation.
         * 
         * @return true if reset succeeded, false otherwise.
         */
        [[nodiscard]] auto reset() noexcept -> bool override;

    private:
        /**
         * @brief Custom deleter for OpenSSL EVP_MD_CTX.
         */
        struct EvpDeleter {
            auto operator()(EVP_MD_CTX *ctx) const noexcept -> void;
        };

        std::unique_ptr<EVP_MD_CTX, EvpDeleter> ctx_{EVP_MD_CTX_new()};
        bool finalized_{false};

        /**
         * @brief Validates that the context was properly allocated.
         * @throws std::runtime_error If context allocation failed.
         */
        auto validateContext() const -> void;
    };
}
