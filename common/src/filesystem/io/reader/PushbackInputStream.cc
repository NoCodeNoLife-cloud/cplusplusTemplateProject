#include "src/filesystem/io/reader/PushbackInputStream.hpp"

#include <fmt/format.h>
#include <cstddef>
#include <stdexcept>
#include <algorithm>

namespace common::filesystem {
    PushbackInputStream::PushbackInputStream(std::unique_ptr<AbstractInputStream> inputStream, const size_t bufferSize) : FilterInputStream(std::move(inputStream)), pushback_buffer_(bufferSize), buffer_pos_(bufferSize) {
    }

    auto PushbackInputStream::available() -> size_t {
        if (!input_stream_) {
            throw std::runtime_error("PushbackInputStream::available: Input stream is not available");
        }
        return pushback_buffer_.size() - buffer_pos_ + input_stream_->available();
    }

    auto PushbackInputStream::read() -> std::byte {
        if (!input_stream_) {
throw std::runtime_error("PushbackInputStream::read: Input stream is not available");
        }

        if (buffer_pos_ < pushback_buffer_.size()) {
return pushback_buffer_[buffer_pos_++];
        }
        return input_stream_->read();
    }

    auto PushbackInputStream::read(std::vector<std::byte> &buffer) -> size_t {
        if (!input_stream_) {
            throw std::runtime_error("PushbackInputStream::read: Input stream is not available");
        }
        return read(buffer, 0, buffer.size());
    }

    auto PushbackInputStream::read(std::vector<std::byte> &buffer, const size_t offset, const size_t len) -> size_t {
        if (!input_stream_) {
            throw std::runtime_error("PushbackInputStream::read: Input stream is not available");
        }

        if (offset >= buffer.size() || len > buffer.size() - offset) {
            throw std::out_of_range("PushbackInputStream::read: Buffer overflow");
        }

        size_t bytesRead = 0;
        while (bytesRead < len && buffer_pos_ < pushback_buffer_.size()) {
            buffer[offset + bytesRead++] = pushback_buffer_[buffer_pos_++];
        }
        if (bytesRead < len) {
            bytesRead += input_stream_->read(buffer, offset + bytesRead, len - bytesRead);
        }
        return bytesRead;
    }

    void PushbackInputStream::unread(const std::vector<std::byte> &buffer) {
        unread(buffer, 0, buffer.size());
    }

    void PushbackInputStream::unread(const std::vector<std::byte> &buffer, const size_t offset, const size_t len) {
        if (offset >= buffer.size() || len > buffer.size() - offset) {
throw std::out_of_range("PushbackInputStream::unread: Buffer offset/length out of range");
        }

        if (len > buffer_pos_) {
throw std::overflow_error("PushbackInputStream::unread: Pushback buffer overflow");
        }

        for (size_t i = 0; i < len; ++i) {
            pushback_buffer_[--buffer_pos_] = buffer[offset + len - i - 1];
        }
}

    void PushbackInputStream::unread(const std::byte b) {
        if (buffer_pos_ == 0) {
throw std::overflow_error("PushbackInputStream::unread: Pushback buffer overflow");
        }
        pushback_buffer_[--buffer_pos_] = b;
}

    auto PushbackInputStream::isClosed() const noexcept -> bool {
        return !input_stream_ || input_stream_->isClosed();
    }
}
