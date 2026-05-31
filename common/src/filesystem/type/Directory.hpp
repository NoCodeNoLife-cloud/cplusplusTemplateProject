/**
 * @file Directory.hpp
 * @brief Directory class declaration
 * @details This header defines the Directory class that provides functionality for Common library utilities.
 */

#pragma once
#include <filesystem>
#include <vector>
#include <optional>
#include <cstdint>
#include <chrono>

namespace common::filesystem
{
    /// @brief A class for directory operations
    /// @details This class provides various operations for directories, such as creation, deletion, listing contents, etc.
    class Directory
    {
    public:
        /// @brief Constructs a Directory object with the specified path
        /// @param filePath The path to the directory
        explicit Directory(std::filesystem::path filePath) noexcept;

        /// @brief Create a directory
        /// @return true if the directory was created successfully, false otherwise
        [[nodiscard]] bool mkdir() const noexcept;

        /// @brief Create directories recursively
        /// @param exist_ok If true, no error will be thrown if the directory already exists
        /// @return true if the directories were created successfully, false otherwise
        [[nodiscard]] bool mkdirs(bool exist_ok = false) const;

        /// @brief Check if the directory exists
        /// @return true if the directory exists, false otherwise
        [[nodiscard]] bool exists() const noexcept;

        /// @brief Check if the path is a directory
        /// @return true if the path is a directory, false otherwise
        [[nodiscard]] bool isDirectory() const noexcept;

        /// @brief Check if the directory is empty
        /// @return true if the directory is empty, false otherwise
        [[nodiscard]] bool isEmpty() const noexcept;

        /// @brief Remove the directory
        /// @return true if the directory was removed successfully, false otherwise
        [[nodiscard]] bool remove() const noexcept;

        /// @brief Remove the directory and all its contents
        /// @return The number of files removed
        [[nodiscard]] std::uintmax_t removeAll() const noexcept;

        /// @brief Move the directory to a destination
        /// @param destination The destination path
        /// @return true if the directory was moved successfully, false otherwise
        [[nodiscard]] bool move(const std::filesystem::path& destination) const noexcept;

        /// @brief Rename the directory
        /// @param newName The new name for the directory
        /// @return true if the directory was renamed successfully, false otherwise
        [[nodiscard]] bool rename(const std::string& newName) const noexcept;

        /// @brief Copy the directory to a destination
        /// @param destination The destination path
        /// @return true if the directory was copied successfully, false otherwise
        [[nodiscard]] bool copy(const std::filesystem::path& destination) const;

        /// @brief Get the size of the directory
        /// @return The size of the directory in bytes
        [[nodiscard]] std::uintmax_t size() const noexcept;

        /// @brief Get the last modified time of the directory
        /// @return The last modified time, or std::nullopt if an error occurred
        [[nodiscard]] std::optional<std::chrono::system_clock::time_point> lastModifiedTime() const;

        /// @brief List directory contents
        /// @param recursive Whether to list subdirectories recursively
        /// @return A vector of directory entries
        [[nodiscard]] std::vector<std::filesystem::directory_entry> listDir(bool recursive) const;

        /// @brief List directory contents
        /// @param dir_path The directory path to list
        /// @param recursive Whether to list subdirectories recursively
        /// @return A vector of directory entries
        [[nodiscard]] static std::vector<std::filesystem::directory_entry> listDir(const std::filesystem::path& dir_path, bool recursive);

        /// @brief List directory entries
        /// @param recursive Whether to list subdirectories recursively
        /// @return A vector of directory entries
        [[nodiscard]] std::vector<std::filesystem::directory_entry> listEntries(bool recursive = false) const;

        /// @brief Clear all contents of the directory
        /// @return true if the directory was cleared successfully, false otherwise
        [[nodiscard]] bool clearAll() const;

        /// @brief Get the current working directory
        /// @return The current working directory path
        [[nodiscard]] static std::filesystem::path getCurrentWorkingDirectory();

    private:
        std::filesystem::path dir_path_{};
    };
}