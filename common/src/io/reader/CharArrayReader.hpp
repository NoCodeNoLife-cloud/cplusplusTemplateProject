/**
 * @file CharArrayReader.hpp
 * @brief CharArrayReader class declaration
 * @details This header defines the CharArrayReader class that provides functionality for Common library utilities.
 */

#pragma once
#include <optional>
#include <vector>

#include "AbstractReader.hpp"

namespace common::io::reader
{
    /// @brief A reader class that reads from a character array.
    /// This class provides methods to read characters from an internal buffer,
    /// supporting operations like reading single characters, skipping characters,
    /// marking positions in the stream, and resetting to marked positions.
    class CharArrayReader final : public AbstractReader
    {
    public:
        /// @brief Constructs a CharArrayReader with the given buffer
        /// @param buffer The character buffer to read from
        explicit CharArrayReader(const std::vector<char>& buffer);

        /// @brief Constructs a CharArrayReader with a portion of the given buffer
        /// @param buffer The character buffer to read from
        /// @param offset The starting position in the buffer
        /// @param length The number of characters to read from the buffer
        CharArrayReader(const std::vector<char>& buffer, size_t offset, size_t length);

        ~CharArrayReader() override;

        /// @brief Reads a single character.
        /// @return The character read, or std::nullopt if the end of the stream has been reached
        [[nodiscard]] std::optional<char> read() override;

        /// @brief Reads characters into an array.
        /// @param b The buffer into which the read characters are stored
        /// @param off The offset at which to store the characters
        /// @param len The maximum number of characters to read
        /// @return The number of characters read, or -1 if the end of the stream has been reached
        [[nodiscard]] int read(std::vector<char>& b, size_t off, size_t len) override;

        /// @brief Skips characters.
        /// @param n The number of characters to skip
        /// @return The number of characters actually skipped
        [[nodiscard]] size_t skip(size_t n) override;

        /// @brief Tests if this reader can be read without blocking.
        /// @return True if this reader can be read without blocking, false otherwise
        [[nodiscard]] bool ready() const  override;

        /// @brief Tests if this reader supports the mark() operation.
        /// @return True if this reader supports the mark() operation, false otherwise
        [[nodiscard]] bool markSupported() const  override;

        /// @brief Marks the present position in the stream.
        /// @param readAheadLimit Limit on the number of characters that may be read while still preserving the mark.
        void mark(size_t readAheadLimit) override;

        /// @brief Resets the stream to the most recent mark.
        void reset() override;

        /// @brief Closes the stream and releases any system resources associated with it.
        void close() override;

        /// @brief Checks if this reader has been closed.
        /// @return true if this reader has been closed, false otherwise.
        [[nodiscard]] bool isClosed() const  override;

    private:
        std::vector<char> buf_;
        size_t pos_{0};
        size_t marked_pos_{0};
        size_t count_{0};
        bool closed_{false};

        /// @brief Validates the constructor parameters
        /// @param buffer_size Size of the buffer
        /// @param offset Starting position in the buffer
        /// @param length Number of characters to read
        /// @throws std::invalid_argument if parameters are invalid
        static void validateConstructorParams(size_t buffer_size, size_t offset, size_t length);

        /// @brief Validates the target buffer parameters
        /// @param target_buffer_size Size of the target buffer
        /// @param offset Offset in target buffer
        /// @param length Number of characters to read
        /// @throws std::out_of_range if parameters are invalid
        static void validateTargetBufferParams(size_t target_buffer_size, size_t offset, size_t length);
    };
}
