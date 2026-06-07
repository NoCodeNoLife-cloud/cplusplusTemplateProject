/**
 * @file BufferedWriter.cc
 * @brief BufferedWriter class implementation
 * @details This file contains the implementation of the BufferedWriter class methods for Common library utilities.
 */

#include "filesystem/io/writer/BufferedWriter.hpp"

#include <stdexcept>

namespace common::filesystem::io::writer
{
    BufferedWriter::BufferedWriter(std::unique_ptr<std::ofstream> os, const size_t size) : output_stream_(std::move(os)), buffer_size_(size)
    {
        if (!output_stream_)
        {
            throw std::runtime_error("Output stream is null.");
        }
        if (!output_stream_->is_open())
        {
            throw std::runtime_error("Output stream is not open.");
        }
        buffer_.reserve(buffer_size_);
    }

    BufferedWriter::~BufferedWriter()
    {
        try
        {
            close();
        }
        catch (...)
        {
            // Ignore exceptions during destruction
        }
    }

    void BufferedWriter::write(const std::string& str)
    {
        if (closed_ || !output_stream_)
        {
            return;
        }
        if (str.size() > buffer_size_)
        {
            flush();
            *output_stream_ << str;
        }
        else
        {
            buffer_.insert(buffer_.end(), str.begin(), str.end());
            checkAndFlush();
        }
    }

    void BufferedWriter::write(const std::vector<char>& cBuf, const size_t off, const size_t len)
    {
        if (closed_ || len == 0 || !output_stream_)
        {
            return;
        }

        if (off > cBuf.size() || len > cBuf.size() - off)
        {
            throw std::out_of_range("Offset and length are out of the bounds of the buffer.");
        }

        if (len > buffer_size_)
        {
            flush();
            output_stream_->write(cBuf.data() + off, static_cast<std::streamsize>(len));
        }
        else
        {
            buffer_.insert(buffer_.end(), cBuf.begin() + static_cast<std::ptrdiff_t>(off),
                           cBuf.begin() + static_cast<std::ptrdiff_t>(off + len));
            checkAndFlush();
        }
    }

    void BufferedWriter::flush()
    {
        if (!output_stream_)
        {
            return;
        }
        if (!buffer_.empty())
        {
            output_stream_->write(buffer_.data(), static_cast<std::streamsize>(buffer_.size()));
            buffer_.clear();
        }
        output_stream_->flush();
    }

    void BufferedWriter::close()
    {
        if (closed_)
        {
            return;
        }
        closed_ = true;
        if (output_stream_ && output_stream_->is_open())
        {
            flush();
            output_stream_->close();
        }
        output_stream_.reset();
    }

    BufferedWriter& BufferedWriter::append(const char c)
    {
        if (closed_)
        {
            return *this;
        }
        buffer_.push_back(c);
        checkAndFlush();
        return *this;
    }

    BufferedWriter& BufferedWriter::append(const std::string& str)
    {
        if (closed_)
        {
            return *this;
        }
        buffer_.insert(buffer_.end(), str.begin(), str.end());
        checkAndFlush();
        return *this;
    }

    BufferedWriter& BufferedWriter::append(const std::string& str, const size_t start, const size_t end)
    {
        if (closed_)
        {
            return *this;
        }
        if (start < str.length() && end <= str.length() && start < end)
        {
            buffer_.insert(buffer_.end(), str.begin() + static_cast<std::ptrdiff_t>(start),
                           str.begin() + static_cast<std::ptrdiff_t>(end));
            checkAndFlush();
        }
        return *this;
    }

    BufferedWriter& BufferedWriter::append(const std::string_view str)
    {
        if (closed_)
        {
            return *this;
        }
        buffer_.insert(buffer_.end(), str.begin(), str.end());
        checkAndFlush();
        return *this;
    }

    BufferedWriter& BufferedWriter::append(const char* str)
    {
        if (closed_)
        {
            return *this;
        }
        if (str)
        {
            const std::string_view sv(str);
            buffer_.insert(buffer_.end(), sv.begin(), sv.end());
            checkAndFlush();
        }
        return *this;
    }

    BufferedWriter& BufferedWriter::append(const std::initializer_list<char> chars)
    {
        if (closed_)
        {
            return *this;
        }
        buffer_.insert(buffer_.end(), chars.begin(), chars.end());
        checkAndFlush();
        return *this;
    }

    BufferedWriter& BufferedWriter::append(const char* chars, const size_t count)
    {
        if (closed_)
        {
            return *this;
        }
        if (chars)
        {
            buffer_.insert(buffer_.end(), chars, chars + count);
            checkAndFlush();
        }
        return *this;
    }

    BufferedWriter& BufferedWriter::append(const char c, const size_t count)
    {
        if (closed_)
        {
            return *this;
        }
        buffer_.insert(buffer_.end(), count, c);
        checkAndFlush();
        return *this;
    }

    BufferedWriter& BufferedWriter::append(const std::span<const char> chars)
    {
        if (closed_)
        {
            return *this;
        }
        buffer_.insert(buffer_.end(), chars.begin(), chars.end());
        checkAndFlush();
        return *this;
    }

    BufferedWriter& BufferedWriter::newLine()
    {
        return append('\n');
    }

    std::string BufferedWriter::toString() const
    {
        return {buffer_.begin(), buffer_.end()};
    }

    bool BufferedWriter::isClosed() const
    {
        return closed_ || output_stream_ == nullptr || !output_stream_->is_open();
    }

    void BufferedWriter::checkAndFlush()
    {
        if (buffer_.size() >= buffer_size_)
        {
            flush();
        }
    }
}
