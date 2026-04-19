#pragma once

#include "HashStrategy.hpp"
#include <openssl/evp.h>
#include <memory>

namespace common::crypto::hash {
    /**
     * @brief Concrete strategy implementation for SHA-256 hash algorithm.
     * 
     * This class implements the HashStrategy interface using OpenSSL's EVP API
     * to provide SHA-256 hashing functionality.
     */
    class SHA256Strategy final : public HashStrategy {
    public:
        /**
         * @brief Size of SHA-256 digest output in bytes.
         */
        static constexpr size_t DIGEST_SIZE = 32;
        /**
         * @brief Size of SHA-256 digest output in hex characters.
         */
        static constexpr size_t HEX_DIGEST_SIZE = 64;

        /**
         * @brief Constructs a new SHA256Strategy and initializes the digest context.
         * @throws std::runtime_error If OpenSSL context initialization fails.
         */
        [[nodiscard]] SHA256Strategy();

        /**
         * @brief Deleted copy constructor to prevent sharing of internal state.
         */
        SHA256Strategy(const SHA256Strategy &) = delete;

        /**
         * @brief Deleted copy assignment to prevent sharing of internal state.
         */
        auto operator=(const SHA256Strategy &) -> SHA256Strategy & = delete;

        /**
         * @brief Move constructor.
         */
        [[nodiscard]] SHA256Strategy(SHA256Strategy &&) noexcept;

        /**
         * @brief Move assignment operator.
         */
        auto operator=(SHA256Strategy &&) noexcept -> SHA256Strategy &;

        /**
         * @brief Default destructor.
         */
        ~SHA256Strategy() override;

        /**
         * @brief Gets the digest size in bytes (32 for SHA-256).
         */
        [[nodiscard]] auto getDigestSize() const noexcept -> size_t override;

        /**
         * @brief Gets the hex digest size in characters (64 for SHA-256).
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
         * @return Optional containing the 32-byte digest, or nullopt on failure.
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
