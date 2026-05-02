#include "src/filesystem/io/reader/PushbackReader.hpp"


#include <fmt/format.h>
#include <algorithm>
#include <stdexcept>

namespace common::filesystem {
    auto PushbackReader::validateOpen() const -> void {
        if (closed_ || !in_) {
            throw std::runtime_error("PushbackReader::operation: Stream is closed");
        }
    }

    auto PushbackReader::validateNotClosed() const -> void {
        if (closed_) {
            throw std::runtime_error("PushbackReader::operation: Stream is closed");
        }
    }

    PushbackReader::PushbackReader(std::shared_ptr<AbstractReader> reader) : PushbackReader(std::move(reader), DEFAULT_BUFFER_SIZE) {
    }

    PushbackReader::PushbackReader(std::shared_ptr<AbstractReader> reader, const size_t size) : FilterReader(std::move(reader)), buffer_(size) {
        if (size == 0) {
            throw std::invalid_argument("PushbackReader::constructor: Buffer size must be greater than zero.");
        }
    }

    auto PushbackReader::close() noexcept -> void {
        closed_ = true;
        FilterReader::close();
        buffer_.clear();
    }

    auto PushbackReader::mark(const size_t readAheadLimit) -> void {
        static_cast<void>(readAheadLimit); // Unused parameter
        throw std::runtime_error("PushbackReader::mark: mark() not supported.");
    }

    auto PushbackReader::markSupported() const -> bool {
        return false;
    }

    auto PushbackReader::read() -> int {
        validateOpen();

        if (buffer_pos_ < buffer_.size()) {
            return static_cast<unsigned char>(buffer_[buffer_pos_++]);
        }
        return FilterReader::read();
    }

    auto PushbackReader::read(std::vector<char> &cBuf, const size_t off, const size_t len) -> int {
        validateOpen();

        if (off > cBuf.size() || len > cBuf.size() - off) {
            throw std::out_of_range("PushbackReader::read: Buffer overflow.");
        }

        size_t bytesRead = 0;
        while (buffer_pos_ < buffer_.size() && bytesRead < len) {
            cBuf[off + bytesRead] = buffer_[buffer_pos_++];
            bytesRead++;
        }
        if (bytesRead < len) {
            const int underlyingBytesRead = FilterReader::read(cBuf, off + bytesRead, len - bytesRead);
            if (underlyingBytesRead == -1) {
                return bytesRead > 0 ? static_cast<int>(bytesRead) : -1;
            }
            bytesRead += static_cast<size_t>(underlyingBytesRead);
        }
        return static_cast<int>(bytesRead);
    }

    auto PushbackReader::ready() const -> bool {
        if (closed_ || !in_) {
            return false;
        }
        return buffer_pos_ < buffer_.size() || FilterReader::ready();
    }

    auto PushbackReader::reset() -> void {
        throw std::runtime_error("PushbackReader::reset: reset() not supported.");
    }

    auto PushbackReader::skip(const size_t n) -> size_t {
        validateOpen();

        size_t skipped = 0;
        if (buffer_pos_ < buffer_.size()) {
            const size_t bufferRemaining = buffer_.size() - buffer_pos_;
            skipped = std::min(n, bufferRemaining);
            buffer_pos_ += skipped;
        }
        if (n > skipped) {
            skipped += FilterReader::skip(n - skipped);
        }
        return skipped;
    }

    auto PushbackReader::unread(const std::vector<char> &cbuf) noexcept -> void {
        unread(cbuf, 0, cbuf.size());
    }

    auto PushbackReader::unread(const std::vector<char> &cBuf, const size_t off, const size_t len) -> void {
        validateNotClosed();

        if (off > cBuf.size() || len > cBuf.size() - off) {
            throw std::out_of_range("PushbackReader::unread: Buffer offset/length out of range");
        }

        if (len > buffer_pos_) {
            throw std::overflow_error("PushbackReader::unread: Pushback buffer overflow.");
        }

        for (size_t i = 0; i < len; ++i) {
            buffer_[--buffer_pos_] = cBuf[off + len - 1 - i];
        }
    }

    auto PushbackReader::unread(const int32_t c) -> void {
        validateNotClosed();

        if (buffer_pos_ == 0) {
            throw std::overflow_error("PushbackReader::unread: Pushback buffer overflow.");
        }
        buffer_[--buffer_pos_] = static_cast<char>(c);
    }

    auto PushbackReader::isClosed() const -> bool {
        return closed_ || !in_ || in_->isClosed();
    }
}
