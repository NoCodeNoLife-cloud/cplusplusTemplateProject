/**
 * @file ShortBuffer.cc
 * @brief ShortBuffer class implementation
 * @details This file contains the implementation of the ShortBuffer class methods for Common library utilities.
 */

#include "buffer/ShortBuffer.hpp"

#include <algorithm>

namespace common::buffer
{
    ShortBuffer::ShortBuffer(const size_t capacity) : buffer_(capacity)
    {
        capacity_ = capacity;
        limit_ = capacity;
    }

    ShortBuffer ShortBuffer::wrap(const int16_t* data, const size_t size)
    {
        ShortBuffer sb(size);
        if (data != nullptr && size > 0)
        {
            std::memcpy(sb.buffer_.data(), data, size * sizeof(int16_t));
        }
        return sb;
    }

    int16_t ShortBuffer::get()
    {
        if (!hasRemaining())
        {
            throw std::underflow_error("ShortBuffer::get: No remaining elements to get");
        }
        return buffer_[position_++];
    }

    int16_t ShortBuffer::get(const size_t index) const
    {
        if (index >= limit_)
        {
            throw std::out_of_range("ShortBuffer::get: Index out of bounds");
        }
        return buffer_[index];
    }

    void ShortBuffer::put(const int16_t value)
    {
        if (!hasRemaining())
        {
            throw std::overflow_error("ShortBuffer::put: No remaining space to put");
        }
        buffer_[position_++] = value;
    }

    void ShortBuffer::put(const size_t index, const int16_t value)
    {
        if (index >= limit_)
        {
            throw std::out_of_range("ShortBuffer::put: Index out of bounds");
        }
        buffer_[index] = value;
    }

    void ShortBuffer::compact()
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

    std::vector<int16_t> ShortBuffer::getRemaining() const
    {
        if (position_ >= limit_)
        {
            return {};
        }
        return {buffer_.begin() + static_cast<std::ptrdiff_t>(position_),
                buffer_.begin() + static_cast<std::ptrdiff_t>(limit_)};
    }

    bool ShortBuffer::hasRemaining() const
    {
        return position_ < limit_;
    }

    size_t ShortBuffer::remaining() const
    {
        return limit_ - position_;
    }

    size_t ShortBuffer::position() const
    {
        return position_;
    }

    void ShortBuffer::position(const size_t newPosition)
    {
        if (newPosition > limit_)
        {
            throw std::out_of_range("ShortBuffer::position: Position out of range");
        }
        position_ = newPosition;
    }

    size_t ShortBuffer::limit() const
    {
        return limit_;
    }

    void ShortBuffer::limit(const size_t newLimit)
    {
        if (newLimit > capacity_)
        {
            throw std::out_of_range("ShortBuffer::limit: Limit exceeds capacity");
        }
        limit_ = newLimit;
        if (position_ > limit_)
        {
            position_ = limit_;
        }
    }

    size_t ShortBuffer::capacity() const
    {
        return capacity_;
    }

    void ShortBuffer::clear()
    {
        position_ = 0;
        limit_ = capacity_;
    }

    void ShortBuffer::flip()
    {
        limit_ = position_;
        position_ = 0;
    }

    void ShortBuffer::rewind()
    {
        position_ = 0;
    }

    int16_t* ShortBuffer::data()
    {
        return buffer_.data();
    }

    const int16_t* ShortBuffer::data() const
    {
        return buffer_.data();
    }
}
