/**
 * @file FilterOutputStream.hpp
 * @brief Decorator base class for byte-output streams
 * @details Implements the decorator pattern for AbstractOutputStream.
 *          Stores a shared_ptr to an underlying stream and delegates all
 *          write/flush/close calls.  Provides ensureStream/ensureOpen guards.
 *          Subclasses override specific methods to add buffering, encryption,
 *          or compression.  Analogous to java.io.FilterOutputStream.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 */

#pragma once
#include <memory>
#include <vector>

#include <cppforge/io/writer/AbstractOutputStream.hpp>

namespace cppforge::io::writer
{
    /// @brief A filter output stream that wraps another output stream.
    /// @details This class provides a base for output stream filters that modify or enhance
    /// the behavior of the underlying output stream.
    class FilterOutputStream : public AbstractOutputStream
    {
    public:
        /// @brief Constructs a FilterOutputStream that wraps the specified output stream.
        /// @param outputStream The underlying output stream to wrap.
        /// @throws std::invalid_argument If outputStream is null.
        explicit FilterOutputStream(std::shared_ptr<AbstractOutputStream> outputStream);

        /// @brief Destructor.
        ~FilterOutputStream() override = default;

        /// @brief Writes a single byte to the output stream.
        /// @param b The byte to write.
        /// @throws std::runtime_error If the output stream is not available or closed.
        void write(std::byte b) override;

        /// @brief Writes all bytes from the given buffer to the output stream.
        /// @param buffer The buffer containing bytes to write.
        /// @throws std::runtime_error If the output stream is not available or closed.
        void write(const std::vector<std::byte>& buffer) override;

        /// @brief Writes a specified number of bytes from the buffer starting at the given offset.
        /// @param buffer The buffer containing bytes to write.
        /// @param offset The starting position in the buffer.
        /// @param len The number of bytes to write.
        /// @throws std::runtime_error If the output stream is not available or closed.
        /// @throws std::out_of_range If offset + len exceeds buffer size.
        void write(const std::vector<std::byte>& buffer, size_t offset, size_t len) override;

        void write(const std::byte* buffer, size_t length) override;

        /// @brief Flushes the output stream, ensuring all buffered data is written.
        /// @throws std::runtime_error If the output stream is not available or closed.
        void flush() override;

        /// @brief Closes the output stream and releases any associated resources.
        /// @throws std::runtime_error If the output stream is not available.
        void close() override;

        /// @brief Checks if the stream is closed.
        /// @return true if the stream is closed, false otherwise.
        [[nodiscard]] bool isClosed() const override;

    protected:
        std::shared_ptr<AbstractOutputStream> output_stream_;
        bool closed_{false};

    private:
        /// @brief Ensures the underlying output stream is available.
        /// @throws std::runtime_error If the output stream is not available.
        void ensureStream() const;

        /// @brief Ensures the stream is not closed.
        /// @throws std::runtime_error If the stream is closed.
        void ensureOpen() const;
    };
}
