#pragma once
#include <algorithm>
#include <cstddef>
#include <vector>

#include "AbstractInputStream.hpp"

namespace common::filesystem {
/// @brief A ByteArrayInputStream contains an internal buffer that contains bytes that may be read from the stream.
/// ByteArrayInputStream uses a std::vector<std::byte> as the internal buffer.
class ByteArrayInputStream final : public AbstractInputStream {
public:
    explicit ByteArrayInputStream(const std::vector<std::byte>& buf);

    /// @brief Reads the next byte of data from the input stream.
    /// @return the next byte of data, or -1 if the end of the stream is reached.
    [[nodiscard]] auto read() -> std::byte override;

    /// @brief Reads up to len bytes of data from the input stream into an array of bytes.
    /// @param cBuf the buffer into which the data is read.
    /// @param off the start offset in the destination array cBuf.
    /// @param len the maximum number of bytes to read.
    /// @return the total number of bytes read into the buffer, or 0 if there is no more data because the end of the stream has been reached.
    [[nodiscard]] auto read(std::vector<std::byte>& cBuf, size_t off, size_t len) -> size_t override;

    /// @brief Skips over and discards n bytes of data from this input stream.
    /// @param n the number of bytes to be skipped.
    /// @return the total number of bytes skipped.
    [[nodiscard]] auto skip(size_t n) -> size_t override;

    /// @brief Returns the number of bytes that can be read (or skipped over) from this input stream without blocking by
    /// the next caller of a method for this input stream.
    /// @return the number of bytes that can be read (or skipped over) from this input stream without blocking.
    [[nodiscard]] auto available() -> size_t override;

    /// @brief Repositions this stream to the position at the time the mark method was last called on this input stream.
    auto reset() -> void override;

    /// @brief Marks the current position in this input stream.
    /// @param readLimit the maximum limit of bytes that can be read ahead.
    auto mark(int32_t readLimit) -> void override;

    /// @brief Tests if this input stream supports the mark and reset methods.
    /// @return true if this stream instance supports the mark and reset methods; false otherwise.
    [[nodiscard]] auto markSupported() const noexcept -> bool override;

    /// @brief Closes this input stream and releases any system resources associated with the stream.
    auto close() noexcept -> void override;

    /// @brief Checks if this input stream has been closed.
    /// @return true if this input stream has been closed, false otherwise.
    [[nodiscard]] auto isClosed() const noexcept -> bool override;

protected:
    const std::vector<std::byte> buffer_;
    size_t pos_{0};
    size_t mark_position_{0};
    bool closed_{false};
};
}