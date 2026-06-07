/**
 * @file IntBuffer.cc
 * @brief IntBuffer class implementation
 * @details This file contains the implementation of the IntBuffer class methods for Common library utilities.
 */

#include "filesystem/nio/IntBuffer.hpp"

#include <algorithm>

namespace common::filesystem::nio
{
    IntBuffer::IntBuffer(const size_t capacity)
    {
        capacity_ = capacity;
        limit_ = capacity;
        position_ = 0;
        buffer_.resize(capacity);
    }

    void IntBuffer::clear()
    {
        position_ = 0;
        limit_ = capacity_;
    }

    void IntBuffer::flip()
    {
        limit_ = position_;
        position_ = 0;
    }

    void IntBuffer::rewind()
    {
        position_ = 0;
    }

    void IntBuffer::compact()
    {
        const auto count = limit_ - position_;
        if (count > 0 && position_ > 0)
        {
            std::move(buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(limit_), buffer_.begin());
        }
        position_ = count;
        limit_ = capacity_;
    }

    int32_t IntBuffer::get()
    {
        if (!hasRemaining())
        {
            throw std::underflow_error("IntBuffer::get: Buffer underflow.");
        }
        return buffer_[position_++];
    }

    int32_t IntBuffer::get(const size_t index) const
    {
        if (index >= limit_)
        {
            throw std::out_of_range("IntBuffer::get: Index out of bounds.");
        }
        return buffer_[index];
    }

    void IntBuffer::put(const int32_t value)
    {
        if (!hasRemaining())
        {
            throw std::overflow_error("IntBuffer::put: Buffer overflow.");
        }
        buffer_[position_++] = value;
    }

    void IntBuffer::put(const size_t index, const int32_t value)
    {
        if (index >= limit_)
        {
            throw std::out_of_range("IntBuffer::put: Index out of bounds.");
        }
        buffer_[index] = value;
    }

    std::vector<int32_t> IntBuffer::getRemaining() const
    {
        if (position_ >= limit_)
        {
            return {};
        }
        return {buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(limit_)};
    }

    size_t IntBuffer::position() const
    {
        return position_;
    }

    void IntBuffer::position(const size_t newPosition)
    {
        if (newPosition > limit_)
        {
            throw std::out_of_range("IntBuffer::position: Position exceeds limit.");
        }
        position_ = newPosition;
    }

    size_t IntBuffer::limit() const
    {
        return limit_;
    }

    void IntBuffer::limit(const size_t newLimit)
    {
        if (newLimit > capacity_)
        {
            throw std::out_of_range("IntBuffer::limit: Limit exceeds capacity.");
        }
        if (position_ > newLimit)
        {
            position_ = newLimit;
        }
        limit_ = newLimit;
    }

    size_t IntBuffer::capacity() const
    {
        return capacity_;
    }

    bool IntBuffer::hasRemaining() const
    {
        return position_ < limit_;
    }

    size_t IntBuffer::remaining() const
    {
        return limit_ - position_;
    }
}
