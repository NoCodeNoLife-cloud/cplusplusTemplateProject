/**
 * @file FileInputStream.hpp
 * @brief FileInputStream class declaration
 * @details This header defines the FileInputStream class that provides functionality for Common library utilities.
 */

#pragma once
#include <filesystem>
#include <fstream>
#include <cstddef>
#include <vector>

#include "AbstractInputStream.hpp"
#include <string>

namespace common::filesystem
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
        [[nodiscard]] size_t skip(size_t n) override;

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