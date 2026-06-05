/**
 * @file ByteArrayInputStream.cc
 * @brief ByteArrayInputStream class implementation
 * @details This file contains the implementation of the ByteArrayInputStream class methods for Common library utilities.
 */

#include "filesystem/io/reader/ByteArrayInputStream.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <fmt/format.h>

namespace common::filesystem
{
    ByteArrayInputStream::ByteArrayInputStream(const std::vector<std::byte>& buf) : buffer_(buf)
    {
        if (buffer_.empty())
        {
            setEof();
        }
    }

    std::byte ByteArrayInputStream::read()
    {
        if (closed_ || pos_ >= buffer_.size())
        {
            setEof();
            return static_cast<std::byte>(-1);
        }
        const auto result = buffer_[pos_++];
        if (pos_ >= buffer_.size())
        {
            setEof();
        }
        return result;
    }

    size_t ByteArrayInputStream::read(std::vector<std::byte>& cBuf)
    {
        if (closed_ || pos_ >= buffer_.size())
        {
            return 0;
        }

        const size_t remaining = buffer_.size() - pos_;
        const size_t bytesToRead = std::min(cBuf.size(), remaining);

        std::copy_n(buffer_.begin() + static_cast<std::ptrdiff_t>(pos_), bytesToRead, cBuf.begin());
        pos_ += bytesToRead;
        if (pos_ >= buffer_.size())
        {
            setEof();
        }
        return bytesToRead;
    }

    size_t ByteArrayInputStream::read(std::vector<std::byte>& cBuf, const size_t off, const size_t len)
    {
        if (off > cBuf.size() || len > cBuf.size() - off)
        {
            throw std::out_of_range("ByteArrayInputStream::read: Offset and length exceed the size of the buffer. off=" + std::to_string(off) + ", len=" + std::to_string(len) + ", buffer.size()=" + std::to_string(cBuf.size()));
        }

        if (closed_ || pos_ >= buffer_.size())
        {
            return 0;
        }

        const size_t remaining = buffer_.size() - pos_;
        const size_t bytesToRead = std::min(len, remaining);

        std::copy_n(buffer_.begin() + static_cast<std::ptrdiff_t>(pos_), bytesToRead, cBuf.begin() + static_cast<std::ptrdiff_t>(off));
        pos_ += bytesToRead;
        if (pos_ >= buffer_.size())
        {
            setEof();
        }
        return bytesToRead;
    }

    size_t ByteArrayInputStream::skip(const size_t n)
    {
        if (closed_)
        {
            return 0;
        }

        const size_t available = buffer_.size() - pos_;
        const size_t bytesToSkip = std::min(n, available);
        pos_ += bytesToSkip;
        if (pos_ >= buffer_.size())
        {
            setEof();
        }
        return bytesToSkip;
    }

    size_t ByteArrayInputStream::available()
    {
        if (closed_)
        {
            return 0;
        }
        return buffer_.size() - pos_;
    }

    void ByteArrayInputStream::reset()
    {
        if (closed_)
        {
            throw std::runtime_error("ByteArrayInputStream::reset: Stream is closed");
        }
        clearEof();
        pos_ = mark_position_;
    }

    void ByteArrayInputStream::mark(const int32_t readLimit)
    {
        if (closed_)
        {
            throw std::runtime_error("ByteArrayInputStream::mark: Stream is closed");
        }
        mark_position_ = pos_;
    }

    bool ByteArrayInputStream::markSupported() const
    {
        return true;
    }

    void ByteArrayInputStream::close()
    {
        closed_ = true;
        setEof();
    }

    bool ByteArrayInputStream::isClosed() const
    {
        return closed_;
    }
}
