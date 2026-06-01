/**
 * @file DoubleBuffer.cc
 * @brief DoubleBuffer class implementation
 * @details This file contains the implementation of the DoubleBuffer class methods for Common library utilities.
 */

#include "filesystem/nio/DoubleBuffer.hpp"

#include <algorithm>
#include <fmt/format.h>

namespace common::filesystem
{
    DoubleBuffer::DoubleBuffer(const size_t capacity)
    {
        position_ = 0;
        limit_ = capacity;
        capacity_ = capacity;
        buffer_.resize(capacity);
    }

    void DoubleBuffer::clear()
    {
        position_ = 0;
        limit_ = capacity_;
    }

    void DoubleBuffer::flip()
    {
        limit_ = position_;
        position_ = 0;
    }

    void DoubleBuffer::rewind()
    {
        position_ = 0;
    }

    void DoubleBuffer::compact()
    {
        if (position_ > 0)
        {
            std::move(buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(limit_), buffer_.begin());
            limit_ -= position_;
            position_ = 0;
        }
    }

    DoubleBuffer& DoubleBuffer::put(const double value)
    {
        if (!hasRemaining())
        {
            throw std::overflow_error("DoubleBuffer::put: Buffer overflow: Position exceeds limit.");
        }
        buffer_[position_++] = value;
        return *this;
    }

    DoubleBuffer& DoubleBuffer::put(const std::vector<double>& values)
    {
        if (values.empty())
        {
            return *this;
        }

        if (values.size() > remaining())
        {
            throw std::overflow_error("DoubleBuffer::put: Buffer overflow: Not enough space for all values.");
        }

        for (const double value : values)
        {
            buffer_[position_++] = value;
        }
        return *this;
    }

    double DoubleBuffer::get()
    {
        if (!hasRemaining())
        {
            throw std::underflow_error("DoubleBuffer::get: Buffer underflow: Position exceeds limit.");
        }
        return buffer_[position_++];
    }

    size_t DoubleBuffer::position() const
    {
        return position_;
    }

    void DoubleBuffer::position(const size_t newPosition)
    {
        if (newPosition > limit_)
        {
            throw std::out_of_range("DoubleBuffer::position: Position exceeds limit.");
        }
        position_ = newPosition;
    }

    size_t DoubleBuffer::limit() const
    {
        return limit_;
    }

    void DoubleBuffer::limit(const size_t newLimit)
    {
        if (newLimit > capacity_)
        {
            throw std::out_of_range("DoubleBuffer::limit: Limit exceeds capacity.");
        }
        if (position_ > newLimit)
        {
            position_ = newLimit;
        }
        limit_ = newLimit;
    }

    size_t DoubleBuffer::capacity() const
    {
        return capacity_;
    }

    bool DoubleBuffer::hasRemaining() const
    {
        return position_ < limit_;
    }

    size_t DoubleBuffer::remaining() const
    {
        return limit_ - position_;
    }

    std::vector<double> DoubleBuffer::getRemaining() const
    {
        if (position_ >= limit_)
        {
            return {};
        }
        return {buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(limit_)};
    }
}
