/**
 * @file BufferedOutputStream.hpp
 * @brief BufferedOutputStream class declaration
 * @details This header defines the BufferedOutputStream class that provides functionality for Common library utilities.
 */

#pragma once
#include <cstring>
#include <memory>
#include <vector>

#include "FilterOutputStream.hpp"

namespace common::filesystem {
/// @brief A buffered output stream that writes data to an underlying output stream.
/// This class buffers the data written to it and flushes it to the underlying stream
/// when the buffer is full or when explicitly requested.
class BufferedOutputStream final : public FilterOutputStream {
public:
    /// @brief Constructs a buffered output stream with default buffer size.
    /// @param out The underlying output stream.
    /// @throws std::invalid_argument if out is null.
    explicit BufferedOutputStream(std::unique_ptr<AbstractOutputStream> out);

    /// @brief Constructs a buffered output stream with specified buffer size.
    /// @param out The underlying output stream.
    /// @param size The buffer size in bytes.
    /// @throws std::invalid_argument if out is null or size is 0.
    BufferedOutputStream(std::unique_ptr<AbstractOutputStream> out, size_t size);

    ~BufferedOutputStream() override;

    /// @brief Writes a single byte to the buffer.
    /// @param b The byte to write.
    auto write(std::byte b) -> void override;

    /// @brief Writes a sequence of bytes to the buffer.
    /// @param data The data to write.
    /// @param offset The start offset in the data.
    /// @param len The number of bytes to write.
    /// @throws std::out_of_range if offset + len exceeds data size.
    auto write(const std::vector<std::byte>& data, size_t offset, size_t len) -> void override;

    /// @brief Writes a sequence of bytes to the buffer.
    /// @param buffer The buffer containing bytes to write.
    /// @param length The number of bytes to write.
    auto write(const std::byte* buffer, size_t length) -> void override;

    /// @brief Flushes the buffer by writing all buffered bytes to the underlying output stream.
    auto flush() -> void override;

    /// @brief Closes the stream by flushing the buffer and closing the underlying output stream.
    auto close() -> void override;

    /// @brief Returns the current size of the buffer.
    /// @return The buffer size in bytes.
    [[nodiscard]] auto getBufferSize() const noexcept -> size_t;

    /// @brief Returns the number of bytes currently in the buffer.
    /// @return The number of bytes buffered.
    [[nodiscard]] auto getBufferedDataSize() const noexcept -> size_t;

    /// @brief Checks if the stream is closed.
    /// @return True if the stream is closed, false otherwise.
    [[nodiscard]] auto isClosed() const noexcept -> bool override;

protected:
    /// @brief Flushes the internal buffer to the underlying output stream.
    auto flushBuffer() -> void;

    static constexpr size_t DEFAULT_BUFFER_SIZE = 8192;
    size_t bufferSize_{};
    std::vector<std::byte> buffer_{};
    size_t buffer_position_{};
};
}