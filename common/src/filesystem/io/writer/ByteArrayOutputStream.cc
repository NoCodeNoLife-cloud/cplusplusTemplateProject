#include "src/filesystem/io/writer/ByteArrayOutputStream.hpp"


#include <fmt/format.h>
#include <stdexcept>

namespace common::filesystem {
    ByteArrayOutputStream::ByteArrayOutputStream() : buf_(32) {
    }

    ByteArrayOutputStream::ByteArrayOutputStream(const size_t size) : buf_(size) {
        if (size == 0) {
            throw std::invalid_argument("Size must be greater than zero");
        }
    }

    auto ByteArrayOutputStream::write(const std::byte b) -> void {
        ensureCapacity(1);
        buf_[count_++] = b;
    }

    auto ByteArrayOutputStream::write(const std::vector<std::byte> &buffer, const size_t offset, const size_t len) -> void {
        if (len == 0) {
            return;
        }

        if (offset + len > buffer.size()) {
            throw std::out_of_range("Buffer offset/length out of range");
        }

        ensureCapacity(len);
        std::copy_n(buffer.begin() + static_cast<std::vector<std::byte>::difference_type>(offset), len, buf_.begin() + static_cast<std::vector<std::byte>::difference_type>(count_));
        count_ += len;
    }

    auto ByteArrayOutputStream::write(const std::byte *buffer, const size_t length) -> void {
        if (length == 0) {
            return;
        }

        if (!buffer) {
            throw std::invalid_argument("Buffer cannot be null");
        }

        ensureCapacity(length);
        std::copy_n(buffer, length, buf_.begin() + static_cast<std::vector<std::byte>::difference_type>(count_));
        count_ += length;
    }

    auto ByteArrayOutputStream::writeTo(AbstractOutputStream &out) const -> void {
        if (count_ > 0) {
            out.write(buf_, 0, count_);
        }
    }

    auto ByteArrayOutputStream::reset() -> void {
        count_ = 0;
    }

    auto ByteArrayOutputStream::toByteArray() const -> std::vector<std::byte> {
        return {buf_.begin(), buf_.begin() + static_cast<std::vector<std::byte>::difference_type>(count_)};
    }

    auto ByteArrayOutputStream::size() const -> size_t {
        return count_;
    }

    auto ByteArrayOutputStream::toString() const -> std::string {
        return {reinterpret_cast<const char *>(buf_.data()), count_};
    }

    auto ByteArrayOutputStream::close() -> void {
        // No operation for ByteArrayOutputStream.
    }

    auto ByteArrayOutputStream::flush() -> void {
        // No operation for ByteArrayOutputStream.
    }

    auto ByteArrayOutputStream::capacity() const -> size_t {
        return buf_.size();
    }

    auto ByteArrayOutputStream::isClosed() const -> bool {
        return false; // ByteArrayOutputStream is never closed
    }

    auto ByteArrayOutputStream::ensureCapacity(const size_t additionalCapacity) -> void {
        if (count_ + additionalCapacity > buf_.size()) {
            const size_t minCapacity = count_ + additionalCapacity;
            const size_t oldCapacity = buf_.size();
            size_t newCapacity = buf_.size() * 2;
            if (newCapacity < minCapacity) {
                newCapacity = minCapacity;
            }
            buf_.resize(newCapacity);
        }
    }
}
