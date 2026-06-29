/**
 * @file Path.hpp
 * @brief Thin value-type wrapper around std::filesystem::path
 * @details Provides a convenient, project-idiomatic path abstraction for
 *          use across the filesystem module.  Delegates all path logic to
 *          std::filesystem::path, adding cross-class conversions to File
 *          and Directory.
 *
 * @par Thread Safety
 * This class is a value type and is thread-safe for reads.  Const methods
 * may be called concurrently on the same instance.
 *
 * @par Usage Example
 * @code
 * Path p("/tmp/data");
 * Path full = p.absolutePath();
 * File f = p.toFile();
 * Directory d = p.toDirectory();
 * @endcode
 */

#pragma once

#include <filesystem>
#include <functional>
#include <string>

// ── Forward declarations ──────────────────────────────────────────
namespace cppforge::filesystem::core
{
class File;
class Directory;
}  // namespace cppforge::filesystem::core

namespace cppforge::filesystem::core
{
// ── Path (value type) ─────────────────────────────────────────────
class Path final
{
public:
    // ── Construction ──────────────────────────────────────────────
    Path() noexcept = default;

    explicit Path(std::filesystem::path p);
    explicit Path(const std::string& p);
    explicit Path(const char* p);

    // ── Accessors ─────────────────────────────────────────────────
    /// Returns a const reference to the underlying native path.
    [[nodiscard]] auto native() const noexcept -> const std::filesystem::path&;

    /// Implicit conversion back to std::filesystem::path.
    operator const std::filesystem::path&() const noexcept;

    // ── Path Queries ──────────────────────────────────────────────
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] bool isAbsolute() const noexcept;
    [[nodiscard]] bool isRelative() const noexcept;
    [[nodiscard]] std::string filename() const;
    [[nodiscard]] std::string stem() const;
    [[nodiscard]] std::string extension() const;

    // ── Path Manipulation ─────────────────────────────────────────
    [[nodiscard]] auto operator/(const Path& rhs) const -> Path;
    [[nodiscard]] auto parentPath() const -> Path;
    [[nodiscard]] auto absolutePath() const -> Path;
    [[nodiscard]] auto lexicallyNormal() const -> Path;

    // ── Comparison ────────────────────────────────────────────────
    [[nodiscard]] bool operator==(const Path& rhs) const noexcept;
    [[nodiscard]] bool operator!=(const Path& rhs) const noexcept;

    // ── Conversion ────────────────────────────────────────────────
    [[nodiscard]] std::string string() const;
    [[nodiscard]] size_t hashCode() const;

    /// Convert this path to a File object (does not check existence).
    [[nodiscard]] auto toFile() const -> File;

    /// Convert this path to a Directory object (does not check existence).
    [[nodiscard]] auto toDirectory() const -> Directory;

private:
    std::filesystem::path path_;
};

}  // namespace cppforge::filesystem::core

// ── std::hash specialization ─────────────────────────────────────
template <>
struct std::hash<cppforge::filesystem::core::Path>
{
    [[nodiscard]] auto operator()(const cppforge::filesystem::core::Path& p) const noexcept -> size_t
    {
        return p.hashCode();
    }
};
