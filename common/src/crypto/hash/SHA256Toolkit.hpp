#pragma once

#include <openssl/evp.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <memory>
#include <optional>

namespace common::crypto::hash {
    /**
     * @brief Toolkit for computing SHA-256 hashes using OpenSSL EVP interface.
     *
     * This class provides an object-oriented wrapper around OpenSSL SHA-256 implementation,
     * supporting incremental hashing, string hashing, and file hashing operations.
     *
     * @note This class is not thread-safe. Each thread should use its own instance.
     */
    class SHA256Toolkit {
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
         * @brief Constructs a new SHA256Toolkit and initializes the digest context.
         * @throws std::runtime_error If OpenSSL context initialization fails.
         */
        [[nodiscard]] SHA256Toolkit();

        /**
         * @brief Deleted copy constructor to prevent sharing of internal state.
         */
        SHA256Toolkit(const SHA256Toolkit &) = delete;

        /**
         * @brief Deleted copy assignment to prevent sharing of internal state.
         */
        auto operator=(const SHA256Toolkit &) -> SHA256Toolkit & = delete;

        /**
         * @brief Move constructor.
         */
        [[nodiscard]] SHA256Toolkit(SHA256Toolkit &&) noexcept;

        /**
         * @brief Move assignment operator.
         */
        auto operator=(SHA256Toolkit &&) noexcept -> SHA256Toolkit &;

        /**
         * @brief Default destructor.
         */
        ~SHA256Toolkit();

        /**
         * @brief Updates the hash computation with additional data.
         *
         * @param data Pointer to the data buffer.
         * @param length Length of the data in bytes.
         * @return true if update succeeded, false otherwise.
         */
        [[nodiscard]] auto update(const void *data, size_t length) const noexcept -> bool;

        /**
         * @brief Updates the hash computation with a string view.
         *
         * @param data String data to hash.
         * @return true if update succeeded, false otherwise.
         */
        [[nodiscard]] auto update(std::string_view data) const noexcept -> bool;

        /**
         * @brief Finalizes the hash computation and returns the digest.
         *
         * After calling this method, the object cannot be used for further updates
         * unless reset() is called.
         *
         * @return Optional containing the 32-byte digest, or nullopt on failure.
         */
        [[nodiscard]] auto finalize() noexcept -> std::optional<std::vector<uint8_t> >;

        /**
         * @brief Resets the toolkit for a new hashing operation.
         *
         * This reinitializes the internal state, allowing the object to be reused.
         *
         * @return true if reset succeeded, false otherwise.
         */
        [[nodiscard]] auto reset() noexcept -> bool;

        /**
         * @brief Converts binary digest to hexadecimal string representation.
         *
         * @param digest Binary digest bytes (must be DIGEST_SIZE bytes).
         * @return Hexadecimal string of length HEX_DIGEST_SIZE.
         */
        [[nodiscard]] static auto toHexString(const std::vector<uint8_t> &digest) -> std::string;

        /**
         * @brief Computes SHA-256 hash of a string in one operation.
         *
         * @param input String to hash.
         * @return Optional containing the binary digest, or nullopt on failure.
         */
        [[nodiscard]] static auto hashString(std::string_view input) noexcept -> std::optional<std::vector<uint8_t> >;

        /**
         * @brief Computes SHA-256 hash of a file in one operation.
         *
         * @param filePath Path to the file to hash.
         * @param chunkSize Buffer size for reading file (default 8192 bytes).
         * @return Optional containing the binary digest, or nullopt on failure.
         */
        [[nodiscard]] static auto hashFile(const std::string &filePath, size_t chunkSize = 8192) -> std::optional<std::vector<uint8_t> >;

        /**
         * @brief Computes SHA-256 hash of a string and returns hex representation.
         *
         * @param input String to hash.
         * @return Hexadecimal hash string, or empty string on failure.
         */
        [[nodiscard]] static auto hashStringToHex(std::string_view input) noexcept -> std::string;

        /**
         * @brief Computes SHA-256 hash of a file and returns hex representation.
         *
         * @param filePath Path to the file to hash.
         * @return Hexadecimal hash string, or empty string on failure.
         */
        [[nodiscard]] static auto hashFileToHex(const std::string &filePath) -> std::string;

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
