/**
 * @file PushbackReader.hpp
 * @brief PushbackReader class declaration
 * @details This header defines the PushbackReader class that provides functionality for Common library utilities.
 */

#pragma once
#include <algorithm>
#include <vector>

#include "FilterReader.hpp"

namespace common::filesystem {
/// @brief Provides a buffering wrapper around another reader, allowing characters to be "pushed back" into the stream.
/// @details The PushbackReader class allows characters to be pushed back into the stream after they have been read.
/// This is useful for situations where a program needs to look ahead in the input stream and then return to the
/// original position. The class maintains an internal buffer to store the pushed-back characters. When reading, it
/// first checks this buffer before reading from the underlying reader.
class PushbackReader final : public FilterReader {
public:
    explicit PushbackReader(std::shared_ptr<AbstractReader> reader);

    PushbackReader(std::shared_ptr<AbstractReader> reader, size_t size);

    /// @brief Closes this pushback reader and releases any system resources associated with it.
    /// @details Once the stream has been closed, further read(), unread(), ready(), mark(),
    /// reset(), or skip() invocations will throw an IOException.
    /// Closing a previously closed stream has no effect.
    auto close() noexcept -> void override;

    /// @brief Marks the current position in this stream.
    /// @details A subsequent call to the reset() method repositions this stream at the last marked position so that
    /// subsequent reads re-read the same bytes. The readAheadLimit argument tells this stream to allow that many bytes
    /// to be read before the mark position gets invalidated.
    /// @param readAheadLimit the maximum limit of bytes that can be read before the mark position becomes invalid
    auto mark(size_t readAheadLimit) -> void override;

    /// @brief Tests if this stream supports the mark() and reset() methods.
    /// @details Whether mark() and reset() are supported is an invariant property of a particular stream instance.
    /// The markSupported() method of FilterReader returns true.
    /// @return true if this stream supports the mark() and reset() methods; false otherwise
    [[nodiscard]] auto markSupported() const -> bool override;

    /// @brief Reads a single character.
    /// @details This method will block until a character is available, an I/O error occurs, or the end of the stream is
    /// reached. If the pushback buffer is not empty, a character from the pushback buffer is returned. Otherwise, a
    /// character from the underlying input stream is returned.
    /// @return The character read, or -1 if the end of the stream has been reached
    [[nodiscard]] auto read() -> int override;

    /// @brief Reads up to len characters of data from this stream into an array of characters.
    /// @details This method will block until some input is available, an I/O error occurs, or the end of the stream is
    /// reached. If the pushback buffer is not empty, characters from the pushback buffer are returned. Otherwise,
    /// characters from the underlying input stream are returned.
    /// @param cBuf the buffer into which the data is read
    /// @param off the start offset in the destination array cBuf
    /// @param len the maximum number of characters read
    /// @return the total number of characters read into the buffer, or -1 if there is no more data because the end of
    /// the stream has been reached
    [[nodiscard]] auto read(std::vector<char>& cBuf, size_t off, size_t len) -> int override;

    /// @brief Tells whether this stream is ready to be read.
    /// @details A stream is ready to be read if there are characters available in the pushback buffer,
    /// or if the underlying input stream is ready to be read.
    /// @return true if the next read() is guaranteed not to block for input, false otherwise.
    /// Note that returning false does not guarantee that the next read() will block.
    [[nodiscard]] auto ready() const -> bool override;

    /// @brief Repositions this stream to the position at the time the mark() method was last called on this stream.
    /// @details This method will block until a character is available, an I/O error occurs, or the end of the stream is
    /// reached. If the pushback buffer is not empty, a character from the pushback buffer is returned. Otherwise, a
    /// character from the underlying input stream is returned.
    auto reset() -> void override;

    /// @brief Skips n characters from the input stream.
    /// @details This method will block until n characters are available, an I/O error occurs, or the end of the stream
    /// is reached. Characters in the pushback buffer are skipped first, then characters from the underlying input
    /// stream are skipped.
    /// @param n the number of characters to skip
    /// @return the actual number of characters skipped
    [[nodiscard]] auto skip(size_t n) -> size_t override;

    /// @brief Pushes back all characters of the given array into the pushback buffer.
    /// @details This method pushes back all characters of the given array into the pushback buffer.
    /// The characters are pushed back in reverse order, so that the next read operation will read
    /// the characters in the same order as they appear in the array.
    /// @param cbuf the array of characters to push back
    auto unread(const std::vector<char>& cbuf) noexcept -> void;

    /// @brief Pushes back a portion of the given character array into the pushback buffer.
    /// @details This method pushes back len characters from the given array starting at offset off
    /// into the pushback buffer. The characters are pushed back in reverse order, so that the next
    /// read operation will read the characters in the same order as they appear in the array.
    /// @param cBuf the array of characters to push back
    /// @param off the start offset in the array
    /// @param len the number of characters to push back
    auto unread(const std::vector<char>& cBuf, size_t off, size_t len) -> void;

    /// @brief Pushes back a single character into the pushback buffer.
    /// @details This method pushes back a single character into the pushback buffer.
    /// The character is pushed back so that the next read operation will read this character.
    /// @param c the character to push back
    auto unread(int32_t c) -> void;

    /// @brief Checks if this reader has been closed.
    /// @return true if this reader has been closed, false otherwise.
    [[nodiscard]] auto isClosed() const -> bool override;

private:
    /// @brief Validates that the stream is open for operations
    /// @throws std::runtime_error if the stream is closed
    auto validateOpen() const -> void;

    /// @brief Validates that the pushback stream is not closed
    /// @throws std::runtime_error if the stream is closed
    auto validateNotClosed() const -> void;

    static constexpr size_t DEFAULT_BUFFER_SIZE = 1024;
    std::vector<char> buffer_;
    size_t buffer_pos_{DEFAULT_BUFFER_SIZE};
    bool closed_{false};
};
}