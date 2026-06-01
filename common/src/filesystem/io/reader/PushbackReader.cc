/**
 * @file PushbackReader.cc
 * @brief PushbackReader class implementation
 * @details This file contains the implementation of the PushbackReader class methods for Common library utilities.
 */

#include "filesystem/io/reader/PushbackReader.hpp"

#include <algorithm>
#include <stdexcept>
#include <fmt/format.h>

namespace common::filesystem
{
    PushbackReader::PushbackReader(std::shared_ptr<AbstractReader> reader) : PushbackReader(std::move(reader), DEFAULT_BUFFER_SIZE)
    {
    }

    PushbackReader::PushbackReader(std::shared_ptr<AbstractReader> reader, const size_t size) : FilterReader(std::move(reader)), buffer_(size)
    {
        if (size == 0)
        {
            throw std::invalid_argument("PushbackReader::constructor: Buffer size must be greater than zero.");
        }
    }

    void PushbackReader::close()
    {
        closed_ = true;
        FilterReader::close();
        buffer_.clear();
    }

    void PushbackReader::mark(const size_t readAheadLimit)
    {
        static_cast<void>(readAheadLimit); // Unused parameter
        throw std::runtime_error("PushbackReader::mark: mark() not supported.");
    }

    bool PushbackReader::markSupported() const
    {
        return false;
    }

    int PushbackReader::read()
    {
        validateOpen();

        if (buffer_pos_ < buffer_.size())
        {
            return static_cast<unsigned char>(buffer_[buffer_pos_++]);
        }
        return FilterReader::read();
    }

    int PushbackReader::read(std::vector<char>& cBuf, const size_t off, const size_t len)
    {
        validateOpen();

        if (off > cBuf.size() || len > cBuf.size() - off)
        {
            throw std::out_of_range("PushbackReader::read: Buffer overflow.");
        }

        size_t bytesRead = 0;
        while (buffer_pos_ < buffer_.size() && bytesRead < len)
        {
            cBuf[off + bytesRead] = buffer_[buffer_pos_++];
            bytesRead++;
        }
        if (bytesRead < len)
        {
            const int underlyingBytesRead = FilterReader::read(cBuf, off + bytesRead, len - bytesRead);
            if (underlyingBytesRead == -1)
            {
                return bytesRead > 0 ? static_cast<int>(bytesRead) : -1;
            }
            bytesRead += static_cast<size_t>(underlyingBytesRead);
        }
        return static_cast<int>(bytesRead);
    }

    bool PushbackReader::ready() const
    {
        if (closed_ || !in_)
        {
            return false;
        }
        return buffer_pos_ < buffer_.size() || FilterReader::ready();
    }

    void PushbackReader::reset()
    {
        throw std::runtime_error("PushbackReader::reset: reset() not supported.");
    }

    size_t PushbackReader::skip(const size_t n)
    {
        validateOpen();

        size_t skipped = 0;
        if (buffer_pos_ < buffer_.size())
        {
            const size_t bufferRemaining = buffer_.size() - buffer_pos_;
            skipped = std::min(n, bufferRemaining);
            buffer_pos_ += skipped;
        }
        if (n > skipped)
        {
            skipped += FilterReader::skip(n - skipped);
        }
        return skipped;
    }

    void PushbackReader::unread(const std::vector<char>& cbuf)
    {
        unread(cbuf, 0, cbuf.size());
    }

    void PushbackReader::unread(const std::vector<char>& cBuf, const size_t off, const size_t len)
    {
        validateNotClosed();

        if (off > cBuf.size() || len > cBuf.size() - off)
        {
            throw std::out_of_range("PushbackReader::unread: Buffer offset/length out of range");
        }

        if (len > buffer_pos_)
        {
            throw std::overflow_error("PushbackReader::unread: Pushback buffer overflow.");
        }

        for (size_t i = 0; i < len; ++i)
        {
            buffer_[--buffer_pos_] = cBuf[off + len - 1 - i];
        }
    }

    void PushbackReader::unread(const int32_t c)
    {
        validateNotClosed();

        if (buffer_pos_ == 0)
        {
            throw std::overflow_error("PushbackReader::unread: Pushback buffer overflow.");
        }
        buffer_[--buffer_pos_] = static_cast<char>(c);
    }

    bool PushbackReader::isClosed() const
    {
        return closed_ || !in_ || in_->isClosed();
    }

    void PushbackReader::validateOpen() const
    {
        if (closed_ || !in_)
        {
            throw std::runtime_error("PushbackReader::operation: Stream is closed");
        }
    }

    void PushbackReader::validateNotClosed() const
    {
        if (closed_)
        {
            throw std::runtime_error("PushbackReader::operation: Stream is closed");
        }
    }
}
