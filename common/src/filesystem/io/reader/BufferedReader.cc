/**
 * @file BufferedReader.cc
 * @brief BufferedReader class implementation
 * @details This file contains the implementation of the BufferedReader class methods for Common library utilities.
 */

#include "filesystem/io/reader/BufferedReader.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <fmt/format.h>

namespace common::filesystem::io::reader
{
    BufferedReader::BufferedReader(std::unique_ptr<AbstractReader> reader, const size_t size) : reader_(std::move(reader)), buffer_size_(size)
    {
        if (size == 0)
        {
            throw std::invalid_argument("Buffer size must be greater than 0");
        }
        buffer_.resize(size);
    }

    void BufferedReader::close()
    {
        reader_->close();
    }

    void BufferedReader::mark(const size_t readAheadLimit)
    {
        if (readAheadLimit == 0)
        {
            throw std::invalid_argument("Mark limit must be greater than 0");
        }
        reader_->mark(readAheadLimit);
        mark_limit_ = pos_;
    }

    bool BufferedReader::markSupported() const
    {
        return true;
    }

    void BufferedReader::reset()
    {
        reader_->reset();
        pos_ = mark_limit_;
    }

    std::optional<char> BufferedReader::read()
    {
        if (pos_ >= count_)
        {
            if (!fillBuffer())
            {
                return std::nullopt;
            }
        }
        return buffer_[pos_++];
    }

    int BufferedReader::read(std::vector<char>& cBuf, const size_t off, const size_t len)
    {
        if (off > cBuf.size() || len > cBuf.size() - off)
        {
            throw std::out_of_range("Buffer offset/length out of range");
        }

        if (len == 0)
        {
            return 0;
        }

        size_t totalBytesRead = 0;
        size_t currentOffset = off;
        size_t remainingLen = len;

        while (remainingLen > 0)
        {
            if (pos_ >= count_)
            {
                if (!fillBuffer())
                {
                    break;
                }
            }

            const size_t bytesAvailable = count_ - pos_;
            const size_t bytesToRead = std::min(bytesAvailable, remainingLen);
            std::copy_n(buffer_.begin() + static_cast<std::ptrdiff_t>(pos_), bytesToRead, cBuf.begin() + static_cast<std::ptrdiff_t>(currentOffset));
            totalBytesRead += bytesToRead;
            currentOffset += bytesToRead;
            remainingLen -= bytesToRead;
            pos_ += bytesToRead;
        }

        return totalBytesRead > 0 ? static_cast<int>(totalBytesRead) : -1;
    }

    std::string BufferedReader::readLine()
    {
        std::string line;

        // Pre-allocate small buffer to avoid multiple allocations for typical lines
        line.reserve(64);

        while (true)
        {
            if (pos_ >= count_)
            {
                if (!fillBuffer())
                {
                    break;
                }
            }

            const char ch = buffer_[pos_++];
            if (ch == '\n')
            {
                break;
            }
            if (ch != '\r')
            {
                line += ch;
            }
        }
        return line;
    }

    bool BufferedReader::ready() const
    {
        return reader_->ready();
    }

    size_t BufferedReader::skip(const size_t n)
    {
        if (n == 0)
        {
            return 0;
        }

        size_t skipped = 0;
        size_t remaining = n;

        while (remaining > 0)
        {
            if (pos_ >= count_)
            {
                if (!fillBuffer())
                {
                    break;
                }
            }

            const size_t bytesToSkip = std::min(count_ - pos_, remaining);
            pos_ += bytesToSkip;
            remaining -= bytesToSkip;
            skipped += bytesToSkip;
        }

        return skipped;
    }

    bool BufferedReader::isClosed() const
    {
        return !reader_ || reader_->isClosed();
    }

    bool BufferedReader::fillBuffer()
    {
        pos_ = 0;
        const int bytesRead = reader_->read(buffer_, 0, buffer_size_);
        count_ = bytesRead > 0 ? static_cast<size_t>(bytesRead) : 0;
        return count_ > 0;
    }
}
