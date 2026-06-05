/**
 * @file BufferedWriter.hpp
 * @brief BufferedWriter class declaration
 * @details This header defines the BufferedWriter class that provides functionality for Common library utilities.
 */

#pragma once
#include <fstream>
#include <span>
#include <vector>

#include "AbstractWriter.hpp"

namespace common::filesystem
{
    /// @brief A buffered writer that writes data to an output stream with buffering.
    /// This class provides efficient writing operations by buffering the output data
    /// before writing it to the underlying output stream.
    class BufferedWriter final : public AbstractWriter, public interfaces::IAppendable<BufferedWriter>
    {
    public:
        /// @brief Constructs a buffered writer with specified buffer size.
        /// @param os The underlying output file stream.
        /// @param size The buffer size in bytes.
        /// @throws std::runtime_error if the output stream is null or not open.
        explicit BufferedWriter(std::unique_ptr<std::ofstream> os, size_t size = DEFAULT_BUFFER_SIZE);

        /// @brief Destructor that closes the writer.
        ~BufferedWriter() override;

        /// @brief Writes a string to the buffer.
        /// @param str The string to write.
        void write(const std::string& str) override;

        /// @brief Writes a portion of a character buffer to the buffer.
        /// @param cBuf The character buffer to write from.
        /// @param off The offset in the buffer to start writing from.
        /// @param len The number of characters to write.
        /// @throws std::out_of_range if offset and length are out of the bounds of the buffer.
        void write(const std::vector<char>& cBuf, size_t off, size_t len) override;

        /// @brief Flushes the buffer to the underlying output stream.
        /// @throws std::ios_base::failure if an I/O error occurs.
        void flush() override;

        /// @brief Closes the writer and releases any resources.
        /// @throws std::ios_base::failure if an I/O error occurs.
        void close() override;

        /// @brief Appends a single character to the buffer.
        /// @param c The character to append.
        /// @return A reference to this BufferedWriter instance.
        BufferedWriter& append(char c) override;

        /// @brief Appends a string to the buffer.
        /// @param str The string to append.
        /// @return A reference to this BufferedWriter instance.
        BufferedWriter& append(const std::string& str) override;

        /// @brief Appends a substring to the buffer.
        /// @param str The string to append.
        /// @param start The starting index of the substring.
        /// @param end The ending index of the substring.
        /// @return A reference to this BufferedWriter instance.
        BufferedWriter& append(const std::string& str, size_t start, size_t end) override;

        /// @brief Appends a string view to the buffer.
        /// @param str The string view to append.
        /// @return A reference to this BufferedWriter instance.
        BufferedWriter& append(std::string_view str) override;

        /// @brief Appends a C-string to the buffer.
        /// @param str The C-string to append.
        /// @return A reference to this BufferedWriter instance.
        BufferedWriter& append(const char* str) override;

        /// @brief Appends an initializer list of characters to the buffer.
        /// @param chars The initializer list of characters to append.
        /// @return A reference to this BufferedWriter instance.
        BufferedWriter& append(std::initializer_list<char> chars) override;

        /// @brief Appends a sequence of characters to the buffer.
        /// @param chars Pointer to the character sequence.
        /// @param count Number of characters to append.
        /// @return A reference to this BufferedWriter instance.
        BufferedWriter& append(const char* chars, size_t count) override;

        /// @brief Appends a character multiple times to the buffer.
        /// @param c The character to append.
        /// @param count Number of times to append the character.
        /// @return A reference to this BufferedWriter instance.
        BufferedWriter& append(char c, size_t count) override;

        /// @brief Appends a span of characters to the buffer.
        /// @param chars The span of characters to append.
        /// @return A reference to this BufferedWriter instance.
        BufferedWriter& append(std::span<const char> chars) override;

        /// @brief Writes a newline character to the buffer.
        /// @return A reference to this BufferedWriter instance.
        BufferedWriter& newLine();

        /// @brief Converts the buffer content to a string representation.
        /// @return The string representation of the buffer content.
        [[nodiscard]] std::string toString() const override;

        /// @brief Checks if the writer is closed.
        /// @return True if the writer is closed, false otherwise.
        [[nodiscard]] bool isClosed() const override;

    private:
        static constexpr size_t DEFAULT_BUFFER_SIZE = 1024;
        std::unique_ptr<std::ofstream> output_stream_;
        std::vector<char> buffer_;
        size_t buffer_size_;

        /// @brief Flushes the internal buffer if it reaches the buffer size threshold.
        void checkAndFlush();
    };
}
