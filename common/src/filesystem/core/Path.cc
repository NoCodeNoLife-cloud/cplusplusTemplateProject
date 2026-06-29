/**
 * @file Path.cc
 * @brief Path implementation �?delegates to std::filesystem::path
 * @details All string-based path operations forward to the standard
 *          library.  Cross-class conversions (toFile, toDirectory)
 *          include the respective headers at .cc level only, avoiding
 *          circular header dependencies.
 */

#include "filesystem/core/Path.hpp"

#include "filesystem/core/File.hpp"
#include "filesystem/core/Directory.hpp"

#include <utility>

namespace cppforge::filesystem::core
{
// ── Construction ────────────────────────────────────────────

Path::Path(std::filesystem::path p)
    : path_(std::move(p))
{
}

Path::Path(const std::string& p)
    : path_(p)
{
}

Path::Path(const char* p)
    : path_(p)
{
}

// ── Accessors ───────────────────────────────────────────────

auto Path::native() const noexcept -> const std::filesystem::path&
{
    return path_;
}

Path::operator const std::filesystem::path&() const noexcept
{
    return path_;
}

// ── Path Queries ────────────────────────────────────────────

bool Path::empty() const noexcept
{
    return path_.empty();
}

bool Path::isAbsolute() const noexcept
{
    return path_.is_absolute();
}

bool Path::isRelative() const noexcept
{
    return path_.is_relative();
}

std::string Path::filename() const
{
    return path_.filename().string();
}

std::string Path::stem() const
{
    return path_.stem().string();
}

std::string Path::extension() const
{
    return path_.extension().string();
}

// ── Path Manipulation ───────────────────────────────────────

auto Path::operator/(const Path& rhs) const -> Path
{
    return Path(path_ / rhs.path_);
}

auto Path::parentPath() const -> Path
{
    return Path(path_.parent_path());
}

auto Path::absolutePath() const -> Path
{
    return Path(std::filesystem::absolute(path_));
}

auto Path::lexicallyNormal() const -> Path
{
    return Path(path_.lexically_normal());
}

// ── Comparison ──────────────────────────────────────────────

bool Path::operator==(const Path& rhs) const noexcept
{
    return path_ == rhs.path_;
}

bool Path::operator!=(const Path& rhs) const noexcept
{
    return path_ != rhs.path_;
}

// ── Conversion ──────────────────────────────────────────────

std::string Path::string() const
{
    return path_.string();
}

size_t Path::hashCode() const
{
    return std::hash<std::filesystem::path>{}(path_);
}

auto Path::toFile() const -> File
{
    return File(path_);
}

auto Path::toDirectory() const -> Directory
{
    return Directory(path_);
}

}  // namespace cppforge::filesystem::core
