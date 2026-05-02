#include "src/filesystem/nio/IntBuffer.hpp"

#include <fmt/format.h>
#include <algorithm>

namespace common::filesystem {
    IntBuffer::IntBuffer(const size_t capacity) {
        capacity_ = capacity;
        limit_ = capacity;
        position_ = 0;
        buffer_.resize(capacity);
    }

    auto IntBuffer::clear() noexcept -> void {
        position_ = 0;
        limit_ = capacity_;
    }

    auto IntBuffer::flip() noexcept -> void {
        limit_ = position_;
        position_ = 0;
    }

    auto IntBuffer::rewind() noexcept -> void {
        position_ = 0;
    }

    auto IntBuffer::compact() -> void {
        if (position_ > 0) {
std::move(buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(limit_), buffer_.begin());
            limit_ -= position_;
            position_ = 0;
        }
    }

    auto IntBuffer::get() -> int32_t {
        if (!hasRemaining()) {
throw std::underflow_error("IntBuffer::get: Buffer underflow.");
        }
        return buffer_[position_++];
    }

    auto IntBuffer::get(const size_t index) const -> int32_t {
        if (index >= limit_) {
throw std::out_of_range("IntBuffer::get: Index out of bounds.");
        }
        return buffer_[index];
    }

    auto IntBuffer::put(const int32_t value) -> void {
        if (!hasRemaining()) {
throw std::overflow_error("IntBuffer::put: Buffer overflow.");
        }
        buffer_[position_++] = value;
    }

    auto IntBuffer::put(const size_t index, const int32_t value) -> void {
        if (index >= limit_) {
throw std::out_of_range("IntBuffer::put: Index out of bounds.");
        }
        buffer_[index] = value;
    }

    auto IntBuffer::getRemaining() const -> std::vector<int32_t> {
        if (position_ >= limit_) {
            return {};
        }
        return {buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(limit_)};
    }

    auto IntBuffer::position() const noexcept -> size_t {
        return position_;
    }

    auto IntBuffer::position(const size_t newPosition) -> void {
        if (newPosition > limit_) {
throw std::out_of_range("IntBuffer::position: Position exceeds limit.");
        }
position_ = newPosition;
    }

    auto IntBuffer::limit() const noexcept -> size_t {
        return limit_;
    }

    auto IntBuffer::limit(const size_t newLimit) -> void {
        if (newLimit > capacity_) {
throw std::out_of_range("IntBuffer::limit: Limit exceeds capacity.");
        }
if (position_ > newLimit) {
            position_ = newLimit;
        }
        limit_ = newLimit;
    }

    auto IntBuffer::capacity() const noexcept -> size_t {
        return capacity_;
    }

    auto IntBuffer::hasRemaining() const noexcept -> bool {
        return position_ < limit_;
    }

    auto IntBuffer::remaining() const noexcept -> size_t {
        return limit_ - position_;
    }
}
