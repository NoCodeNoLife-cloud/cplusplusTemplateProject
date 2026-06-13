/**
 * @file FileInputStream.hpp
 * @brief Byte-input stream from a file (std::ifstream wrapper)
 * @details Reads bytes from a file using std::ifstream in binary mode.
 *          Provides checked file open with descriptive error messages.
 *          Supports available() via seekg to determine remaining file size.
 *          Analogous to java.io.FileInputStream.
 *
 * @par Thread Safety
 * This class is **not** thread-safe (file streams have mutable internal state).
 * External synchronisation is required for concurrent access.
 *
 * @par Usage Example
 * @code
 * FileInputStream file("data.bin");
 * std::vector<std::byte> buf(1024);
 * int n = file.read(buf, 0, 1024);
 * @endcode
 */

#pragma once
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "AbstractInputStream.hpp"

namespace common::io::reader
{
    /// @brief A FileInputStream class for reading data from a file.
    /// @details This class provides functionality to read bytes from a file,
    ///          including methods for reading single bytes, multiple bytes,
    ///          skipping bytes, checking available bytes, and closing the stream.
    class FileInputStream final : public AbstractInputStream
    {
    public:
        explicit FileInputStream(const std::string& name);

        explicit FileInputStream(const char* name);

        explicit FileInputStream(const std::filesystem::path& file);

        ~FileInputStream() override;

        /// @brief Read a single byte from the stream.
        /// @return The byte read or EOF (-1) if the end of the stream is reached.
        [[nodiscard]] std::byte read() override;

        /// @brief Read bytes from the stream.
        /// @param buffer The buffer to read into.
        /// @return The number of bytes read.
        [[nodiscard]] size_t read(std::vector<std::byte>& buffer) override;

        /// @brief Read bytes from the stream.
        /// @param buffer The buffer to read into.
        /// @param offset The offset to start reading from.
        /// @param len The number of bytes to read.
        /// @return The number of bytes read.
        [[nodiscard]] size_t read(std::vector<std::byte>& buffer, size_t offset, size_t len) override;

        /// @brief Skip bytes in the stream.
        /// @param n The number of bytes to skip.
        /// @return The number of bytes skipped.
        [[nodiscard]] int64_t skip(int64_t n) override;

        /// @brief Get the number of bytes available to read.
        /// @return The number of bytes available to read.
        [[nodiscard]] size_t available() override;

        /// @brief Close the stream.
        void close() override;

        /// @brief Checks if this input stream has been closed.
        /// @return true if this input stream has been closed, false otherwise.
        [[nodiscard]] bool isClosed() const override;

        /// @brief Check if the stream supports marking.
        /// @return True if the stream supports marking, false otherwise.
        [[nodiscard]] bool markSupported() const override;

    private:
        /// @brief Check if the stream is valid for operations
        /// @return true if stream is open and not closed, false otherwise
        [[nodiscard]] bool isValid() const ;

        /// @brief Validate that the buffer parameters are within bounds
        /// @param buffer The buffer to validate
        /// @param offset The offset to start reading from
        /// @param len The number of bytes to read
        /// @throws std::out_of_range if parameters are invalid
        static void validateBufferParams(const std::vector<std::byte>& buffer, size_t offset, size_t len);

        std::ifstream file_stream_;
        std::string file_name_{};
        bool closed_{false};
    };
}
