/**
 * @file BufferedInputStream.cc
 * @brief BufferedInputStream implementation â€?buffer fill, mark/reset, read
 * @details Implements buffered byte input: fillBuffer() refills the internal
 *          8 KB buffer from the underlying stream, read() copies from buffer,
 *          and mark/reset are supported up to the buffer capacity.
 */

#include "io/reader/BufferedInputStream.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <fmt/format.h>

namespace cppforge::io::reader
{
    BufferedInputStream::BufferedInputStream(std::unique_ptr<AbstractInputStream> in)  : BufferedInputStream(std::move(in), DEFAULT_BUFFER_SIZE)
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

    size_t BufferedInputStream::available() const
    {
        return count_ - pos_ + input_stream_->available();
    }

    void BufferedInputStream::close()
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

    bool BufferedInputStream::markSupported() const
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
                setEof();
                return static_cast<std::byte>(-1);
            }
        }
        return buf_[pos_++];
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

    int64_t BufferedInputStream::skip(const int64_t n)
    {
        if (n <= 0)
        {
            return 0;
        }

        int64_t skipped = 0;
        int64_t remaining = n;

        while (remaining > 0)
        {
            const size_t bytesProcessed = processWithBuffer([&](const size_t available) -> size_t
            {
                const size_t bytesToSkip = std::min(static_cast<size_t>(remaining), available);
                pos_ += bytesToSkip;
                remaining -= static_cast<int64_t>(bytesToSkip);
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

    bool BufferedInputStream::isClosed() const
    {
        return !input_stream_ || input_stream_->isClosed();
    }

    void BufferedInputStream::fillBuffer()
    {
        count_ = input_stream_->read(buf_, 0, buf_.size());
        pos_ = 0;
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
}
