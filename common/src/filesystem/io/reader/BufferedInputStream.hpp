/**
 * @file BufferedInputStream.hpp
 * @brief BufferedInputStream class declaration
 * @details This header defines the BufferedInputStream class that provides functionality for Common library utilities.
 */

#pragma once
#include <memory>
#include <cstddef>
#include <vector>

#include "FilterInputStream.hpp"

namespace common::filesystem
{
    /// @brief BufferedInputStream is a wrapper around another input stream that provides buffering functionality.
    /// It reads data from the underlying stream in chunks and stores it in an internal buffer to improve performance.
    /// @details This class implements the decorator pattern to add buffering capability to any AbstractInputStream.
    /// The buffer size can be customized during construction, with a default size of 8192 bytes.
    class BufferedInputStream final : public FilterInputStream
    {
    public:
        explicit BufferedInputStream(std::unique_ptr<AbstractInputStream> in) noexcept;

        BufferedInputStream(std::unique_ptr<AbstractInputStream> in, size_t size);

        /// @brief Returns the number of bytes that can be read (or skipped over) from this input stream without blocking.
        /// @return the number of bytes that can be read (or skipped over) from this input stream without blocking.
        [[nodiscard]] size_t available() const noexcept;

        /// @brief Closes this input stream and releases any system resources associated with the stream.
        void close() noexcept override;

        /// @brief Marks the current position in this input stream.
        /// @param readLimit the maximum limit of bytes that can be read before the mark position becomes invalid.
        void mark(int32_t readLimit) override;

        /// @brief Tests if this input stream supports the mark and reset methods.
        /// @return true if this stream instance supports the mark and reset methods; false otherwise.
        [[nodiscard]] bool markSupported() const noexcept override;

        /// @brief Reads the next byte of data from this input stream.
        /// @return the next byte of data, or -1 if the end of the stream is reached.
        [[nodiscard]] std::byte read() override;

        /// @brief Reads up to len bytes of data from this input stream into an array of bytes.
        /// @param buffer the buffer into which the data is read.
        /// @param offset the start offset in the destination array buffer.
        /// @param len the maximum number of bytes to read.
        /// @return the total number of bytes read into the buffer, or 0 if there is no more data because the end of the stream has been reached.
        size_t read(std::vector<std::byte>& buffer, size_t offset, size_t len) override;

        /// @brief Repositions this stream to the position at the time the mark method was last called on this input stream.
        void reset() override;

        /// @brief Skips over and discards n bytes of data from this input stream.
        /// @param n the number of bytes to skip.
        /// @return the actual number of bytes skipped.
        size_t skip(size_t n) override;

        /// @brief Checks if this input stream has been closed.
        /// @return true if this input stream has been closed, false otherwise.
        [[nodiscard]] bool isClosed() const noexcept override;

    protected:
        static constexpr size_t DEFAULT_BUFFER_SIZE = 8192;
        std::vector<std::byte> buf_;
        size_t count_{0};
        size_t mark_limit_{0};
        size_t mark_pos_{0};
        size_t pos_{0};

        void fillBuffer();

    private:
        /// @brief Helper method to process data with available buffer space.
        /// @tparam Operation A callable that takes the number of available bytes and returns processed bytes count
        /// @param op The operation to perform on the available buffer space
        /// @return Number of bytes processed by the operation
        template <typename Operation>
        size_t processWithBuffer(Operation&& op);
    };
}