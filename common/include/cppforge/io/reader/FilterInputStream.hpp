/**
 * @file FilterInputStream.hpp
 * @brief Decorator base class for byte-input streams
 * @details Implements the decorator (wrapper) pattern for AbstractInputStream.
 *          Stores a shared_ptr to an underlying stream and delegates all
 *          read/skip/available/mark/reset/close calls to it.  Subclasses
 *          override specific methods to add buffering, filtering, or other
 *          transformations.  Analogous to java.io.FilterInputStream.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  The underlying stream's thread-safety
 * depends on its implementation.
 */

#pragma once
#include <memory>

#include <cppforge/io/reader/AbstractInputStream.hpp>

namespace cppforge::io::reader
{
    /// @brief A filter stream is a stream that filters another input stream.
    /// This class is the basis for all input streams that filter another input stream.
    class FilterInputStream : public AbstractInputStream
    {
    public:
        explicit FilterInputStream(std::unique_ptr<AbstractInputStream> inputStream) ;

        ~FilterInputStream() override = default;

        /// @brief Returns the number of bytes that can be read (or skipped over) from this input stream without blocking.
        /// @return the number of bytes that can be read (or skipped over) from this input stream without blocking.
        [[nodiscard]] size_t available() override;

        /// @brief Marks the current position in this input stream.
        /// @param readLimit the maximum number of bytes that can be read before the mark position becomes invalid.
        void mark(int32_t readLimit) override;

        /// @brief Tests if this input stream supports the mark and reset methods.
        /// @return true if this input stream supports the mark and reset methods; false otherwise.
        [[nodiscard]] bool markSupported() const  override;

        /// @brief Reads the next byte of data from this input stream.
        /// @return the next byte of data, or -1 if the end of the stream is reached.
        [[nodiscard]] std::byte read() override;

        /// @brief Reads up to len bytes of data from this input stream into an array of bytes.
        /// @param buffer the buffer into which the data is read.
        /// @return the total number of bytes read into the buffer, or 0 if there is no more data because the end of the
        /// stream has been reached.
        [[nodiscard]] size_t read(std::vector<std::byte>& buffer) override;

        /// @brief Reads up to len bytes of data from this input stream into an array of bytes.
        /// @param buffer the buffer into which the data is read.
        /// @param offset the start offset in the destination array buffer.
        /// @param len the maximum number of bytes to read.
        /// @return the total number of bytes read into the buffer, or 0 if there is no more data because the end of the
        /// stream has been reached.
        [[nodiscard]] size_t read(std::vector<std::byte>& buffer, size_t offset, size_t len) override;

        /// @brief Repositions this stream to the position at the time the mark method was last called on this input stream.
        void reset() override;

        /// @brief Skips over and discards n bytes of data from this input stream.
        /// @param n the number of bytes to be skipped.
        /// @return the number of bytes skipped.
        [[nodiscard]] int64_t skip(int64_t n) override;

        /// @brief Closes this input stream and releases any system resources associated with the stream.
        void close() override;

        /// @brief Checks if this input stream has been closed.
        /// @return true if this input stream has been closed, false otherwise.
        [[nodiscard]] bool isClosed() const  override;

    protected:
        std::unique_ptr<AbstractInputStream> input_stream_;

    private:
        /// @brief Validates that the input stream is available, throws exception if not
        /// @throws std::runtime_error If input stream is null
        void validateInputStream() const;
    };
}
