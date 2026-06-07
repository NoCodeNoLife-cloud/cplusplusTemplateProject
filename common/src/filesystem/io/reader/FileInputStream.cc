/**
 * @file FileInputStream.cc
 * @brief FileInputStream class implementation
 * @details This file contains the implementation of the FileInputStream class methods for Common library utilities.
 */

#include "filesystem/io/reader/FileInputStream.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <fmt/format.h>
#include <glog/logging.h>

namespace common::filesystem::io::reader
{
    FileInputStream::FileInputStream(const std::string& name)
    {
        if (!std::filesystem::exists(name))
        {
            DLOG(WARNING) << fmt::format("FileInputStream: File does not exist - {}", name);
            throw std::invalid_argument("FileInputStream::FileInputStream: File does not exist - " + name);
        }
        if (std::filesystem::is_directory(name))
        {
            DLOG(WARNING) << fmt::format("FileInputStream: Path is a directory - {}", name);
            throw std::invalid_argument("FileInputStream::FileInputStream: Path is a directory - " + name);
        }
        file_stream_.open(name, std::ios::binary);
        if (!file_stream_.is_open())
        {
            DLOG(WARNING) << fmt::format("FileInputStream: Unable to open file - {}", name);
            throw std::ios_base::failure("FileInputStream::FileInputStream: Unable to open file - " + name);
        }
        file_name_ = name;
    }

    FileInputStream::FileInputStream(const char* name) : FileInputStream(std::string(name))
    {
    }

    FileInputStream::FileInputStream(const std::filesystem::path& file) : FileInputStream(file.string())
    {
    }

    FileInputStream::~FileInputStream()
    {
        close();
    }

    std::byte FileInputStream::read()
    {
        if (!isValid())
        {
            setEof();
            return static_cast<std::byte>(-1);
        }

        std::byte byte{};
        if (file_stream_.read(reinterpret_cast<char*>(&byte), 1))
        {
            return byte;
        }
        setEof();
        return static_cast<std::byte>(-1);
    }

    size_t FileInputStream::read(std::vector<std::byte>& buffer)
    {
        return read(buffer, 0, buffer.size());
    }

    size_t FileInputStream::read(std::vector<std::byte>& buffer, const size_t offset, const size_t len)
    {
        validateBufferParams(buffer, offset, len);

        if (!isValid())
        {
            return 0;
        }

        file_stream_.read(reinterpret_cast<char*>(buffer.data() + offset), static_cast<std::streamsize>(len));
        const auto bytes_read = file_stream_.gcount();
        return static_cast<size_t>(bytes_read);
    }

    size_t FileInputStream::skip(const size_t n)
    {
        if (!isValid())
        {
            return 0;
        }

        const auto current_pos = file_stream_.tellg();
        if (current_pos == std::streampos(-1))
        {
            return 0;
        }

        file_stream_.seekg(static_cast<std::streamoff>(n), std::ios::cur);
        if (!file_stream_.good())
        {
            // Restore the original position if seek failed
            file_stream_.seekg(current_pos, std::ios::beg);
            return 0;
        }

        const auto new_pos = file_stream_.tellg();
        if (new_pos == std::streampos(-1))
        {
            return 0;
        }

        const auto skipped = new_pos - current_pos;
        return static_cast<size_t>(skipped);
    }

    size_t FileInputStream::available()
    {
        if (!isValid())
        {
            return 0;
        }

        const auto current = file_stream_.tellg();
        if (current == std::streampos(-1))
        {
            return 0;
        }

        file_stream_.seekg(0, std::ios::end);
        const auto end = file_stream_.tellg();
        file_stream_.seekg(current, std::ios::beg);

        if (end == std::streampos(-1))
        {
            return 0;
        }

        const auto available_bytes = end - current;
        return static_cast<size_t>(std::max(available_bytes, static_cast<std::streamoff>(0)));
    }

    void FileInputStream::close()
    {
        if (file_stream_.is_open())
        {
            file_stream_.close();
        }
        closed_ = true;
    }

    [[nodiscard]] bool FileInputStream::isClosed() const
    {
        return closed_;
    }

    [[nodiscard]] bool FileInputStream::markSupported() const
    {
        return false;
    }

    bool FileInputStream::isValid() const
    {
        return !closed_ && file_stream_.good();
    }

    void FileInputStream::validateBufferParams(const std::vector<std::byte>& buffer, const size_t offset, const size_t len)
    {
        if (offset > buffer.size() || len > buffer.size() - offset)
        {
            throw std::out_of_range("FileInputStream::validateBufferParams: Invalid buffer, offset, or length - offset: " + std::to_string(offset) + ", len: " + std::to_string(len) + ", buffer_size: " + std::to_string(buffer.size()));
        }
    }
}
