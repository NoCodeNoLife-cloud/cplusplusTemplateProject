/**
 * @file DoubleBuffer.cc
 * @brief DoubleBuffer implementation â€?get/put/flip/compact/array operations
 * @details Implements double buffer operations: absolute and relative get/put,
 *          bulk array transfer, and buffer state management (flip/clear/compact).
 */

#include "buffer/DoubleBuffer.hpp"

#include <algorithm>

namespace cppforge::buffer
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
        const auto count = limit_ - position_;
        if (count > 0 && position_ > 0)
        {
            std::move(buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(limit_), buffer_.begin());
        }
        position_ = count;
        limit_ = capacity_;
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

        std::ranges::copy(values, buffer_.begin() + static_cast<std::ptrdiff_t>(position_));
        position_ += values.size();
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
