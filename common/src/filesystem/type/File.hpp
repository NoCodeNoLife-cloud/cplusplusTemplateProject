/**
 * @file File.hpp
 * @brief File class declaration
 * @details This header defines the File class that provides functionality for Common library utilities.
 */

#pragma once
#include <windows.h>

#include <chrono>
#include <filesystem>
#include <string>

namespace common::filesystem {
/// @brief A class representing a file in the file system
/// This class provides various operations that can be performed on files,
/// such as checking permissions, creating, deleting, renaming, and retrieving
/// file properties like size, modification time, and path information.
class File final {
public:
    /// @brief Constructs a File object with the specified path
    /// @param path The file path as a string
    explicit File(const std::string& path);

    /// @brief Constructs a File object with the specified path
    /// @param path The file path as a filesystem path
    explicit File(std::filesystem::path path);

    /// @brief Constructs a File object with the specified path
    /// @param path The file path as a C-string
    explicit File(const char* path);

    /// @brief Destructor
    ~File();

    /// @brief Checks if the file can be executed
    /// @return true if the file can be executed, false otherwise
    [[nodiscard]] auto canExecute() const noexcept -> bool;

    /// @brief Checks if the file can be read
    /// @return true if the file can be read, false otherwise
    [[nodiscard]] auto canRead() const noexcept -> bool;

    /// @brief Checks if the file can be written to
    /// @return true if the file can be written to, false otherwise
    [[nodiscard]] auto canWrite() const noexcept -> bool;

    /// @brief Checks if the file exists
    /// @return true if the file exists, false otherwise
    [[nodiscard]] auto exists() const noexcept -> bool;

    /// @brief Checks if the path refers to a regular file
    /// @return true if it's a regular file, false otherwise
    [[nodiscard]] auto isFile() const noexcept -> bool;

    /// @brief Checks if the file is hidden
    /// @return true if the file is hidden, false otherwise
    [[nodiscard]] auto isHidden() const noexcept -> bool;

    /// @brief Checks if the file path is absolute
    /// @return true if the path is absolute, false otherwise
    [[nodiscard]] auto isAbsolute() const noexcept -> bool;

    /// @brief Creates a new file
    /// @return true if the file was created successfully, false otherwise
    [[nodiscard]] auto createNewFile() const -> bool;

    /// @brief Deletes the file
    /// @return true if the file was deleted successfully, false otherwise
    [[nodiscard]] auto deleteFile() const noexcept -> bool;

    /// @brief Renames the file to the destination file
    /// @param dest The destination file
    /// @return true if the file was renamed successfully, false otherwise
    [[nodiscard]] auto renameTo(const File& dest) const noexcept -> bool;

    /// @brief Copies the file to the destination path
    /// @param dest The destination file
    /// @return true if the file was copied successfully, false otherwise
    [[nodiscard]] auto copyTo(const File& dest) const -> bool;

    /// @brief Gets the length of the file
    /// @return The file length in bytes
    [[nodiscard]] auto length() const noexcept -> int64_t;

    /// @brief Gets the last modified time of the file
    /// @return The last modified time as a timestamp
    [[nodiscard]] auto lastModified() const noexcept -> int64_t;

    /// @brief Sets the last modified time of the file
    /// @param time The time to set as last modified time
    /// @return true if the time was set successfully, false otherwise
    [[nodiscard]] auto setLastModified(int64_t time) const noexcept -> bool;

    /// @brief Sets the file as read-only
    /// @return true if the file was set as read-only successfully, false otherwise
    [[nodiscard]] auto setReadOnly() const noexcept -> bool;

    /// @brief Gets the absolute path of the file
    /// @return The absolute path as a string
    [[nodiscard]] auto getAbsolutePath() const -> std::string;

    /// @brief Gets the absolute file object
    /// @return A File object representing the absolute path
    [[nodiscard]] auto getAbsoluteFile() const -> File;

    /// @brief Gets the name of the file
    /// @return The file name as a string
    [[nodiscard]] auto getName() const noexcept -> std::string;

    /// @brief Gets the file extension
    /// @return The file extension as a string (including the dot)
    [[nodiscard]] auto getExtension() const noexcept -> std::string;

    /// @brief Gets the parent directory path
    /// @return The parent directory path as a string
    [[nodiscard]] auto getParent() const noexcept -> std::string;

    /// @brief Gets the parent directory as a File object
    /// @return A File object representing the parent directory
    [[nodiscard]] auto getParentFile() const -> File;

    /// @brief Gets the file path
    /// @return The file path as a string
    [[nodiscard]] auto getPath() const noexcept -> std::string;

    /// @brief Gets the total space of the file system
    /// @return The total space in bytes
    [[nodiscard]] auto getTotalSpace() const noexcept -> int64_t;

    /// @brief Gets the usable space of the file system
    /// @return The usable space in bytes
    [[nodiscard]] auto getUsableSpace() const noexcept -> int64_t;

    /// @brief Gets the file size in a human-readable format
    /// @return The file size as a string with appropriate units (B, KB, MB, GB)
    [[nodiscard]] auto getSizeString() const noexcept -> std::string;

    /// @brief Gets the hash code of the file path
    /// @return The hash code as a size_t
    [[nodiscard]] auto hashCode() const noexcept -> size_t;

    /// @brief Converts the file path to a URI string
    /// @return The URI as a string
    [[nodiscard]] auto toURI() const noexcept -> std::string;

    /// @brief Prints files with depth information
    /// @param file_path The path to start printing from
    static auto printFilesWithDepth(const std::filesystem::path& file_path) -> void;

    /// @brief Gets the MD5 hash of a file
    /// @param filePath The path to the file
    /// @return The MD5 hash as a string
    static auto getFileMD5(const std::filesystem::path& filePath) -> std::string;

private:
    std::filesystem::path file_path_{};
};
}