/**
 * @file OutputStreamWriter.hpp
 * @brief OutputStreamWriter class declaration
 * @details This header defines the OutputStreamWriter class that provides functionality for Common library utilities.
 */

#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "AbstractWriter.hpp"

namespace common::io::writer
{
    /// @brief A writer that converts character streams to byte streams using a specified charset.
    /// @details OutputStreamWriter is a bridge from character streams to byte streams: Characters written to it
    /// are encoded into bytes using the specified charset. The charset can be specified by name or
    /// defaults to the system's default charset.
    class OutputStreamWriter final : public AbstractWriter
    {
    public:
        /// @brief Constructs an OutputStreamWriter that uses the specified charset.
        /// @param outputStream The underlying output stream.
        /// @param charsetName The name of the charset to be used.
        /// @throws std::invalid_argument If the charset is not supported or outputStream is null.
        OutputStreamWriter(std::unique_ptr<AbstractWriter> outputStream, const std::string& charsetName);

        /// @brief Constructs an OutputStreamWriter that uses the default charset.
        /// @param outputStream The underlying output stream.
        explicit OutputStreamWriter(std::unique_ptr<AbstractWriter> outputStream);

        /// @brief Destructor.
        ~OutputStreamWriter() override;

        /// @brief Get the encoding of the writer.
        /// @return The encoding name as a string.
        [[nodiscard]] std::string getEncoding() const;

        /// @brief Write a single character.
        /// @param c The character to write.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        void write(char c) override;

        /// @brief Write an entire character array.
        /// @param cBuf The character array to write.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        void write(const std::vector<char>& cBuf) override;

        /// @brief Write a portion of a character array.
        /// @param cBuf The character array to write from.
        /// @param off The offset in the array to start writing from.
        /// @param len The number of characters to write.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        /// @throws std::out_of_range If offset and length exceed buffer size.
        void write(const std::vector<char>& cBuf, size_t off, size_t len) override;

        /// @brief Write a string.
        /// @param str The string to write.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        void write(const std::string& str) override;

        /// @brief Write a portion of a string.
        /// @param str The string to write from.
        /// @param off The offset in the string to start writing from.
        /// @param len The number of characters to write.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        /// @throws std::out_of_range If offset and length exceed string size.
        void write(const std::string& str, size_t off, size_t len) override;

        /// @brief Flush the stream.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        void flush() override;

        /// @brief Close the writer.
        /// @throws std::ios_base::failure If the output stream is not available.
        void close() override;

        /// @brief Checks if the stream is closed.
        /// @return true if the stream is closed, false otherwise.
        [[nodiscard]] bool isClosed() const override;

        /// @brief Append a character to the writer.
        /// @param c The character to append.
        /// @return A reference to this writer.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        AbstractWriter& append(char c) override;

        /// @brief Append a string to the writer.
        /// @param csq The string to append.
        /// @return A reference to this writer.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        AbstractWriter& append(const std::string& csq) override;

        /// @brief Append a subsequence of a string to the writer.
        /// @param csq The string to append from.
        /// @param start The starting index of the subsequence.
        /// @param end The ending index of the subsequence.
        /// @return A reference to this writer.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        /// @throws std::out_of_range If start or end is out of bounds.
        AbstractWriter& append(const std::string& csq, size_t start, size_t end) override;

        /// @brief Convert the writer to a string representation.
        /// @return A string representation of the writer.
        [[nodiscard]] std::string toString() const override;

    private:
        std::unique_ptr<AbstractWriter> output_writer_;
        std::string charset_;
        bool closed_;

        /// @brief Checks if the stream is closed.
        /// @throws std::ios_base::failure if the stream is closed.
        void checkIfClosed() const;
    };
}