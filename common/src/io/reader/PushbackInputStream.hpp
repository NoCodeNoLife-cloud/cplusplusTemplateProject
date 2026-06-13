/**
 * @file PushbackInputStream.hpp
 * @brief Byte-input stream with single-byte / array unread capability
 * @description Allows bytes to be "pushed back" (unread) into the stream,
 *          enabling lookahead parsing.  Supports unread of a single byte
 *          or a full array.  When the pushback buffer overflows, throws
 *          std::ios_base::failure.  Analogous to java.io.PushbackInputStream.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Memory
 * Allocates an internal pushback buffer of configurable size (default matches
 * the underlying stream's buffer size).
 */

#pragma once
#include <cstddef>

#include "FilterInputStream.hpp"

namespace common::io::reader
{
    /// @brief A pushback input stream allows bytes to be pushed back into the stream.
    /// This class wraps another input stream and provides the ability to "unread" bytes,
    /// making them available to be read again. This is useful for parsers that need to
    /// look ahead in the input stream.
    class PushbackInputStream final : public FilterInputStream
    {
    public:
        /// @brief Constructs a PushbackInputStream with the given input stream and buffer size
        /// @param inputStream The underlying input stream to wrap
        /// @param bufferSize Size of the pushback buffer
        explicit PushbackInputStream(std::unique_ptr<AbstractInputStream> inputStream, size_t bufferSize);

        ~PushbackInputStream() override = default;

        /// @brief Returns the number of bytes that can be read from this input stream without blocking.
        /// @return the number of bytes that can be read from this input stream without blocking.
        [[nodiscard]] size_t available() override;

        /// @brief Reads the next byte of data from the input stream.
        /// @return the next byte of data, or -1 if the end of the stream is reached.
        [[nodiscard]] std::byte read() override;

        /// @brief Reads up to `buffer.size()` bytes of data from the input stream into the buffer.
        /// @param buffer the buffer into which the data is read.
        /// @return the total number of bytes read into the buffer, or 0 if there is no more data because the end of the
        /// stream has been reached.
        [[nodiscard]] size_t read(std::vector<std::byte>& buffer) override;

        /// @brief Reads up to `len` bytes of data from the input stream into the buffer starting at `offset`.
        /// @param buffer the buffer into which the data is read.
        /// @param offset the start offset in the buffer at which the data is written.
        /// @param len the maximum number of bytes to read.
        /// @return the total number of bytes read into the buffer, or 0 if there is no more data because the end of the
        /// stream has been reached.
        [[nodiscard]] size_t read(std::vector<std::byte>& buffer, size_t offset, size_t len) override;

        /// @brief Pushes back the entire buffer into the stream.
        /// @param buffer the buffer to push back.
        void unread(const std::vector<std::byte>& buffer);

        /// @brief Pushes back a portion of the buffer into the stream.
        /// @param buffer the buffer containing data to push back.
        /// @param offset the start offset in the buffer from which data is pushed back.
        /// @param len the number of bytes to push back.
        void unread(const std::vector<std::byte>& buffer, size_t offset, size_t len);

        /// @brief Pushes back a single byte into the stream.
        /// @param b the byte to push back.
        void unread(std::byte b);

        /// @brief Checks if this input stream has been closed.
        /// @return true if this input stream has been closed, false otherwise.
        [[nodiscard]] bool isClosed() const  override;

    private:
        std::vector<std::byte> pushback_buffer_;
        size_t buffer_pos_{0};
    };
}
