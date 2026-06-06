/**
 * @file Directory.cc
 * @brief Directory class implementation
 * @details This file contains the implementation of the Directory class methods for Common library utilities.
 */

#include "filesystem/type/Directory.hpp"

#include <chrono>
#include <filesystem>
#include <optional>
#include <vector>

#include <glog/logging.h>

namespace common::filesystem
{
    Directory::Directory(std::filesystem::path filePath)
        : dir_path_(std::move(filePath))
    {
    }

    const std::filesystem::path& Directory::getPath() const
    {
        return dir_path_;
    }

    bool Directory::mkdir() const
    {
        try
        {
            return std::filesystem::create_directory(dir_path_);
        }
        catch (const std::exception& e)
        {
            LOG(WARNING) << "mkdir failed: " << e.what();
            return false;
        }
    }

    bool Directory::mkdirs(const bool exist_ok) const
    {
        try
        {
            if (std::filesystem::exists(dir_path_))
            {
                return exist_ok;
            }
            return std::filesystem::create_directories(dir_path_);
        }
        catch (const std::exception& e)
        {
            LOG(WARNING) << "mkdirs failed: " << e.what();
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
        catch (const std::exception& e)
        {
            LOG(WARNING) << "isEmpty failed: " << e.what();
            return false;
        }
    }

    bool Directory::remove() const
    {
        try
        {
            return std::filesystem::remove(dir_path_);
        }
        catch (const std::exception& e)
        {
            LOG(WARNING) << "remove failed: " << e.what();
            return false;
        }
    }

    std::uintmax_t Directory::removeAll() const
    {
        try
        {
            return std::filesystem::remove_all(dir_path_);
        }
        catch (const std::exception& e)
        {
            LOG(WARNING) << "removeAll failed: " << e.what();
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
        catch (const std::exception& e)
        {
            LOG(WARNING) << "move failed: " << e.what();
            return false;
        }
    }

    bool Directory::rename(const std::string& newName) const
    {
        try
        {
            const auto newPath = dir_path_.parent_path() / newName;
            std::filesystem::rename(dir_path_, newPath);
            return true;
        }
        catch (const std::exception& e)
        {
            LOG(WARNING) << "rename failed: " << e.what();
            return false;
        }
    }

    bool Directory::copy(const std::filesystem::path& destination) const
    {
        try
        {
            if (!isDirectory())
            {
                return false;
            }
            std::filesystem::create_directories(destination);
            for (const auto& entry : std::filesystem::recursive_directory_iterator(dir_path_))
            {
                const auto& entryPath = entry.path();
                const auto relativePath = entryPath.lexically_relative(dir_path_);
                const auto targetPath = destination / relativePath;
                if (entry.is_directory())
                {
                    std::filesystem::create_directory(targetPath);
                }
                else if (entry.is_regular_file())
                {
                    std::filesystem::copy_file(entryPath, targetPath,
                        std::filesystem::copy_options::overwrite_existing);
                }
            }
            return true;
        }
        catch (const std::exception& e)
        {
            LOG(WARNING) << "copy failed: " << e.what();
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
        catch (const std::exception& e)
        {
            LOG(WARNING) << "size calculation failed: " << e.what();
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
        catch (const std::exception& e)
        {
            LOG(WARNING) << "lastModifiedTime failed: " << e.what();
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
        catch (const std::exception& e)
        {
            LOG(WARNING) << "listDir failed: " << e.what();
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
        catch (const std::exception& e)
        {
            LOG(WARNING) << "clearAll failed: " << e.what();
            return false;
        }
    }

    std::filesystem::path Directory::getCurrentWorkingDirectory()
    {
        return std::filesystem::current_path();
    }
}
