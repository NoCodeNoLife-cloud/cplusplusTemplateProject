#pragma once

#include <fstream>
#include <vector>

#include "AbstractWriter.hpp"

namespace common::filesystem {
    /// @brief A buffered writer that writes data to an output stream with buffering.
    /// This class provides efficient writing operations by buffering the output data
    /// before writing it to the underlying output stream.
    class BufferedWriter final : public AbstractWriter, public interfaces::IAppendable<BufferedWriter> {
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
        auto write(const std::string &str) -> void override;

        /// @brief Writes a portion of a character buffer to the buffer.
        /// @param cBuf The character buffer to write from.
        /// @param off The offset in the buffer to start writing from.
        /// @param len The number of characters to write.
        /// @throws std::out_of_range if offset and length are out of the bounds of the buffer.
        auto write(const std::vector<char> &cBuf, size_t off, size_t len) -> void override;

        /// @brief Flushes the buffer to the underlying output stream.
        /// @throws std::ios_base::failure if an I/O error occurs.
        auto flush() -> void override;

        /// @brief Closes the writer and releases any resources.
        /// @throws std::ios_base::failure if an I/O error occurs.
        auto close() -> void override;

        /// @brief Appends a single character to the buffer.
        /// @param c The character to append.
        /// @return A reference to this BufferedWriter instance.
        auto append(char c) -> BufferedWriter & override;

        /// @brief Appends a string to the buffer.
        /// @param str The string to append.
        /// @return A reference to this BufferedWriter instance.
        auto append(const std::string &str) -> BufferedWriter & override;

        /// @brief Appends a substring to the buffer.
        /// @param str The string to append.
        /// @param start The starting index of the substring.
        /// @param end The ending index of the substring.
        /// @return A reference to this BufferedWriter instance.
        auto append(const std::string &str, size_t start, size_t end) -> BufferedWriter & override;

        /// @brief Appends a string view to the buffer.
        /// @param str The string view to append.
        /// @return A reference to this BufferedWriter instance.
        auto append(std::string_view str) -> BufferedWriter & override;

        /// @brief Appends a C-string to the buffer.
        /// @param str The C-string to append.
        /// @return A reference to this BufferedWriter instance.
        auto append(const char *str) -> BufferedWriter & override;

        /// @brief Appends an initializer list of characters to the buffer.
        /// @param chars The initializer list of characters to append.
        /// @return A reference to this BufferedWriter instance.
        auto append(std::initializer_list<char> chars) -> BufferedWriter & override;

        /// @brief Appends a sequence of characters to the buffer.
        /// @param chars Pointer to the character sequence.
        /// @param count Number of characters to append.
        /// @return A reference to this BufferedWriter instance.
        auto append(const char *chars, size_t count) -> BufferedWriter & override;

        /// @brief Appends a character multiple times to the buffer.
        /// @param c The character to append.
        /// @param count Number of times to append the character.
        /// @return A reference to this BufferedWriter instance.
        auto append(char c, size_t count) -> BufferedWriter & override;

        /// @brief Writes a newline character to the buffer.
        /// @return A reference to this BufferedWriter instance.
        auto newLine() -> BufferedWriter &;

        /// @brief Converts the buffer content to a string representation.
        /// @return The string representation of the buffer content.
        [[nodiscard]] auto toString() const -> std::string override;

        /// @brief Checks if the writer is closed.
        /// @return True if the writer is closed, false otherwise.
        [[nodiscard]] auto isClosed() const noexcept -> bool override;

    private:
        static constexpr size_t DEFAULT_BUFFER_SIZE = 1024;
        std::unique_ptr<std::ofstream> output_stream_;
        std::vector<char> buffer_;
        size_t buffer_size_;

        /// @brief Flushes the internal buffer if it reaches the buffer size threshold.
        auto checkAndFlush() -> void;
    };
}
