/**
 * @file LongBuffer.cc
 * @brief LongBuffer class implementation
 * @details This file contains the implementation of the LongBuffer class methods for Common library utilities.
 */

#include "filesystem/nio/LongBuffer.hpp"

#include <fmt/format.h>

namespace common::filesystem::nio
{
    LongBuffer::LongBuffer(const std::size_t capacity)
    {
        capacity_ = capacity;
        limit_ = capacity;
        buffer_.resize(capacity);
    }

    int64_t LongBuffer::get()
    {
        if (!hasRemaining())
        {
            throw std::underflow_error("LongBuffer::get: No remaining elements to get");
        }
        return buffer_[position_++];
    }

    void LongBuffer::put(const int64_t value)
    {
        if (!hasRemaining())
        {
            throw std::overflow_error("LongBuffer::put: No remaining space to put");
        }
        buffer_[position_++] = value;
    }

    void LongBuffer::compact()
    {
        const auto count = limit_ - position_;
        if (count > 0 && position_ > 0)
        {
            std::move(buffer_.begin() + static_cast<std::ptrdiff_t>(position_),
                      buffer_.begin() + static_cast<std::ptrdiff_t>(limit_),
                      buffer_.begin());
        }
        position_ = count;
        limit_ = capacity_;
    }

    std::vector<int64_t> LongBuffer::getRemaining() const
    {
        if (position_ >= limit_)
        {
            return {};
        }
        return {buffer_.begin() + static_cast<std::ptrdiff_t>(position_),
                buffer_.begin() + static_cast<std::ptrdiff_t>(limit_)};
    }

    bool LongBuffer::hasRemaining() const
    {
        return position_ < limit_;
    }

    std::size_t LongBuffer::remaining() const
    {
        return limit_ - position_;
    }

    std::size_t LongBuffer::position() const
    {
        return position_;
    }

    void LongBuffer::position(const std::size_t newPosition)
    {
        if (newPosition > limit_)
        {
            throw std::out_of_range("LongBuffer::position: Position out of range");
        }
        position_ = newPosition;
    }

    std::size_t LongBuffer::limit() const
    {
        return limit_;
    }

    void LongBuffer::limit(const std::size_t newLimit)
    {
        if (newLimit > capacity_)
        {
            throw std::out_of_range("LongBuffer::limit: Limit exceeds capacity");
        }
        limit_ = newLimit;
        if (position_ > limit_)
        {
            position_ = limit_;
        }
    }

    std::size_t LongBuffer::capacity() const
    {
        return capacity_;
    }

    void LongBuffer::clear()
    {
        position_ = 0;
        limit_ = capacity_;
    }

    void LongBuffer::flip()
    {
        limit_ = position_;
        position_ = 0;
    }

    void LongBuffer::rewind()
    {
        position_ = 0;
    }
}
