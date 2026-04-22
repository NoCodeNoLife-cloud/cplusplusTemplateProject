#include "src/filesystem/nio/CharBuffer.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <algorithm>
#include <stdexcept>

namespace common::filesystem {
    CharBuffer::CharBuffer(const size_t cap) : buffer_(cap, '\0') {
        position_ = 0;
        limit_ = cap;
        capacity_ = cap;
    }

    auto CharBuffer::clear() noexcept -> void {
        position_ = 0;
        limit_ = capacity_;
    }

    auto CharBuffer::flip() noexcept -> void {
        limit_ = position_;
        position_ = 0;
    }

    auto CharBuffer::rewind() noexcept -> void {
        position_ = 0;
    }

    auto CharBuffer::compact() -> void {
        if (position_ > 0) {
            DLOG(INFO) << fmt::format("CharBuffer compact - moving {} characters, new limit: {}", limit_ - position_, limit_ - position_);
            std::move(buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(limit_), buffer_.begin());
            limit_ -= position_;
            position_ = 0;
        }
    }

    auto CharBuffer::put(const char c) -> void {
        if (!hasRemaining()) {
            DLOG(ERROR) << fmt::format("CharBuffer put failed - buffer overflow: position={}, limit={}", position_, limit_);
            throw std::overflow_error("CharBuffer::put: Buffer overflow.");
        }
        buffer_[position_++] = c;
    }

    auto CharBuffer::put(const std::string &src) -> void {
        if (src.empty()) {
            return;
        }

        if (position_ + src.size() > limit_) {
            DLOG(ERROR) << fmt::format("CharBuffer put failed - buffer overflow: position={}, src_size={}, limit={}", position_, src.size(), limit_);
            throw std::overflow_error("CharBuffer::put: Buffer overflow.");
        }
        DLOG(INFO) << fmt::format("CharBuffer put - writing {} characters", src.size());
        std::ranges::copy(src, buffer_.begin() + static_cast<std::ptrdiff_t>(position_));
        position_ += src.size();
    }

    auto CharBuffer::get() -> char {
        if (!hasRemaining()) {
            DLOG(ERROR) << fmt::format("CharBuffer get failed - buffer underflow: position={}, limit={}", position_, limit_);
            throw std::underflow_error("CharBuffer::get: Buffer underflow.");
        }
        return buffer_[position_++];
    }

    auto CharBuffer::getRemaining() const -> std::string {
        if (position_ >= limit_) {
            return {};
        }
        return {buffer_.begin() + static_cast<std::ptrdiff_t>(position_), buffer_.begin() + static_cast<std::ptrdiff_t>(limit_)};
    }

    auto CharBuffer::position() const noexcept -> size_t {
        return position_;
    }

    auto CharBuffer::position(const size_t newPosition) -> void {
        if (newPosition > limit_) {
            DLOG(ERROR) << fmt::format("CharBuffer position failed - position exceeds limit: newPosition={}, limit={}", newPosition, limit_);
            throw std::out_of_range("CharBuffer::position: Position exceeds limit.");
        }
        DLOG(INFO) << fmt::format("CharBuffer position set to {}", newPosition);
        position_ = newPosition;
    }

    auto CharBuffer::limit() const noexcept -> size_t {
        return limit_;
    }

    auto CharBuffer::limit(const size_t newLimit) -> void {
        if (newLimit > capacity_) {
            DLOG(ERROR) << fmt::format("CharBuffer limit failed - limit exceeds capacity: newLimit={}, capacity={}", newLimit, capacity_);
            throw std::out_of_range("CharBuffer::limit: Limit exceeds capacity.");
        }
        DLOG(INFO) << fmt::format("CharBuffer limit set to {} (was {})", newLimit, limit_);
        if (position_ > newLimit) {
            position_ = newLimit;
        }
        limit_ = newLimit;
    }

    auto CharBuffer::capacity() const noexcept -> size_t {
        return capacity_;
    }

    auto CharBuffer::hasRemaining() const noexcept -> bool {
        return position_ < limit_;
    }

    auto CharBuffer::remaining() const noexcept -> size_t {
        return limit_ - position_;
    }
}
