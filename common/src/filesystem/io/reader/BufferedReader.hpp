/**
 * @file BufferedReader.hpp
 * @brief BufferedReader class declaration
 * @details This header defines the BufferedReader class that provides functionality for Common library utilities.
 */

#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include "AbstractReader.hpp"

namespace common::filesystem
{
    /// @brief Buffered character-input stream.
    class BufferedReader final : public AbstractReader
    {
    public:
        explicit BufferedReader(std::unique_ptr<AbstractReader> reader, size_t size = DEFAULT_BUFFER_SIZE);

        ~BufferedReader() override = default;

        /// @brief Close the stream.
        auto close() -> void override;

        /// @brief Mark the present position in the stream.
        /// @param readAheadLimit The maximum limit of characters that can be read before the mark position becomes invalid.
        auto mark(size_t readAheadLimit) -> void override;

        /// @brief Tell whether this stream supports the mark() operation.
        /// @return true if the stream supports mark(), false otherwise.
        [[nodiscard]] auto markSupported() const -> bool override;

        /// @brief Reset the stream to the most recent mark.
        auto reset() -> void override;

        /// @brief Read a single character.
        /// @return The character read, as an integer in the range 0 to 65535 (0x00-0xffff),
        ///         or -1 if the end of the stream has been reached.
        auto read() -> int override;

        /// @brief Read characters into an array.
        /// @param cBuf The buffer into which characters are read.
        /// @param off The offset at which to start storing characters.
        /// @param len The maximum number of characters to read.
        /// @return The number of characters read, or -1 if the end of the stream has been reached.
        auto read(std::vector<char>& cBuf, size_t off, size_t len) -> int override;

        /// @brief Read a line of text.
        /// @return A String containing the next line of text, or empty string if the end of the stream has been reached.
        [[nodiscard]] auto readLine() -> std::string;

        /// @brief Tell whether this stream is ready to be read.
        /// @return true if the next read() is guaranteed not to block for input, false otherwise.
        [[nodiscard]] auto ready() const -> bool override;

        /// @brief Skip characters.
        /// @param n The number of characters to skip.
        /// @return The number of characters actually skipped.
        auto skip(size_t n) -> size_t override;

        /// @brief Checks if this reader has been closed.
        /// @return true if this reader has been closed, false otherwise.
        [[nodiscard]] auto isClosed() const -> bool override;

    private:
        static constexpr size_t DEFAULT_BUFFER_SIZE = 8192;
        std::vector<char> buffer_;
        std::unique_ptr<AbstractReader> reader_;
        size_t buffer_size_{0};
        size_t pos_{0};
        size_t count_{0};
        size_t mark_limit_{0};

        /// @brief Fills the internal buffer from the underlying reader.
        /// @return true if data was read, false if end of stream reached.
        [[nodiscard]] auto fillBuffer() -> bool;
    };
}
