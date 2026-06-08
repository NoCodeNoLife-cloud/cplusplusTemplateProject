/**
 * @file StringReader.hpp
 * @brief StringReader class declaration
 * @details This header defines the StringReader class that provides functionality for Common library utilities.
 */

#pragma once
#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "AbstractReader.hpp"

namespace common::io::reader
{
    /// @brief A StringReader class that reads data from a string.
    /// @details This class implements the AbstractReader interface to provide functionality
    ///          for reading characters from a string. It supports marking and resetting
    ///          to a previous position in the string.
    class StringReader final : public AbstractReader
    {
    public:
        explicit StringReader(std::string s);

        ~StringReader() override = default;

        /// @brief Closes the StringReader and releases any associated resources.
        void close()  override;

        /// @brief Marks the current position in the stream.
        /// @param readAheadLimit the maximum number of characters that can be read from the stream before the mark position
        /// becomes invalid.
        void mark(size_t readAheadLimit) override;

        /// @brief Tests if this input stream supports the mark and reset methods.
        /// @return true if this stream type supports the mark and reset methods; false otherwise.
        [[nodiscard]] bool markSupported() const  override;

        /// @brief Reads a single character from the string.
        /// @return The character read, or std::nullopt if the end of the string has been reached.
        [[nodiscard]] std::optional<char> read() override;

        /// @brief Reads up to len characters from the string into the buffer cBuf starting at offset off.
        /// @param cBuf The buffer into which the data is read.
        /// @param off The start offset in the buffer at which the data is written.
        /// @param len The maximum number of characters to read.
        /// @return The total number of characters read into the buffer, or -1 if there is no more data because the end of
        /// the string has been reached.
        [[nodiscard]] int read(std::vector<char>& cBuf, size_t off, size_t len) override;

        /// @brief Tests if this input stream is ready to be read.
        /// @return true if the next read() is guaranteed not to block for input, false otherwise.
        [[nodiscard]] bool ready() const  override;

        /// @brief Resets the stream to the most recent mark position.
        void reset() override;

        /// @brief Skips over and discards n characters from the input stream.
        /// @param ns The number of characters to skip.
        /// @return The actual number of characters skipped.
        [[nodiscard]] size_t skip(size_t ns)  override;

        /// @brief Checks if this reader has been closed.
        /// @return true if this reader has been closed, false otherwise.
        [[nodiscard]] bool isClosed() const  override;

    private:
        std::string source_;
        size_t position_;
        size_t mark_position_;
        bool mark_set_;
        bool closed_;
    };
}
