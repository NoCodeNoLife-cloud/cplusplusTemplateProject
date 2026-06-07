/**
 * @file FloatBuffer.cc
 * @brief FloatBuffer class implementation
 * @details This file contains the implementation of the FloatBuffer class methods for Common library utilities.
 */

#include "filesystem/nio/FloatBuffer.hpp"

#include <algorithm>

namespace common::filesystem::nio
{
    FloatBuffer::FloatBuffer(const size_t capacity)
    {
        limit_ = capacity;
        position_ = 0;
        capacity_ = capacity;
        buffer_.resize(capacity);
    }

    FloatBuffer FloatBuffer::allocate(const size_t capacity)
    {
        return FloatBuffer(capacity);
    }

    void FloatBuffer::clear()
    {
        position_ = 0;
        limit_ = capacity_;
    }

    void FloatBuffer::flip()
    {
        limit_ = position_;
        position_ = 0;
    }

    void FloatBuffer::rewind()
    {
        position_ = 0;
    }

    void FloatBuffer::compact()
    {
        const auto count = limit_ - position_;
        if (count > 0 && position_ > 0)
        {
            std::move(buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(limit_), buffer_.begin());
        }
        position_ = count;
        limit_ = capacity_;
    }

    void FloatBuffer::put(const float value)
    {
        if (!hasRemaining())
        {
            throw std::overflow_error("FloatBuffer::put: Buffer overflow");
        }
        buffer_[position_++] = value;
    }

    void FloatBuffer::put(const std::vector<float>& values)
    {
        if (values.empty())
        {
            return;
        }

        if (values.size() > remaining())
        {
            throw std::overflow_error("FloatBuffer::put: Buffer overflow");
        }
        std::ranges::copy(values, buffer_.begin() + static_cast<std::ptrdiff_t>(position_));
        position_ += values.size();
    }

    float FloatBuffer::get()
    {
        if (!hasRemaining())
        {
            throw std::underflow_error("FloatBuffer::get: Buffer underflow");
        }
        return buffer_[position_++];
    }

    std::vector<float> FloatBuffer::get(const size_t length)
    {
        if (length == 0)
        {
            return {};
        }

        if (position_ + length > limit_)
        {
            throw std::underflow_error("FloatBuffer::get: Buffer underflow");
        }
        std::vector result(buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(position_ + length));
        position_ += length;
        return result;
    }

    std::vector<float> FloatBuffer::getRemaining() const
    {
        if (position_ >= limit_)
        {
            return {};
        }
        return {buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(limit_)};
    }

    size_t FloatBuffer::position() const
    {
        return position_;
    }

    void FloatBuffer::position(const size_t newPosition)
    {
        if (newPosition > limit_)
        {
            throw std::out_of_range("FloatBuffer::position: Position exceeds limit.");
        }
        position_ = newPosition;
    }

    size_t FloatBuffer::limit() const
    {
        return limit_;
    }

    void FloatBuffer::limit(const size_t newLimit)
    {
        if (newLimit > capacity_)
        {
            throw std::out_of_range("FloatBuffer::limit: Limit exceeds capacity.");
        }
        if (position_ > newLimit)
        {
            position_ = newLimit;
        }
        limit_ = newLimit;
    }

    size_t FloatBuffer::capacity() const
    {
        return capacity_;
    }

    bool FloatBuffer::hasRemaining() const
    {
        return position_ < limit_;
    }

    size_t FloatBuffer::remaining() const
    {
        return limit_ - position_;
    }
}
