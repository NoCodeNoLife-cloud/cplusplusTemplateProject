#pragma once
#include <algorithm>
#include <cstddef>
#include <vector>
#include <cstdint>

#include "src/interface/ICloseable.hpp"

namespace common::filesystem {
    /// @brief Abstract base class for all input streams.
    /// Provides standard input stream functionality that can be extended by concrete implementations.
    class AbstractInputStream : public interfaces::ICloseable {
    public:
        ~AbstractInputStream() override = default;

        /// @brief Returns the number of bytes that can be read (or skipped over) from this input stream without blocking.
        /// @return The number of bytes that can be read without blocking.
        [[nodiscard]] virtual auto available() -> size_t = 0;

        /// @brief Marks the current position in this input stream.
        /// @param readLimit The maximum limit of bytes that can be read before the mark position becomes invalid.
        virtual auto mark(std::int32_t readLimit) -> void;

        /// @brief Tells whether this stream supports the mark() operation.
        /// @return true if this stream supports the mark operation, false otherwise.
        [[nodiscard]] virtual auto markSupported() const -> bool;

        /// @brief Reads the next byte of data from the input stream.
        /// @return The next byte of data, or -1 if the end of the stream is reached.
        virtual auto read() -> std::byte = 0;

        /// @brief Reads up to len bytes of data from the input stream into an array of bytes.
        /// @param buffer The buffer into which the data is read.
        /// @return The total number of bytes read into the buffer, or 0 if there is no more data.
        virtual auto read(std::vector<std::byte> &buffer) -> size_t;

        /// @brief Reads up to len bytes of data from the input stream into an array of bytes.
        /// @param buffer The buffer into which the data is read.
        /// @param offset The start offset in the destination array buffer.
        /// @param len The maximum number of bytes to read.
        /// @return The total number of bytes read into the buffer, or 0 if there is no more data.
        virtual auto read(std::vector<std::byte> &buffer, std::size_t offset, std::size_t len) -> size_t;

        /// @brief Repositions this stream to the position at the time the mark() method was last called.
        virtual auto reset() -> void;

        /// @brief Skips over and discards n bytes of data from this input stream.
        /// @param n The number of bytes to skip.
        /// @return The actual number of bytes skipped.
        virtual auto skip(std::size_t n) -> size_t;

        /// @brief Closes this input stream and releases any system resources associated with the stream.
        auto close() -> void override = 0;
    };
}
