/**
 * @file Directory.hpp
 * @brief Directory class declaration
 * @details This header defines the Directory class that provides functionality for Common library utilities.
 */

#pragma once
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

namespace common::filesystem
{
    class Directory
    {
    public:
        explicit Directory(std::filesystem::path filePath);

        [[nodiscard]] bool mkdir() const;

        [[nodiscard]] bool mkdirs(bool exist_ok = false) const;

        [[nodiscard]] bool exists() const;

        [[nodiscard]] bool isDirectory() const;

        [[nodiscard]] bool isEmpty() const;

        [[nodiscard]] bool remove() const;

        [[nodiscard]] std::uintmax_t removeAll() const;

        [[nodiscard]] bool move(const std::filesystem::path& destination) const;

        [[nodiscard]] bool rename(const std::string& newName) const;

        [[nodiscard]] bool copy(const std::filesystem::path& destination) const;

        [[nodiscard]] std::uintmax_t size() const;

        [[nodiscard]] std::optional<std::chrono::system_clock::time_point> lastModifiedTime() const;

        [[nodiscard]] std::vector<std::filesystem::directory_entry> listDir(bool recursive) const;

        [[nodiscard]] static std::vector<std::filesystem::directory_entry> listDir(const std::filesystem::path& dir_path, bool recursive);

        [[nodiscard]] std::vector<std::filesystem::directory_entry> listEntries(bool recursive = false) const;

        [[nodiscard]] bool clearAll() const;

        [[nodiscard]] const std::filesystem::path& getPath() const;

        [[nodiscard]] static std::filesystem::path getCurrentWorkingDirectory();

    private:
        std::filesystem::path dir_path_;
    };
}
