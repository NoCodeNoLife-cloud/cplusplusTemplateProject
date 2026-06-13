/**
 * @file AbstractReader.hpp
 * @brief Abstract base class for character-stream readers
 * @details Defines the contract for reading character data from various sources.
 *          Implements ICloseable and IReadable with pure-virtual read() targeted
 *          at buffered or unbuffered character input.  Provides default
 *          implementations for skip(), ready(), and markSupported().
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  Derived classes may add thread-safety
 * guarantees.
 *
 * @par Design
 * Follows the Java Reader abstraction pattern: a single abstract read()
 * method that subclasses implement, with template-method defaults for the
 * rest.
 */

#pragma once
#include <algorithm>
#include <optional>
#include <vector>

#include "interface/io/ICloseable.hpp"
#include "interface/io/IReadable.hpp"

namespace common::io::reader
{
    /// @brief Abstract base class for reading character streams.
    /// This class provides a standard interface for reading character data from various sources.
    /// It implements the ICloseable and IReadable interfaces and provides standard implementations
    /// for some common reading operations.
    class AbstractReader : public interface::io::ICloseable, public interface::io::IReadable
    {
    public:
        /// @brief Virtual destructor for proper cleanup of derived classes
        ~AbstractReader() override = default;

        /// @brief Read a single character.
        /// This method reads the next character from the input stream.
        /// @return The character read, or std::nullopt if the end of the stream has been reached
        std::optional<char> read() override;

        /// @brief Read characters into an array.
        /// This method reads up to 'len' characters from the input stream into the buffer 'cBuf'
        /// starting at the specified offset.
        /// @param cBuf Destination buffer to store the characters read
        /// @param off Offset at which to start storing characters
        /// @param len Maximum number of characters to read
        /// @return The number of characters read, or -1 if the end of the stream has been reached
        virtual int read(std::vector<char>& cBuf, size_t off, size_t len) = 0;

        /// @brief Read characters into an array.
        /// This method reads characters from the input stream into the entire buffer 'cBuf'.
        /// @param cBuf Destination buffer to store the characters read
        /// @return The number of characters read, or -1 if the end of the stream has been reached
        virtual int read(std::vector<char>& cBuf);

        /// @brief Tests if this stream supports the mark and reset methods.
        /// @return True if this stream supports the mark and reset methods, false otherwise
        [[nodiscard]] virtual bool markSupported() const;

        /// @brief Marks the present position in the stream.
        /// This method marks the current position in the input stream, allowing a subsequent
        /// call to reset to return to this position.
        /// @param readAheadLimit Limit on the number of characters that may be read while still preserving the mark
        virtual void mark(size_t readAheadLimit) = 0;

        /// @brief Resets the stream to the most recent mark.
        /// This method repositions the stream to the position at the time the mark method was last called.
        virtual void reset() = 0;

        /// @brief Tests if the stream is ready to be read.
        /// This method checks whether the next read operation will block for input.
        /// @return True if the next read() is guaranteed not to block for input, false otherwise
        [[nodiscard]] virtual bool ready() const;

        /// @brief Skips characters.
        /// This method skips over and discards n characters from the input stream.
        /// @param n Number of characters to skip
        /// @return The number of characters actually skipped
        virtual size_t skip(size_t n);

        /// @brief Closes this input stream and releases any system resources associated with the stream.
        void close() override = 0;
    };
}
