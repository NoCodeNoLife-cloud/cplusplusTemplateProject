#include "src/filesystem/nio/DoubleBuffer.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <algorithm>

namespace common::filesystem {
    DoubleBuffer::DoubleBuffer(const size_t capacity) {
        position_ = 0;
        limit_ = capacity;
        capacity_ = capacity;
        buffer_.resize(capacity);
    }

    auto DoubleBuffer::clear() noexcept -> void {
        position_ = 0;
        limit_ = capacity_;
    }

    auto DoubleBuffer::flip() noexcept -> void {
        limit_ = position_;
        position_ = 0;
    }

    auto DoubleBuffer::rewind() noexcept -> void {
        position_ = 0;
    }

    auto DoubleBuffer::compact() -> void {
        if (position_ > 0) {
            DLOG(INFO) << fmt::format("DoubleBuffer compact - moving {} doubles, new limit: {}", limit_ - position_, limit_ - position_);
            std::move(buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(limit_), buffer_.begin());
            limit_ -= position_;
            position_ = 0;
        }
    }

    auto DoubleBuffer::put(const double value) -> DoubleBuffer & {
        if (!hasRemaining()) {
            DLOG(ERROR) << fmt::format("DoubleBuffer put failed - buffer overflow: position={}, limit={}", position_, limit_);
            throw std::overflow_error("DoubleBuffer::put: Buffer overflow: Position exceeds limit.");
        }
        buffer_[position_++] = value;
        return *this;
    }

    auto DoubleBuffer::put(const std::vector<double> &values) -> DoubleBuffer & {
        if (values.empty()) {
            return *this;
        }

        if (values.size() > remaining()) {
            DLOG(ERROR) << fmt::format("DoubleBuffer put failed - insufficient space: values_size={}, remaining={}", values.size(), remaining());
            throw std::overflow_error("DoubleBuffer::put: Buffer overflow: Not enough space for all values.");
        }

        DLOG(INFO) << fmt::format("DoubleBuffer put - writing {} doubles", values.size());
        for (const double value: values) {
            buffer_[position_++] = value;
        }
        return *this;
    }

    auto DoubleBuffer::get() -> double {
        if (!hasRemaining()) {
            DLOG(ERROR) << fmt::format("DoubleBuffer get failed - buffer underflow: position={}, limit={}", position_, limit_);
            throw std::underflow_error("DoubleBuffer::get: Buffer underflow: Position exceeds limit.");
        }
        return buffer_[position_++];
    }

    auto DoubleBuffer::position() const noexcept -> size_t {
        return position_;
    }

    auto DoubleBuffer::position(const size_t newPosition) -> void {
        if (newPosition > limit_) {
            DLOG(ERROR) << fmt::format("DoubleBuffer position failed - position exceeds limit: newPosition={}, limit={}", newPosition, limit_);
            throw std::out_of_range("DoubleBuffer::position: Position exceeds limit.");
        }
        DLOG(INFO) << fmt::format("DoubleBuffer position set to {}", newPosition);
        position_ = newPosition;
    }

    auto DoubleBuffer::limit() const noexcept -> size_t {
        return limit_;
    }

    auto DoubleBuffer::limit(const size_t newLimit) -> void {
        if (newLimit > capacity_) {
            DLOG(ERROR) << fmt::format("DoubleBuffer limit failed - limit exceeds capacity: newLimit={}, capacity={}", newLimit, capacity_);
            throw std::out_of_range("DoubleBuffer::limit: Limit exceeds capacity.");
        }
        DLOG(INFO) << fmt::format("DoubleBuffer limit set to {} (was {})", newLimit, limit_);
        if (position_ > newLimit) {
            position_ = newLimit;
        }
        limit_ = newLimit;
    }

    auto DoubleBuffer::capacity() const noexcept -> size_t {
        return capacity_;
    }

    auto DoubleBuffer::hasRemaining() const noexcept -> bool {
        return position_ < limit_;
    }

    auto DoubleBuffer::remaining() const noexcept -> size_t {
        return limit_ - position_;
    }

    auto DoubleBuffer::getRemaining() const -> std::vector<double> {
        if (position_ >= limit_) {
            return {};
        }
        return {buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(limit_)};
    }
}
