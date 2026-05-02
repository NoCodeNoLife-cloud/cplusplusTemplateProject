#include "src/filesystem/nio/ByteBuffer.hpp"

#include <fmt/format.h>

namespace common::filesystem {
    ByteBuffer::ByteBuffer(const size_t capacity) : buffer_(capacity) {
        capacity_ = capacity;
        limit_ = capacity;
    }

    auto ByteBuffer::capacity() const noexcept -> size_t {
        return capacity_;
    }

    auto ByteBuffer::position() const noexcept -> size_t {
        return position_;
    }

    auto ByteBuffer::position(const size_t newPosition) -> void {
        if (newPosition > limit_) {
            throw std::out_of_range("ByteBuffer::position: Position exceeds the current limit.");
        }
        position_ = newPosition;
    }

    auto ByteBuffer::limit() const noexcept -> size_t {
        return limit_;
    }

    auto ByteBuffer::limit(const size_t newLimit) -> void {
        if (newLimit > capacity_) {
            throw std::out_of_range("ByteBuffer::limit: New limit exceeds capacity.");
        }
        limit_ = newLimit;
        if (position_ > limit_) {
            position_ = limit_;
        }
    }

    auto ByteBuffer::clear() noexcept -> void {
        position_ = 0;
        limit_ = capacity_;
    }

    auto ByteBuffer::flip() noexcept -> void {
        limit_ = position_;
        position_ = 0;
    }

    auto ByteBuffer::rewind() noexcept -> void {
        position_ = 0;
    }

    auto ByteBuffer::remaining() const noexcept -> size_t {
        return limit_ - position_;
    }

    auto ByteBuffer::hasRemaining() const noexcept -> bool {
        return position_ < limit_;
    }

    auto ByteBuffer::put(const std::byte value) -> void {
        if (!hasRemaining()) {
            throw std::overflow_error("ByteBuffer::put: Buffer overflow");
        }
        buffer_[position_++] = value;
    }

    auto ByteBuffer::put(const std::vector<std::byte> &src) -> void {
        if (src.empty()) {
            return;
        }

        if (src.size() > remaining()) {
            throw std::overflow_error("ByteBuffer::put: Insufficient space in buffer");
        }

        std::memcpy(buffer_.data() + position_, src.data(), src.size());
        position_ += src.size();
    }

    auto ByteBuffer::get() -> std::byte {
        if (!hasRemaining()) {
            throw std::underflow_error("ByteBuffer::get: Buffer underflow");
        }
        return buffer_[position_++];
    }

    auto ByteBuffer::get(const size_t length) -> std::vector<std::byte> {
        if (length == 0) {
            return {};
        }

        if (length > remaining()) {
            throw std::underflow_error("ByteBuffer::get: Insufficient data in buffer");
        }

        std::vector result(buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(position_ + length));
        position_ += length;
        return result;
    }
}
