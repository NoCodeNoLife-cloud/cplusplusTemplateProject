#pragma once

#include "HashStrategy.hpp"
#include <memory>

namespace common::crypto::hash {
    /**
     * @brief Unified toolkit for SHA hash algorithms using Strategy pattern.
     * 
     * This class provides a unified interface for different SHA hash algorithms
     * (SHA-1, SHA-256, etc.) through the Strategy design pattern. It delegates
     * all operations to the underlying HashStrategy implementation.
     * 
     * Example usage:
     * @code
     * // Using SHA-256
     * auto sha256 = SHAToolkit::createSHA256();
     * sha256.update("hello");
     * auto hash = sha256.finalize();
     * 
     * // Using SHA-1
     * auto sha1 = SHAToolkit::createSHA1();
     * sha1.update("hello");
     * auto hash = sha1.finalize();
     * @endcode
     */
    class SHAToolkit final {
    public:
        /**
         * @brief Creates a new SHAToolkit instance with the specified strategy.
         * 
         * @param strategy The hash strategy implementation to use.
         */
        explicit SHAToolkit(std::unique_ptr<HashStrategy> strategy);

        /**
         * @brief Deleted default constructor - strategy must be provided.
         */
        SHAToolkit() = delete;

        /**
         * @brief Deleted copy constructor to prevent sharing of internal state.
         */
        SHAToolkit(const SHAToolkit &) = delete;

        /**
         * @brief Deleted copy assignment to prevent sharing of internal state.
         */
        auto operator=(const SHAToolkit &) -> SHAToolkit & = delete;

        /**
         * @brief Move constructor.
         */
        [[nodiscard]] SHAToolkit(SHAToolkit &&) noexcept;

        /**
         * @brief Move assignment operator.
         */
        auto operator=(SHAToolkit &&) noexcept -> SHAToolkit &;

        /**
         * @brief Default destructor.
         */
        ~SHAToolkit();

        /**
         * @brief Gets the digest size in bytes for the current algorithm.
         * @return The size of the hash output in bytes.
         */
        [[nodiscard]] auto getDigestSize() const noexcept -> size_t;

        /**
         * @brief Gets the hex digest size in characters for the current algorithm.
         * @return The size of the hex representation.
         */
        [[nodiscard]] auto getHexDigestSize() const noexcept -> size_t;

        /**
         * @brief Updates the hash computation with additional data.
         * 
         * @param data Pointer to the data buffer.
         * @param length Length of the data in bytes.
         * @return true if update succeeded, false otherwise.
         */
        [[nodiscard]] auto update(const void *data, size_t length) noexcept -> bool;

        /**
         * @brief Updates the hash computation with a string view.
         * 
         * @param data String data to hash.
         * @return true if update succeeded, false otherwise.
         */
        [[nodiscard]] auto update(std::string_view data) noexcept -> bool;

        /**
         * @brief Finalizes the hash computation and returns the digest.
         * 
         * @return Optional containing the binary digest, or nullopt on failure.
         */
        [[nodiscard]] auto finalize() noexcept -> std::optional<std::vector<uint8_t>>;

        /**
         * @brief Resets the toolkit for a new hashing operation.
         * 
         * @return true if reset succeeded, false otherwise.
         */
        [[nodiscard]] auto reset() noexcept -> bool;

        /**
         * @brief Converts binary digest to hexadecimal string representation.
         * 
         * @param digest Binary digest bytes.
         * @return Hexadecimal string.
         */
        [[nodiscard]] static auto toHexString(const std::vector<uint8_t> &digest, size_t expected_size) -> std::string;

        /**
         * @brief Creates a SHAToolkit configured for SHA-256.
         * @return A new SHAToolkit instance using SHA-256 algorithm.
         */
        [[nodiscard]] static auto createSHA256() -> SHAToolkit;

        /**
         * @brief Creates a SHAToolkit configured for SHA-1.
         * @return A new SHAToolkit instance using SHA-1 algorithm.
         * @warning SHA-1 is cryptographically broken. Use SHA-256 for security applications.
         */
        [[nodiscard]] static auto createSHA1() -> SHAToolkit;

        /**
         * @brief Computes SHA-256 hash of a string in one operation.
         * 
         * @param input String to hash.
         * @return Optional containing the binary digest, or nullopt on failure.
         */
        [[nodiscard]] static auto hashStringSHA256(std::string_view input) noexcept -> std::optional<std::vector<uint8_t>>;

        /**
         * @brief Computes SHA-1 hash of a string in one operation.
         * 
         * @param input String to hash.
         * @return Optional containing the binary digest, or nullopt on failure.
         */
        [[nodiscard]] static auto hashStringSHA1(std::string_view input) noexcept -> std::optional<std::vector<uint8_t>>;

        /**
         * @brief Computes SHA-256 hash of a file in one operation.
         * 
         * @param filePath Path to the file to hash.
         * @param chunkSize Buffer size for reading file (default 8192 bytes).
         * @return Optional containing the binary digest, or nullopt on failure.
         */
        [[nodiscard]] static auto hashFileSHA256(const std::string &filePath, size_t chunkSize = 8192) -> std::optional<std::vector<uint8_t>>;

        /**
         * @brief Computes SHA-1 hash of a file in one operation.
         * 
         * @param filePath Path to the file to hash.
         * @param chunkSize Buffer size for reading file (default 8192 bytes).
         * @return Optional containing the binary digest, or nullopt on failure.
         */
        [[nodiscard]] static auto hashFileSHA1(const std::string &filePath, size_t chunkSize = 8192) -> std::optional<std::vector<uint8_t>>;

        /**
         * @brief Computes SHA-256 hash of a string and returns hex representation.
         * 
         * @param input String to hash.
         * @return Hexadecimal hash string, or empty string on failure.
         */
        [[nodiscard]] static auto hashStringToHexSHA256(std::string_view input) noexcept -> std::string;

        /**
         * @brief Computes SHA-1 hash of a string and returns hex representation.
         * 
         * @param input String to hash.
         * @return Hexadecimal hash string, or empty string on failure.
         */
        [[nodiscard]] static auto hashStringToHexSHA1(std::string_view input) noexcept -> std::string;

        /**
         * @brief Computes SHA-256 hash of a file and returns hex representation.
         * 
         * @param filePath Path to the file to hash.
         * @return Hexadecimal hash string, or empty string on failure.
         */
        [[nodiscard]] static auto hashFileToHexSHA256(const std::string &filePath) -> std::string;

        /**
         * @brief Computes SHA-1 hash of a file and returns hex representation.
         * 
         * @param filePath Path to the file to hash.
         * @return Hexadecimal hash string, or empty string on failure.
         */
        [[nodiscard]] static auto hashFileToHexSHA1(const std::string &filePath) -> std::string;

    private:
        std::unique_ptr<HashStrategy> strategy_;
    };
}
