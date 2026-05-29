/**
 * @file OutputStreamWriter.hpp
 * @brief OutputStreamWriter class declaration
 * @details This header defines the OutputStreamWriter class that provides functionality for Common library utilities.
 */

#pragma once
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <codecvt>
#include <memory>
#include <string>
#include <vector>

#include "AbstractWriter.hpp"

namespace common::filesystem
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
        /// @throws std::invalid_argument If the charset is not supported.
        OutputStreamWriter(std::unique_ptr<AbstractWriter> outputStream, const std::string& charsetName);

        /// @brief Constructs an OutputStreamWriter that uses the default charset.
        /// @param outputStream The underlying output stream.
        explicit OutputStreamWriter(std::unique_ptr<AbstractWriter> outputStream);

        /// @brief Destructor.
        ~OutputStreamWriter() override;

        /// @brief Get the encoding of the writer.
        /// @return The encoding name as a string.
        [[nodiscard]] auto getEncoding() const -> std::string;

        /// @brief Write a single character.
        /// @param c The character to write.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        auto write(char c) -> void override;

        /// @brief Write an entire character array.
        /// @param cBuf The character array to write.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        auto write(const std::vector<char>& cBuf) -> void override;

        /// @brief Write a portion of a character array.
        /// @param cBuf The character array to write from.
        /// @param off The offset in the array to start writing from.
        /// @param len The number of characters to write.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        /// @throws std::out_of_range If offset and length exceed buffer size.
        auto write(const std::vector<char>& cBuf, size_t off, size_t len) -> void override;

        /// @brief Write a string.
        /// @param str The string to write.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        auto write(const std::string& str) -> void override;

        /// @brief Write a portion of a string.
        /// @param str The string to write from.
        /// @param off The offset in the string to start writing from.
        /// @param len The number of characters to write.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        /// @throws std::out_of_range If offset and length exceed string size.
        auto write(const std::string& str, size_t off, size_t len) -> void override;

        /// @brief Flush the stream.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        auto flush() -> void override;

        /// @brief Close the writer.
        /// @throws std::ios_base::failure If the output stream is not available.
        auto close() -> void override;

        /// @brief Checks if the stream is closed.
        /// @return true if the stream is closed, false otherwise.
        [[nodiscard]] auto isClosed() const -> bool override;

        /// @brief Append a character to the writer.
        /// @param c The character to append.
        /// @return A reference to this writer.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        auto append(char c) -> AbstractWriter& override;

        /// @brief Append a string to the writer.
        /// @param csq The string to append.
        /// @return A reference to this writer.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        auto append(const std::string& csq) -> AbstractWriter& override;

        /// @brief Append a subsequence of a string to the writer.
        /// @param csq The string to append from.
        /// @param start The starting index of the subsequence.
        /// @param end The ending index of the subsequence.
        /// @return A reference to this writer.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        auto append(const std::string& csq, size_t start, size_t end) -> AbstractWriter& override;

        /// @brief Convert the writer to a string representation.
        /// @return A string representation of the writer.
        /// @throws std::ios_base::failure If the stream is closed or output stream is not available.
        [[nodiscard]] auto toString() const -> std::string override;

    private:
        std::unique_ptr<AbstractWriter> output_writer_;
        std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> converter_;
        std::string charset_;
        bool closed_;

        /// @brief Checks if the stream is closed.
        /// @throws std::ios_base::failure if the stream is closed.
        auto checkIfClosed() const -> void;

        /// @brief Checks if the output stream is available.
        /// @throws std::ios_base::failure if the output stream is not available.
        auto checkOutputStream() const -> void;
    };
}
