/**
 * @file LongBuffer.cc
 * @brief LongBuffer class implementation
 * @details This file contains the implementation of the LongBuffer class methods for Common library utilities.
 */

#include "src/filesystem/nio/LongBuffer.hpp"

#include <fmt/format.h>

namespace common::filesystem
{
    LongBuffer::LongBuffer(const std::size_t capacity) : capacity_(capacity), limit_(capacity)
    {
        buffer_.resize(capacity);
    }

    int64_t LongBuffer::get()
    {
        if (!hasRemaining())
        {
            throw std::out_of_range("LongBuffer::get: No remaining elements to get");
        }
        return buffer_[position_++];
    }

    void LongBuffer::put(const int64_t value)
    {
        if (!hasRemaining())
        {
            throw std::out_of_range("LongBuffer::put: No remaining space to put");
        }
        buffer_[position_++] = value;
    }

    bool LongBuffer::hasRemaining() const noexcept
    {
        return position_ < limit_;
    }

    std::size_t LongBuffer::remaining() const noexcept
    {
        return limit_ - position_;
    }

    std::size_t LongBuffer::position() const noexcept
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

    std::size_t LongBuffer::limit() const noexcept
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

    std::size_t LongBuffer::capacity() const noexcept
    {
        return capacity_;
    }

    void LongBuffer::clear() noexcept
    {
        position_ = 0;
        limit_ = capacity_;
    }

    void LongBuffer::flip() noexcept
    {
        limit_ = position_;
        position_ = 0;
    }

    void LongBuffer::rewind() noexcept
    {
        position_ = 0;
    }
}