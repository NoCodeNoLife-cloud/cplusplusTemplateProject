/**
 * @file InputStreamReader.hpp
 * @brief InputStreamReader class declaration
 * @details This header defines the InputStreamReader class that provides functionality for Common library utilities.
 */

#pragma once
#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "AbstractReader.hpp"

namespace common::io::reader
{
    /// @brief InputStreamReader is a class for reading characters from a byte stream.
    /// This class inherits from AbstractReader and provides functionality to read characters
    /// from an input stream with specified charset encoding.
    class InputStreamReader final : public AbstractReader
    {
    public:
        explicit InputStreamReader(std::shared_ptr<AbstractReader> input);

        InputStreamReader(std::shared_ptr<AbstractReader> input, const std::string& charsetName);

        ~InputStreamReader() override = default;

        /// @brief Read a single character.
        /// @return The character read, or std::nullopt if the end of the stream has been reached
        std::optional<char> read() override;

        /// @brief Read characters into an array.
        /// @param cBuf Destination buffer
        /// @param off Offset at which to start storing characters
        /// @param len Maximum number of characters to read
        /// @return The number of characters read, or -1 if the end of the stream has been reached
        int read(std::vector<char>& cBuf, size_t off, size_t len) override;

        /// @brief Tests if this input stream is ready to be read.
        /// @return true if the next read() is guaranteed not to block for input, false otherwise.
        [[nodiscard]] bool ready() const override;

        /// @brief Close the stream.
        void close() override;

        /// @brief Tests if this stream supports the mark and reset methods.
        /// @return true if this stream type supports the mark and reset methods; false otherwise.
        [[nodiscard]] bool markSupported() const override;

        /// @brief Marks the current position in the stream.
        /// @param readAheadLimit the maximum limit of bytes that can be read before the mark position becomes invalid.
        void mark(size_t readAheadLimit) override;

        /// @brief Resets the stream to the most recent mark.
        void reset() override;

        /// @brief Checks if this reader has been closed.
        /// @return true if this reader has been closed, false otherwise.
        [[nodiscard]] bool isClosed() const override;

    private:
        std::shared_ptr<AbstractReader> reader_;
        bool closed_{false};
    };
}
