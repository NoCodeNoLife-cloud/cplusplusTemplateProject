/**
 * @file Directory.hpp
 * @brief Directory abstraction — listing, creation, recursive traversal
 * @description Represents a filesystem directory.  Provides operations for
 *          listing contents, creating/deleting directories, recursive
 *          traversal (depth-first), and filtering by extension or name
 *          pattern.
 *
 * @par Thread Safety
 * This class is **not** thread-safe for mutation.  Concurrent reads on
 * different Directory instances are safe.
 */

#pragma once
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

// ── Forward declarations ──────────────────────────────────────────
namespace common::filesystem::core
{
class Path;
class File;

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

    /// Returns a Path view of this directory's path.
    [[nodiscard]] auto toPath() const -> Path;

    /// Lists only regular files in this directory (optionally recursive).
    [[nodiscard]] auto listFiles(bool recursive = false) const -> std::vector<File>;

private:
    std::filesystem::path dir_path_;
};

}  // namespace common::filesystem::core
