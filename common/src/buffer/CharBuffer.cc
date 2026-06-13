/**
 * @file CharBuffer.cc
 * @brief CharBuffer implementation — get/put/flip/compact/array operations
 * @details Implements char buffer operations: absolute and relative get/put,
 *          bulk array transfer, flip/clear/rewind/compact, and state queries.
 */

#include "buffer/CharBuffer.hpp"

#include <algorithm>
#include <stdexcept>

namespace common::buffer
{
    CharBuffer::CharBuffer(const size_t cap) : buffer_(cap, '\0')
    {
        position_ = 0;
        limit_ = cap;
        capacity_ = cap;
    }

    void CharBuffer::clear()
    {
        position_ = 0;
        limit_ = capacity_;
    }

    void CharBuffer::flip()
    {
        limit_ = position_;
        position_ = 0;
    }

    void CharBuffer::rewind()
    {
        position_ = 0;
    }

    void CharBuffer::compact()
    {
        const auto count = limit_ - position_;
        if (count > 0 && position_ > 0)
        {
            std::move(buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(limit_), buffer_.begin());
        }
        position_ = count;
        limit_ = capacity_;
    }

    void CharBuffer::put(const char c)
    {
        if (!hasRemaining())
        {
            throw std::overflow_error("CharBuffer::put: Buffer overflow.");
        }
        buffer_[position_++] = c;
    }

    void CharBuffer::put(const std::string& src)
    {
        if (src.empty())
        {
            return;
        }

        if (position_ + src.size() > limit_)
        {
            throw std::overflow_error("CharBuffer::put: Buffer overflow.");
        }
        std::ranges::copy(src, buffer_.begin() + static_cast<std::ptrdiff_t>(position_));
        position_ += src.size();
    }

    char CharBuffer::get()
    {
        if (!hasRemaining())
        {
            throw std::underflow_error("CharBuffer::get: Buffer underflow.");
        }
        return buffer_[position_++];
    }

    std::string CharBuffer::getRemaining() const
    {
        if (position_ >= limit_)
        {
            return {};
        }
        return {buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(limit_)};
    }

    size_t CharBuffer::position() const
    {
        return position_;
    }

    void CharBuffer::position(const size_t newPosition)
    {
        if (newPosition > limit_)
        {
            throw std::out_of_range("CharBuffer::position: Position exceeds limit.");
        }
        position_ = newPosition;
    }

    size_t CharBuffer::limit() const
    {
        return limit_;
    }

    void CharBuffer::limit(const size_t newLimit)
    {
        if (newLimit > capacity_)
        {
            throw std::out_of_range("CharBuffer::limit: Limit exceeds capacity.");
        }
        if (position_ > newLimit)
        {
            position_ = newLimit;
        }
        limit_ = newLimit;
    }

    size_t CharBuffer::capacity() const
    {
        return capacity_;
    }

    bool CharBuffer::hasRemaining() const
    {
        return position_ < limit_;
    }

    size_t CharBuffer::remaining() const
    {
        return limit_ - position_;
    }
}
