/**
 * @file ByteBuffer.cc
 * @brief ByteBuffer class implementation
 * @details This file contains the implementation of the ByteBuffer class methods for Common library utilities.
 */

#include "filesystem/nio/ByteBuffer.hpp"

#include <algorithm>
#include <cstring>
#include <fmt/format.h>
#include <glog/logging.h>

namespace common::filesystem
{
    ByteBuffer::ByteBuffer(const size_t capacity) : buffer_(capacity)
    {
        capacity_ = capacity;
        limit_ = capacity;
    }

    size_t ByteBuffer::capacity() const
    {
        return capacity_;
    }

    size_t ByteBuffer::position() const
    {
        return position_;
    }

    void ByteBuffer::position(const size_t newPosition)
    {
        if (newPosition > limit_)
        {
            DLOG(WARNING) << fmt::format("ByteBuffer position {} exceeds limit {}", newPosition, limit_);
            throw std::out_of_range("ByteBuffer::position: Position exceeds the current limit.");
        }
        position_ = newPosition;
    }

    size_t ByteBuffer::limit() const
    {
        return limit_;
    }

    void ByteBuffer::limit(const size_t newLimit)
    {
        if (newLimit > capacity_)
        {
            DLOG(WARNING) << fmt::format("ByteBuffer limit {} exceeds capacity {}", newLimit, capacity_);
            throw std::out_of_range("ByteBuffer::limit: New limit exceeds capacity.");
        }
        limit_ = newLimit;
        if (position_ > limit_)
        {
            position_ = limit_;
        }
    }

    void ByteBuffer::clear()
    {
        position_ = 0;
        limit_ = capacity_;
    }

    void ByteBuffer::flip()
    {
        limit_ = position_;
        position_ = 0;
    }

    void ByteBuffer::rewind()
    {
        position_ = 0;
    }

    size_t ByteBuffer::remaining() const
    {
        return limit_ - position_;
    }

    bool ByteBuffer::hasRemaining() const
    {
        return position_ < limit_;
    }

    void ByteBuffer::compact()
    {
        if (position_ > 0)
        {
            std::move(buffer_.begin() + static_cast<std::ptrdiff_t>(position_),
                      buffer_.begin() + static_cast<std::ptrdiff_t>(limit_),
                      buffer_.begin());
            limit_ -= position_;
            position_ = 0;
        }
    }

    std::vector<std::byte> ByteBuffer::getRemaining() const
    {
        if (position_ >= limit_)
        {
            return {};
        }
        return {buffer_.begin() + static_cast<std::ptrdiff_t>(position_),
                buffer_.begin() + static_cast<std::ptrdiff_t>(limit_)};
    }

    void ByteBuffer::put(const std::byte value)
    {
        if (!hasRemaining())
        {
            DLOG(WARNING) << "ByteBuffer put: Buffer overflow";
            throw std::overflow_error("ByteBuffer::put: Buffer overflow");
        }
        buffer_[position_++] = value;
    }

    void ByteBuffer::put(const std::vector<std::byte>& src)
    {
        if (src.empty())
        {
            return;
        }

        if (src.size() > remaining())
        {
            throw std::overflow_error("ByteBuffer::put: Insufficient space in buffer");
        }

        std::memcpy(buffer_.data() + position_, src.data(), src.size());
        position_ += src.size();
    }

    std::byte ByteBuffer::get()
    {
        if (!hasRemaining())
        {
            DLOG(WARNING) << "ByteBuffer get: Buffer underflow";
            throw std::underflow_error("ByteBuffer::get: Buffer underflow");
        }
        return buffer_[position_++];
    }

    std::vector<std::byte> ByteBuffer::get(const size_t length)
    {
        if (length == 0)
        {
            return {};
        }

        if (length > remaining())
        {
            throw std::underflow_error("ByteBuffer::get: Insufficient data in buffer");
        }

        std::vector result(buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(position_ + length));
        position_ += length;
        return result;
    }
}
