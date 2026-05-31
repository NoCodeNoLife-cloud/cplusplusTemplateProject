/**
 * @file Directory.cc
 * @brief Directory class implementation
 * @details This file contains the implementation of the Directory class methods for Common library utilities.
 */

#include "filesystem/type/Directory.hpp"

#include <fmt/format.h>
#include <filesystem>
#include <queue>
#include <utility>
#include <vector>
#include <optional>
#include <chrono>
#include <glog/logging.h>

namespace common::filesystem
{
    Directory::Directory(std::filesystem::path filePath)  : dir_path_(std::move(filePath))
    {
    }

    bool Directory::mkdir() const
    {
        try
        {
            return std::filesystem::create_directory(dir_path_);
        }
        catch (...)
        {
            return false;
        }
    }

    bool Directory::mkdirs(const bool exist_ok) const
    {
        try
        {
            if (exist_ok)
            {
                return std::filesystem::create_directories(dir_path_);
            }
            // Check if any part of the path already exists
            if (std::filesystem::exists(dir_path_))
            {
                return false;
            }
            // For parent directories, we need to check recursively
            auto parent = dir_path_.parent_path();
            while (!parent.empty() && parent != dir_path_.root_path())
            {
                if (std::filesystem::exists(parent) && !std::filesystem::is_directory(parent))
                {
                    return false;
                }
                parent = parent.parent_path();
            }
            return std::filesystem::create_directories(dir_path_);
        }
        catch (...)
        {
            return false;
        }
    }

    bool Directory::exists() const
    {
        return std::filesystem::exists(dir_path_);
    }

    bool Directory::isDirectory() const
    {
        return std::filesystem::is_directory(dir_path_);
    }

    bool Directory::isEmpty() const
    {
        try
        {
            return std::filesystem::is_empty(dir_path_);
        }
        catch (...)
        {
            return false;
        }
    }

    bool Directory::remove() const
    {
        try
        {
            return std::filesystem::remove(dir_path_);
        }
        catch (...)
        {
            return false;
        }
    }

    std::uintmax_t Directory::removeAll() const
    {
        try
        {
            return std::filesystem::remove_all(dir_path_);
        }
        catch (...)
        {
            return 0;
        }
    }

    bool Directory::move(const std::filesystem::path& destination) const
    {
        try
        {
            std::filesystem::rename(dir_path_, destination);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool Directory::rename(const std::string& newName) const
    {
        try
        {
            auto newPath = dir_path_;
            newPath.replace_filename(newName);
            std::filesystem::rename(dir_path_, newPath);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool Directory::copy(const std::filesystem::path& destination) const
    {
        try
        {
            if (!exists() || !isDirectory())
            {
                return false;
            }
            if (!std::filesystem::create_directories(destination))
            {
                return false;
            }

            std::queue<std::pair<std::filesystem::path, std::filesystem::path>> dirQueue;
            dirQueue.emplace(dir_path_, destination);

            while (!dirQueue.empty())
            {
                const auto [srcPath, dstPath] = dirQueue.front();
                dirQueue.pop();

                for (const auto& entry : std::filesystem::directory_iterator(srcPath))
                {
                    const auto& entryPath = entry.path();
                    const auto relativePath = entryPath.lexically_relative(srcPath);
                    const auto targetPath = dstPath / relativePath;

                    if (entry.is_directory())
                    {
                        if (!std::filesystem::create_directory(targetPath))
                        {
                            return false;
                        }
                        dirQueue.emplace(entryPath, targetPath);
                    }
                    else if (entry.is_regular_file())
                    {
                        std::filesystem::copy_file(entryPath, targetPath, std::filesystem::copy_options::overwrite_existing);
                    }
                }
            }
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    std::uintmax_t Directory::size() const
    {
        std::uintmax_t total = 0;
        try
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(dir_path_))
            {
                if (entry.is_regular_file())
                {
                    total += entry.file_size();
                }
            }
        }
        catch (...)
        {
        }
        return total;
    }

    std::optional<std::chrono::system_clock::time_point> Directory::lastModifiedTime() const
    {
        try
        {
            const auto ftime = std::filesystem::last_write_time(dir_path_);
            return std::chrono::clock_cast<std::chrono::system_clock>(ftime);
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    std::vector<std::filesystem::directory_entry> Directory::listDir(const bool recursive) const
    {
        return listDir(dir_path_, recursive);
    }

    std::vector<std::filesystem::directory_entry> Directory::listDir(const std::filesystem::path& dir_path, const bool recursive)
    {
        std::vector<std::filesystem::directory_entry> entries;
        try
        {
            if (recursive)
            {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(dir_path))
                {
                    entries.push_back(entry);
                }
            }
            else
            {
                for (const auto& entry : std::filesystem::directory_iterator(dir_path))
                {
                    entries.push_back(entry);
                }
            }
        }
        catch (...)
        {
        }
        return entries;
    }

    std::vector<std::filesystem::directory_entry> Directory::listEntries(const bool recursive) const
    {
        return listDir(dir_path_, recursive);
    }

    bool Directory::clearAll() const
    {
        try
        {
            if (!std::filesystem::exists(dir_path_) || !std::filesystem::is_directory(dir_path_))
            {
                return false;
            }
            for (const auto& entry : std::filesystem::directory_iterator(dir_path_))
            {
                std::filesystem::remove_all(entry.path());
            }
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    std::filesystem::path Directory::getCurrentWorkingDirectory()
    {
        return std::filesystem::current_path();
    }
}