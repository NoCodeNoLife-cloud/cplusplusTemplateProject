/**
 * @file BufferedOutputStream.hpp
 * @brief Buffered byte-output stream with 8 KB default buffer
 * @description Wraps an AbstractOutputStream with an 8 KB buffer to batch
 *          small writes into larger chunks, reducing I/O calls.  The buffer
 *          is flushed automatically when full, and manually via flush() or
 *          close().  Uses memcpy for efficient buffer management.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Memory
 * Allocates a fixed-size buffer of @p size bytes on construction (default 8192).
 */

#pragma once
#include <cstring>
#include <memory>
#include <vector>

#include "FilterOutputStream.hpp"

namespace common::io::writer
{
    /// @brief A buffered output stream that writes data to an underlying output stream.
    /// This class buffers the data written to it and flushes it to the underlying stream
    /// when the buffer is full or when explicitly requested.
    class BufferedOutputStream final : public FilterOutputStream
    {
    public:
        using FilterOutputStream::write;

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
        void write(std::byte b) override;

        /// @brief Writes a sequence of bytes to the buffer.
        /// @param data The data to write.
        /// @param offset The start offset in the data.
        /// @param len The number of bytes to write.
        /// @throws std::out_of_range if offset + len exceeds data size.
        void write(const std::vector<std::byte>& data, size_t offset, size_t len) override;

        /// @brief Writes a sequence of bytes to the buffer.
        /// @param buffer The buffer containing bytes to write.
        /// @param length The number of bytes to write.
        void write(const std::byte* buffer, size_t length) override;

        /// @brief Flushes the buffer by writing all buffered bytes to the underlying output stream.
        void flush() override;

        /// @brief Closes the stream by flushing the buffer and closing the underlying output stream.
        void close() override;

        /// @brief Returns the current size of the buffer.
        /// @return The buffer size in bytes.
        [[nodiscard]] size_t getBufferSize() const;

        /// @brief Returns the number of bytes currently in the buffer.
        /// @return The number of bytes buffered.
        [[nodiscard]] size_t getBufferedDataSize() const;

        /// @brief Checks if the stream is closed.
        /// @return True if the stream is closed, false otherwise.
        [[nodiscard]] bool isClosed() const override;

    protected:
        /// @brief Flushes the internal buffer to the underlying output stream.
        void flushBuffer();

        static constexpr size_t DEFAULT_BUFFER_SIZE = 8192;
        size_t bufferSize_{};
        std::vector<std::byte> buffer_{};
        size_t buffer_position_{};
        bool closed_{false};
    };
}
