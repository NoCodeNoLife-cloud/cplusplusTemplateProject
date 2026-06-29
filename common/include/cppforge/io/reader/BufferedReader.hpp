/**
 * @file BufferedReader.hpp
 * @brief Buffered character-input stream with readLine() support
 * @details Wraps an AbstractReader with an 8 KB default buffer to reduce
 *          per-character read overhead.  Supports mark/reset up to the buffer
 *          size and provides a convenience readLine() method.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Memory
 * Allocates a fixed-size buffer of @p size bytes on construction (default 8192).
 * The buffer persists for the lifetime of the reader.
 */

#pragma once
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <cppforge/io/reader/AbstractReader.hpp>

namespace cppforge::io::reader
{
    /// @brief Buffered character-input stream.
    class BufferedReader final : public AbstractReader
    {
    public:
        explicit BufferedReader(std::unique_ptr<AbstractReader> reader, size_t size = DEFAULT_BUFFER_SIZE);

        ~BufferedReader() override = default;

        /// @brief Close the stream.
        void close() override;

        /// @brief Mark the present position in the stream.
        /// @param readAheadLimit The maximum limit of characters that can be read before the mark position becomes invalid.
        void mark(size_t readAheadLimit) override;

        /// @brief Tell whether this stream supports the mark() operation.
        /// @return true if the stream supports mark(), false otherwise.
        [[nodiscard]] bool markSupported() const override;

        /// @brief Reset the stream to the most recent mark.
        void reset() override;

        /// @brief Read a single character.
        /// @return The character read, or std::nullopt if the end of the stream has been reached.
        std::optional<char> read() override;

        /// @brief Read characters into an array.
        /// @param cBuf The buffer into which characters are read.
        /// @param off The offset at which to start storing characters.
        /// @param len The maximum number of characters to read.
        /// @return The number of characters read, or -1 if the end of the stream has been reached.
        int read(std::vector<char>& cBuf, size_t off, size_t len) override;

        /// @brief Read a line of text.
        /// @return A String containing the next line of text, or empty string if the end of the stream has been reached.
        [[nodiscard]] std::string readLine();

        /// @brief Tell whether this stream is ready to be read.
        /// @return true if the next read() is guaranteed not to block for input, false otherwise.
        [[nodiscard]] bool ready() const override;

        /// @brief Skip characters.
        /// @param n The number of characters to skip.
        /// @return The number of characters actually skipped.
        size_t skip(size_t n) override;

        /// @brief Checks if this reader has been closed.
        /// @return true if this reader has been closed, false otherwise.
        [[nodiscard]] bool isClosed() const override;

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
        [[nodiscard]] bool fillBuffer();
    };
}
