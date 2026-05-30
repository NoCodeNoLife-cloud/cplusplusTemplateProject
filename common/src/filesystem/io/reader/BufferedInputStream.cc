/**
 * @file BufferedInputStream.cc
 * @brief BufferedInputStream class implementation
 * @details This file contains the implementation of the BufferedInputStream class methods for Common library utilities.
 */

#include "src/filesystem/io/reader/BufferedInputStream.hpp"

#include <fmt/format.h>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>

#include "src/filesystem/io/reader/AbstractInputStream.hpp"
#include "src/filesystem/io/reader/BufferedReader.hpp"
#include "src/filesystem/io/reader/FilterInputStream.hpp"

namespace common::filesystem
{
    BufferedInputStream::BufferedInputStream(std::unique_ptr<AbstractInputStream> in) noexcept : BufferedInputStream(std::move(in), DEFAULT_BUFFER_SIZE)
    {
    }

    BufferedInputStream::BufferedInputStream(std::unique_ptr<AbstractInputStream> in, const size_t size) : FilterInputStream(std::move(in)), buf_(size)
    {
        if (!input_stream_)
        {
            throw std::invalid_argument("BufferedInputStream: Input stream cannot be null");
        }
        if (size == 0)
        {
            throw std::invalid_argument("BufferedInputStream: Buffer size must be greater than zero");
        }
    }

    size_t BufferedInputStream::available() const noexcept
    {
        return count_ - pos_ + input_stream_->available();
    }

    void BufferedInputStream::close() noexcept
    {
        input_stream_->close();
        buf_.clear();
    }

    void BufferedInputStream::mark(const int32_t readLimit)
    {
        mark_limit_ = readLimit;
        input_stream_->mark(readLimit);
        mark_pos_ = pos_;
    }

    bool BufferedInputStream::markSupported() const noexcept
    {
        return true;
    }

    std::byte BufferedInputStream::read()
    {
        if (pos_ >= count_)
        {
            fillBuffer();
            if (pos_ >= count_)
            {
                return static_cast<std::byte>(-1);
            }
        }
        return buf_[pos_++];
    }

    template <typename Operation>
    size_t BufferedInputStream::processWithBuffer(Operation&& op)
    {
        if (const size_t bytesAvailable = count_ - pos_; bytesAvailable == 0)
        {
            fillBuffer();
            if (count_ - pos_ == 0)
            {
                return 0; // No more data available
            }
        }

        return op(count_ - pos_); // Pass available bytes to the operation
    }

    size_t BufferedInputStream::read(std::vector<std::byte>& buffer, const size_t offset, const size_t len)
    {
        if (offset > buffer.size() || len > buffer.size() - offset)
        {
            throw std::out_of_range("BufferedInputStream::read: Buffer offset/length out of range");
        }

        if (len == 0)
        {
            return 0;
        }

        size_t totalBytesRead = 0;
        size_t currentOffset = offset;
        size_t remainingLen = len;

        while (remainingLen > 0)
        {
            const size_t bytesProcessed = processWithBuffer([&](const size_t available) -> size_t
            {
                const size_t bytesToRead = std::min(remainingLen, available);
                std::copy_n(buf_.begin() + static_cast<std::ptrdiff_t>(pos_), bytesToRead, buffer.begin() + static_cast<std::ptrdiff_t>(currentOffset));
                pos_ += bytesToRead;
                currentOffset += bytesToRead;
                remainingLen -= bytesToRead;
                return bytesToRead;
            });

            if (bytesProcessed == 0)
            {
                break; // No more data available
            }

            totalBytesRead += bytesProcessed;
        }

        return totalBytesRead;
    }

    void BufferedInputStream::reset()
    {
        pos_ = mark_pos_;
        input_stream_->reset();
    }

    size_t BufferedInputStream::skip(const size_t n)
    {
        if (n == 0)
        {
            return 0;
        }

        size_t skipped = 0;
        size_t remaining = n;

        while (remaining > 0)
        {
            const size_t bytesProcessed = processWithBuffer([&](const size_t available) -> size_t
            {
                const size_t bytesToSkip = std::min(remaining, available);
                pos_ += bytesToSkip;
                remaining -= bytesToSkip;
                return bytesToSkip;
            });

            if (bytesProcessed == 0)
            {
                break; // No more data available
            }

            skipped += bytesProcessed;
        }

        return skipped;
    }

    bool BufferedInputStream::isClosed() const noexcept
    {
        return !input_stream_ || input_stream_->isClosed();
    }

    void BufferedInputStream::fillBuffer()
    {
        count_ = input_stream_->read(buf_, 0, buf_.size());
        pos_ = 0;
        if (count_ > 0)
        {
        }
        else
        {
        }
    }
}
