#include "src/filesystem/nio/LongBuffer.hpp"

#include <glog/logging.h>
#include <fmt/format.h>

namespace common::filesystem {
    LongBuffer::LongBuffer(const std::size_t capacity) : capacity_(capacity), limit_(capacity) {
        buffer_.resize(capacity);
    }

    auto LongBuffer::get() -> int64_t {
        if (!hasRemaining()) {
            DLOG(ERROR) << fmt::format("LongBuffer get failed - no remaining elements: position={}, limit={}", position_, limit_);
            throw std::out_of_range("LongBuffer::get: No remaining elements to get");
        }
        return buffer_[position_++];
    }

    auto LongBuffer::put(const int64_t value) -> void {
        if (!hasRemaining()) {
            DLOG(ERROR) << fmt::format("LongBuffer put failed - no remaining space: position={}, limit={}", position_, limit_);
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
            DLOG(ERROR) << fmt::format("LongBuffer position failed - position out of range: newPosition={}, limit={}", newPosition, limit_);
            throw std::out_of_range("LongBuffer::position: Position out of range");
        }
        DLOG(INFO) << fmt::format("LongBuffer position set to {}", newPosition);
        position_ = newPosition;
    }

    auto LongBuffer::limit() const noexcept -> std::size_t {
        return limit_;
    }

    auto LongBuffer::limit(const std::size_t newLimit) -> void {
        if (newLimit > capacity_) {
            DLOG(ERROR) << fmt::format("LongBuffer limit failed - limit exceeds capacity: newLimit={}, capacity={}", newLimit, capacity_);
            throw std::out_of_range("LongBuffer::limit: Limit exceeds capacity");
        }
        DLOG(INFO) << fmt::format("LongBuffer limit set to {} (was {})", newLimit, limit_);
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
        DLOG(INFO) << fmt::format("LongBuffer flip - limit set to {}, position reset to 0", position_);
        limit_ = position_;
        position_ = 0;
    }

    auto LongBuffer::rewind() noexcept -> void {
        position_ = 0;
    }
}
