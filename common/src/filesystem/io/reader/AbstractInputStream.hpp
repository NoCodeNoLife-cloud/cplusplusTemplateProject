/**
 * @file AbstractInputStream.hpp
 * @brief AbstractInputStream class declaration
 * @details This header defines the AbstractInputStream class that provides functionality for Common library utilities.
 */

#pragma once
#include <algorithm>
#include <cstddef>
#include <vector>
#include <cstdint>

#include "src/interface/ICloseable.hpp"

namespace common::filesystem
{
    /// @brief Abstract base class for all input streams.
    /// Provides standard input stream functionality that can be extended by concrete implementations.
    class AbstractInputStream : public interfaces::ICloseable
    {
    public:
        ~AbstractInputStream() override = default;

        /// @brief Returns the number of bytes that can be read (or skipped over) from this input stream without blocking.
        /// @return The number of bytes that can be read without blocking.
        [[nodiscard]] virtual size_t available() = 0;

        /// @brief Marks the current position in this input stream.
        /// @param readLimit The maximum limit of bytes that can be read before the mark position becomes invalid.
        virtual void mark(std::int32_t readLimit);

        /// @brief Tells whether this stream supports the mark() operation.
        /// @return true if this stream supports the mark operation, false otherwise.
        [[nodiscard]] virtual bool markSupported() const;

        /// @brief Reads the next byte of data from the input stream.
        /// @return The next byte of data, or -1 if the end of the stream is reached.
        virtual std::byte read() = 0;

        /// @brief Reads up to len bytes of data from the input stream into an array of bytes.
        /// @param buffer The buffer into which the data is read.
        /// @return The total number of bytes read into the buffer, or 0 if there is no more data.
        virtual size_t read(std::vector<std::byte>& buffer);

        /// @brief Reads up to len bytes of data from the input stream into an array of bytes.
        /// @param buffer The buffer into which the data is read.
        /// @param offset The start offset in the destination array buffer.
        /// @param len The maximum number of bytes to read.
        /// @return The total number of bytes read into the buffer, or 0 if there is no more data.
        virtual size_t read(std::vector<std::byte>& buffer, std::size_t offset, std::size_t len);

        /// @brief Repositions this stream to the position at the time the mark() method was last called.
        virtual void reset();

        /// @brief Skips over and discards n bytes of data from this input stream.
        /// @param n The number of bytes to skip.
        /// @return The actual number of bytes skipped.
        virtual size_t skip(std::size_t n);

        /// @brief Closes this input stream and releases any system resources associated with the stream.
        void close() override = 0;
    };
}
