/**
 * @file FileOutputStream.cc
 * @brief FileOutputStream class implementation
 * @details This file contains the implementation of the FileOutputStream class methods for Common library utilities.
 */

#include "filesystem/io/writer/FileOutputStream.hpp"

#include <glog/logging.h>

namespace common::filesystem
{
    FileOutputStream::FileOutputStream(const std::string& name, const bool append)
    {
        file_stream_.open(name, std::ios::binary | (append ? std::ios::app : std::ios::trunc));
        if (!file_stream_.is_open())
        {
            DLOG(WARNING) << "FileOutputStream: Unable to open or create file - " << name;
            throw std::ios_base::failure("FileNotFoundException: Unable to open or create file.");
        }
        file_name_ = name;
    }

    FileOutputStream::FileOutputStream(const char* name, const bool append) : FileOutputStream(std::string(name), append)
    {
    }

    FileOutputStream::FileOutputStream(const std::filesystem::path& file, const bool append) : FileOutputStream(file.string(), append)
    {
    }

    FileOutputStream::~FileOutputStream()
    {
        if (file_stream_.is_open())
        {
            file_stream_.close();
        }
    }

    void FileOutputStream::write(std::byte b)
    {
        checkStreamWritable("Cannot write to closed or unwritable stream.");
        file_stream_.put(static_cast<char>(b));
        checkStreamState();
    }

    void FileOutputStream::write(const std::vector<std::byte>& buffer)
    {
        if (buffer.empty())
        {
            return;
        }
        write(buffer.data(), buffer.size());
    }

    void FileOutputStream::write(const std::vector<std::byte>& buffer, const size_t offset, const size_t len)
    {
        if (len == 0)
        {
            return;
        }
        if (offset > buffer.size() || len > buffer.size() - offset)
        {
            throw std::out_of_range("Buffer offset/length out of range");
        }
        write(buffer.data() + offset, len);
    }

    void FileOutputStream::write(const std::byte* buffer, const size_t length)
    {
        if (length == 0)
        {
            return;
        }

        if (!buffer)
        {
            throw std::invalid_argument("Buffer cannot be null");
        }
        checkStreamWritable("Cannot write to closed or unwritable stream.");

        file_stream_.write(reinterpret_cast<const char*>(buffer), static_cast<std::streamsize>(length));

        checkStreamState();
    }

    void FileOutputStream::flush()
    {
        checkStreamWritable("Cannot flush closed or unwritable stream.");
        file_stream_.flush();
        checkStreamState();
    }

    void FileOutputStream::close()
    {
        if (!file_stream_.is_open())
        {
            return;
        }
        try
        {
            file_stream_.flush();
        }
        catch (...)
        {
        }
        file_stream_.close();
    }

    [[nodiscard]] bool FileOutputStream::isClosed() const
    {
        return !file_stream_.is_open();
    }

    void FileOutputStream::checkStreamWritable(const std::string& message) const
    {
        if (!file_stream_.is_open())
        {
            throw std::ios_base::failure(message);
        }
    }

    void FileOutputStream::checkStreamState() const
    {
        if (file_stream_.bad())
        {
            throw std::ios_base::failure("IOException: Stream is in bad state.");
        }
        if (file_stream_.fail())
        {
            throw std::ios_base::failure("IOException: Stream operation failed.");
        }
    }
}
