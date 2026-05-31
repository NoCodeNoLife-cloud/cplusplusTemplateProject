/**
 * @file HashStrategy.hpp
 * @brief HashStrategy class declaration
 * @details This header defines the HashStrategy class that provides functionality for Cryptographic utilities and toolkit.
 */

#pragma once

#include <vector>
#include <string>
#include <optional>
#include <memory>

namespace common::crypto::hash
{
    /**
     * @brief Abstract strategy interface for hash algorithms.
     * 
     * This class defines the common interface for all hash algorithm implementations,
     * following the Strategy design pattern. It supports incremental hashing,
     * one-shot hashing, and file hashing operations.
     */
    class HashStrategy
    {
    public:
        virtual ~HashStrategy() = default;

        /**
         * @brief Gets the digest size in bytes for this hash algorithm.
         * @return The size of the hash output in bytes.
         */
        [[nodiscard]] virtual size_t getDigestSize() const noexcept = 0;

        /**
         * @brief Gets the hex digest size in characters for this hash algorithm.
         * @return The size of the hex representation (digest_size * 2).
         */
        [[nodiscard]] virtual size_t getHexDigestSize() const noexcept = 0;

        /**
         * @brief Updates the hash computation with additional data.
         *
         * @param data Pointer to the data buffer.
         * @param length Length of the data in bytes.
         * @return true if update succeeded, false otherwise.
         */
        [[nodiscard]] virtual bool update(const void* data, size_t length) noexcept = 0;

        /**
         * @brief Updates the hash computation with a string view.
         *
         * @param data String data to hash.
         * @return true if update succeeded, false otherwise.
         */
        [[nodiscard]] bool update(std::string_view data) noexcept
        {
            return update(data.data(), data.size());
        }

        /**
         * @brief Finalizes the hash computation and returns the digest.
         *
         * After calling this method, the object cannot be used for further updates
         * unless reset() is called.
         *
         * @return Optional containing the binary digest, or nullopt on failure.
         */
        [[nodiscard]] virtual std::optional<std::vector<uint8_t>> finalize() noexcept = 0;

        /**
         * @brief Resets the toolkit for a new hashing operation.
         *
         * This reinitializes the internal state, allowing the object to be reused.
         *
         * @return true if reset succeeded, false otherwise.
         */
        [[nodiscard]] virtual bool reset() noexcept = 0;

        /**
         * @brief Converts binary digest to hexadecimal string representation.
         *
         * @param digest Binary digest bytes (must match getDigestSize()).
         * @return Optional containing hexadecimal string of length getHexDigestSize(), or nullopt on failure.
         */
        [[nodiscard]] static std::optional<std::string> toHexString(const std::vector<uint8_t>& digest, size_t expected_size);

        /**
         * @brief Computes hash of a string in one operation.
         *
         * @param input String to hash.
         * @return Optional containing the binary digest, or nullopt on failure.
         */
        [[nodiscard]] static std::optional<std::vector<uint8_t>> hashString(std::unique_ptr<HashStrategy> strategy, std::string_view input) noexcept;

        /**
         * @brief Computes hash of a file in one operation.
         *
         * @param filePath Path to the file to hash.
         * @param chunkSize Buffer size for reading file (default 8192 bytes).
         * @return Optional containing the binary digest, or nullopt on failure.
         */
        [[nodiscard]] static std::optional<std::vector<uint8_t>> hashFile(std::unique_ptr<HashStrategy> strategy, const std::string& filePath, size_t chunkSize = 8192);

        /**
         * @brief Computes hash of a string and returns hex representation.
         *
         * @param input String to hash.
         * @return Optional containing hexadecimal hash string, or nullopt on failure.
         */
        [[nodiscard]] static std::optional<std::string> hashStringToHex(std::unique_ptr<HashStrategy> strategy, std::string_view input) noexcept;

        /**
         * @brief Computes hash of a file and returns hex representation.
         *
         * @param filePath Path to the file to hash.
         * @return Optional containing hexadecimal hash string, or nullopt on failure.
         */
        [[nodiscard]] static std::optional<std::string> hashFileToHex(std::unique_ptr<HashStrategy> strategy, const std::string& filePath);
    };
}