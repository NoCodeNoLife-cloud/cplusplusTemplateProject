#include "src/filesystem/nio/ShortBuffer.hpp"

#include <glog/logging.h>
#include <fmt/format.h>

namespace common::filesystem {
    ShortBuffer::ShortBuffer(const size_t capacity) : buffer_(capacity), capacity_(capacity), limit_(capacity) {
    }

    auto ShortBuffer::wrap(const int16_t *data, const size_t size) -> ShortBuffer {
        DLOG(INFO) << fmt::format("ShortBuffer wrap - wrapping {} shorts from data", size);
        ShortBuffer sb(size);
        if (data != nullptr && size > 0) {
            std::memcpy(sb.buffer_.data(), data, size * sizeof(int16_t));
        }
        return sb;
    }

    auto ShortBuffer::get() -> int16_t {
        if (!hasRemaining()) {
            DLOG(ERROR) << fmt::format("ShortBuffer get failed - no remaining elements: position={}, limit={}", position_, limit_);
            throw std::out_of_range("ShortBuffer::get: No remaining elements to get");
        }
        return buffer_[position_++];
    }

    auto ShortBuffer::get(const size_t index) const -> int16_t {
        if (index >= limit_) {
            DLOG(ERROR) << fmt::format("ShortBuffer get failed - index out of bounds: index={}, limit={}", index, limit_);
            throw std::out_of_range("ShortBuffer::get: Index out of bounds");
        }
        return buffer_[index];
    }

    auto ShortBuffer::put(const int16_t value) -> void {
        if (!hasRemaining()) {
            DLOG(ERROR) << fmt::format("ShortBuffer put failed - no remaining space: position={}, limit={}", position_, limit_);
            throw std::out_of_range("ShortBuffer::put: No remaining space to put");
        }
        buffer_[position_++] = value;
    }

    auto ShortBuffer::put(const size_t index, const int16_t value) -> void {
        if (index >= limit_) {
            DLOG(ERROR) << fmt::format("ShortBuffer put failed - index out of bounds: index={}, limit={}", index, limit_);
            throw std::out_of_range("ShortBuffer::put: Index out of bounds");
        }
        buffer_[index] = value;
    }

    auto ShortBuffer::hasRemaining() const noexcept -> bool {
        return position_ < limit_;
    }

    auto ShortBuffer::remaining() const noexcept -> size_t {
        return limit_ - position_;
    }

    auto ShortBuffer::position() const noexcept -> size_t {
        return position_;
    }

    auto ShortBuffer::position(const size_t newPosition) -> void {
        if (newPosition > limit_) {
            DLOG(ERROR) << fmt::format("ShortBuffer position failed - position out of range: newPosition={}, limit={}", newPosition, limit_);
            throw std::out_of_range("ShortBuffer::position: Position out of range");
        }
        DLOG(INFO) << fmt::format("ShortBuffer position set to {}", newPosition);
        position_ = newPosition;
    }

    auto ShortBuffer::limit() const noexcept -> size_t {
        return limit_;
    }

    auto ShortBuffer::limit(const size_t newLimit) -> void {
        if (newLimit > capacity_) {
            DLOG(ERROR) << fmt::format("ShortBuffer limit failed - limit exceeds capacity: newLimit={}, capacity={}", newLimit, capacity_);
            throw std::out_of_range("ShortBuffer::limit: Limit exceeds capacity");
        }
        DLOG(INFO) << fmt::format("ShortBuffer limit set to {} (was {})", newLimit, limit_);
        limit_ = newLimit;
        if (position_ > limit_) {
            position_ = limit_;
        }
    }

    auto ShortBuffer::capacity() const noexcept -> size_t {
        return capacity_;
    }

    auto ShortBuffer::clear() noexcept -> void {
        position_ = 0;
        limit_ = capacity_;
    }

    auto ShortBuffer::flip() noexcept -> void {
        DLOG(INFO) << fmt::format("ShortBuffer flip - limit set to {}, position reset to 0", position_);
        limit_ = position_;
        position_ = 0;
    }

    auto ShortBuffer::rewind() noexcept -> void {
        position_ = 0;
    }

    auto ShortBuffer::data() -> int16_t * {
        return buffer_.data();
    }

    auto ShortBuffer::data() const -> const int16_t * {
        return buffer_.data();
    }
}
