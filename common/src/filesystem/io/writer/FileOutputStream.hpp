#pragma once
#include <filesystem>
#include <fstream>
#include <vector>

#include "AbstractOutputStream.hpp"

namespace common::filesystem {
/// @brief A class for writing data to a file.
/// @details This class provides methods for writing bytes to a file, either
/// appending to or overwriting the file's contents. It inherits from
/// AbstractOutputStream to provide a consistent interface for output streams.
class FileOutputStream final : public AbstractOutputStream {
public:
    /// @brief Constructs a FileOutputStream for the specified file path.
    /// @param name The file path as a string.
    /// @param append If true, opens file in append mode; otherwise truncates.
    /// @throws std::ios_base::failure If the path is a directory or file cannot be opened.
    FileOutputStream(const std::string& name, bool append);

    /// @brief Constructs a FileOutputStream for the specified file path.
    /// @param name The file path as a C-string.
    /// @param append If true, opens file in append mode; otherwise truncates.
    /// @throws std::ios_base::failure If the path is a directory or file cannot be opened.
    FileOutputStream(const char* name, bool append);

    /// @brief Constructs a FileOutputStream for the specified file path.
    /// @param file The file path as a filesystem path object.
    /// @param append If true, opens file in append mode; otherwise truncates.
    /// @throws std::ios_base::failure If the path is a directory or file cannot be opened.
    FileOutputStream(const std::filesystem::path& file, bool append);

    /// @brief Destructor that closes the file stream if open.
    ~FileOutputStream() override;

    /// @brief Writes a single byte to the file stream.
    /// @param b The byte to write.
    /// @throws std::ios_base::failure If the stream is not writable or in a bad state.
    auto write(std::byte b) -> void override;

    /// @brief Writes a vector of bytes to the file stream.
    /// @param buffer The vector of bytes to write.
    /// @throws std::ios_base::failure If the stream is not writable or in a bad state.
    auto write(const std::vector<std::byte>& buffer) -> void override;

    /// @brief Writes a portion of a vector of bytes to the file stream.
    /// @param buffer The vector of bytes to write from.
    /// @param offset The starting position in the buffer.
    /// @param len The number of bytes to write.
    /// @throws std::ios_base::failure If the stream is not writable or in a bad state.
    auto write(const std::vector<std::byte>& buffer, size_t offset, size_t len) -> void override;

    /// @brief Writes a sequence of bytes to the file stream.
    /// @param buffer The buffer containing bytes to write.
    /// @param length The number of bytes to write.
    /// @throws std::ios_base::failure If the stream is not writable or in a bad state.
    auto write(const std::byte* buffer, size_t length) -> void override;

    /// @brief Flushes the file stream.
    /// @throws std::ios_base::failure If the stream is not writable or in a bad state.
    auto flush() -> void override;

    /// @brief Closes the file stream.
    /// @throws std::ios_base::failure If the stream is not writable.
    auto close() -> void override;

    /// @brief Checks if the stream is closed.
    /// @return true if the stream is closed, false otherwise.
    [[nodiscard]] auto isClosed() const -> bool override;

private:
    std::ofstream file_stream_;
    std::string file_name_;

    /// @brief Checks if the stream is writable, throwing an exception if not.
    /// @param message The message for the exception if the stream is not writable.
    /// @throws std::ios_base::failure If the stream is not open or not writable.
    auto checkStreamWritable(const std::string& message) const -> void;

    /// @brief Checks the stream state and throws an exception if badbit or failbit is set.
    /// @throws std::ios_base::failure If the stream is in a bad state.
    auto checkStreamState() const -> void;
};
}