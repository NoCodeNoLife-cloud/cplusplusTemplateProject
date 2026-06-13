/**
 * @file SHAToolkit.hpp
 * @brief SHA hash utilities — convenience methods for SHA-1 / SHA-256
 * @details Provides static convenience methods for computing SHA digests
 *          from strings, byte arrays, and files.  Routes to the appropriate
 *          HashStrategy (SHA1Strategy or SHA256Strategy) based on the
 *          requested bit length.
 *
 * @par Thread Safety
 * Static methods are thread-safe as long as input parameters remain valid.
 */

#pragma once
#include <memory>

#include "HashStrategy.hpp"

namespace common::crypto::hash
{
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
    class SHAToolkit final
    {
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
        SHAToolkit(const SHAToolkit&) = delete;

        /**
         * @brief Deleted copy assignment to prevent sharing of internal state.
         */
        SHAToolkit& operator=(const SHAToolkit&) = delete;

        /**
         * @brief Move constructor.
         */
        [[nodiscard]] SHAToolkit(SHAToolkit&&) ;

        /**
         * @brief Move assignment operator.
         */
        SHAToolkit& operator=(SHAToolkit&&) ;

        /**
         * @brief Default destructor.
         */
        ~SHAToolkit();

        /**
         * @brief Gets the digest size in bytes for the current algorithm.
         * @return The size of the hash output in bytes.
         */
        [[nodiscard]] size_t getDigestSize() const ;

        /**
         * @brief Gets the hex digest size in characters for the current algorithm.
         * @return The size of the hex representation.
         */
        [[nodiscard]] size_t getHexDigestSize() const ;

        /**
         * @brief Updates the hash computation with additional data.
         *
         * @param data Pointer to the data buffer.
         * @param length Length of the data in bytes.
         * @return true if update succeeded, false otherwise.
         */
        [[nodiscard]] bool update(const void* data, size_t length) ;

        /**
         * @brief Updates the hash computation with a string view.
         *
         * @param data String data to hash.
         * @return true if update succeeded, false otherwise.
         */
        [[nodiscard]] bool update(std::string_view data) ;

        /**
         * @brief Finalizes the hash computation and returns the digest.
         *
         * @return Optional containing the binary digest, or nullopt on failure.
         */
        [[nodiscard]] std::optional<std::vector<uint8_t>> finalize() ;

        /**
         * @brief Resets the toolkit for a new hashing operation.
         *
         * @return true if reset succeeded, false otherwise.
         */
        [[nodiscard]] bool reset() ;

        /**
         * @brief Converts binary digest to hexadecimal string representation.
         *
         * @param digest Binary digest bytes.
         * @return Optional containing hexadecimal string, or nullopt on failure.
         */
        [[nodiscard]] static std::optional<std::string> toHexString(const std::vector<uint8_t>& digest, size_t expected_size);

        /**
         * @brief Creates a SHAToolkit configured for SHA-256.
         * @return A new SHAToolkit instance using SHA-256 algorithm.
         */
        [[nodiscard]] static SHAToolkit createSHA256();

        /**
         * @brief Creates a SHAToolkit configured for SHA-1.
         * @return A new SHAToolkit instance using SHA-1 algorithm.
         * @warning SHA-1 is cryptographically broken. Use SHA-256 for security applications.
         */
        [[nodiscard]] static SHAToolkit createSHA1();

        /**
         * @brief Computes SHA-256 hash of a string in one operation.
         *
         * @param input String to hash.
         * @return Optional containing the binary digest, or nullopt on failure.
         */
        [[nodiscard]] static std::optional<std::vector<uint8_t>> hashStringSHA256(std::string_view input) ;

        /**
         * @brief Computes SHA-1 hash of a string in one operation.
         *
         * @param input String to hash.
         * @return Optional containing the binary digest, or nullopt on failure.
         */
        [[nodiscard]] static std::optional<std::vector<uint8_t>> hashStringSHA1(std::string_view input) ;

        /**
         * @brief Computes SHA-256 hash of a file in one operation.
         *
         * @param filePath Path to the file to hash.
         * @param chunkSize Buffer size for reading file (default 8192 bytes).
         * @return Optional containing the binary digest, or nullopt on failure.
         */
        [[nodiscard]] static std::optional<std::vector<uint8_t>> hashFileSHA256(const std::string& filePath, size_t chunkSize = 8192);

        /**
         * @brief Computes SHA-1 hash of a file in one operation.
         *
         * @param filePath Path to the file to hash.
         * @param chunkSize Buffer size for reading file (default 8192 bytes).
         * @return Optional containing the binary digest, or nullopt on failure.
         */
        [[nodiscard]] static std::optional<std::vector<uint8_t>> hashFileSHA1(const std::string& filePath, size_t chunkSize = 8192);

        /**
         * @brief Computes SHA-256 hash of a string and returns hex representation.
         *
         * @param input String to hash.
         * @return Optional containing hexadecimal hash string, or nullopt on failure.
         */
        [[nodiscard]] static std::optional<std::string> hashStringToHexSHA256(std::string_view input) ;

        /**
         * @brief Computes SHA-1 hash of a string and returns hex representation.
         *
         * @param input String to hash.
         * @return Optional containing hexadecimal hash string, or nullopt on failure.
         */
        [[nodiscard]] static std::optional<std::string> hashStringToHexSHA1(std::string_view input) ;

        /**
         * @brief Computes SHA-256 hash of a file and returns hex representation.
         *
         * @param filePath Path to the file to hash.
         * @return Optional containing hexadecimal hash string, or nullopt on failure.
         */
        [[nodiscard]] static std::optional<std::string> hashFileToHexSHA256(const std::string& filePath, size_t chunkSize = 8192);

        /**
         * @brief Computes SHA-1 hash of a file and returns hex representation.
         *
         * @param filePath Path to the file to hash.
         * @param chunkSize Buffer size for reading file (default 8192 bytes).
         * @return Optional containing hexadecimal hash string, or nullopt on failure.
         */
        [[nodiscard]] static std::optional<std::string> hashFileToHexSHA1(const std::string& filePath, size_t chunkSize = 8192);

    private:
        std::unique_ptr<HashStrategy> strategy_;
    };
}
