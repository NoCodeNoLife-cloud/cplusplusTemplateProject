#include "src/filesystem/nio/LongBuffer.hpp"

#include <fmt/format.h>

namespace common::filesystem {
    LongBuffer::LongBuffer(const std::size_t capacity) : capacity_(capacity), limit_(capacity) {
        buffer_.resize(capacity);
    }

    auto LongBuffer::get() -> int64_t {
        if (!hasRemaining()) {
throw std::out_of_range("LongBuffer::get: No remaining elements to get");
        }
        return buffer_[position_++];
    }

    auto LongBuffer::put(const int64_t value) -> void {
        if (!hasRemaining()) {
throw std::out_of_range("LongBuffer::put: No remaining space to put");
        }
        buffer_[position_++] = value;
    }

    auto LongBuffer::hasRemaining() const noexcept -> bool {
        return position_ < limit_;
    }

    auto LongBuffer::remaining() const noexcept -> std::size_t {
        return limit_ - position_;
    }

    auto LongBuffer::position() const noexcept -> std::size_t {
        return position_;
    }

    auto LongBuffer::position(const std::size_t newPosition) -> void {
        if (newPosition > limit_) {
throw std::out_of_range("LongBuffer::position: Position out of range");
        }
position_ = newPosition;
    }

    auto LongBuffer::limit() const noexcept -> std::size_t {
        return limit_;
    }

    auto LongBuffer::limit(const std::size_t newLimit) -> void {
        if (newLimit > capacity_) {
throw std::out_of_range("LongBuffer::limit: Limit exceeds capacity");
        }
limit_ = newLimit;
        if (position_ > limit_) {
            position_ = limit_;
        }
    }

    auto LongBuffer::capacity() const noexcept -> std::size_t {
        return capacity_;
    }

    auto LongBuffer::clear() noexcept -> void {
        position_ = 0;
        limit_ = capacity_;
    }

    auto LongBuffer::flip() noexcept -> void {
limit_ = position_;
        position_ = 0;
    }

    auto LongBuffer::rewind() noexcept -> void {
        position_ = 0;
    }
}
