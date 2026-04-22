#include "src/filesystem/io/reader/CharArrayReader.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <algorithm>
#include <stdexcept>
#include <iterator>

namespace common::filesystem {
    void CharArrayReader::validateConstructorParams(const size_t buffer_size, const size_t offset, const size_t length) {
        if (offset > buffer_size || offset + length > buffer_size) {
            DLOG(ERROR) << fmt::format("CharArrayReader validation failed - invalid params: buffer_size={}, offset={}, length={}", buffer_size, offset, length);
            throw std::invalid_argument("CharArrayReader::CharArrayReader: Invalid offset or length");
        }
    }

    void CharArrayReader::validateTargetBufferParams(const size_t target_buffer_size, const size_t offset, const size_t length) {
        if (offset > target_buffer_size || length > target_buffer_size - offset) {
            throw std::out_of_range("CharArrayReader::read: Invalid offset or length for target buffer");
        }
    }

    CharArrayReader::CharArrayReader(const std::vector<char> &buffer) : buf_(buffer), count_(buffer.size()) {
        DLOG(INFO) << fmt::format("CharArrayReader initialized with {} characters", buffer.size());
    }

    CharArrayReader::CharArrayReader(const std::vector<char> &buffer, const size_t offset, const size_t length) {
        validateConstructorParams(buffer.size(), offset, length);
        buf_.assign(buffer.begin() + static_cast<std::ptrdiff_t>(offset), buffer.begin() + static_cast<std::ptrdiff_t>(offset + length));
        count_ = length;
    }

    CharArrayReader::~CharArrayReader() = default;

    auto CharArrayReader::read() -> int {
        if (closed_ || pos_ >= count_) {
            DLOG(INFO) << "CharArrayReader read - stream closed or end of buffer";
            return -1;
        }
        return static_cast<unsigned char>(buf_[pos_++]);
    }

    auto CharArrayReader::read(std::vector<char> &b, const size_t off, const size_t len) -> int {
        validateTargetBufferParams(b.size(), off, len);

        if (closed_ || pos_ >= count_) {
            DLOG(INFO) << "CharArrayReader read - stream closed or end of buffer";
            return -1;
        }

        const size_t toRead = std::min(len, count_ - pos_);
        std::copy_n(buf_.begin() + static_cast<std::ptrdiff_t>(pos_), toRead, b.begin() + static_cast<std::ptrdiff_t>(off));
        pos_ += toRead;
        DLOG(INFO) << fmt::format("CharArrayReader read - characters read: {}", toRead);
        return static_cast<int>(toRead);
    }

    auto CharArrayReader::skip(const size_t n) -> size_t {
        if (closed_) return 0;

        const size_t skipped = std::min(n, count_ - pos_);
        pos_ += skipped;
        return skipped;
    }

    auto CharArrayReader::ready() const noexcept -> bool {
        return !closed_ && pos_ < count_;
    }

    auto CharArrayReader::markSupported() const noexcept -> bool {
        return true;
    }

    auto CharArrayReader::mark(const size_t readAheadLimit) -> void {
        if (closed_) {
            DLOG(ERROR) << "CharArrayReader mark failed - stream is closed";
            throw std::runtime_error("CharArrayReader::mark: Stream is closed");
        }
        DLOG(INFO) << fmt::format("CharArrayReader mark - position marked at {}", pos_);
        marked_pos_ = pos_;
    }

    auto CharArrayReader::reset() -> void {
        if (closed_) {
            DLOG(ERROR) << "CharArrayReader reset failed - stream is closed";
            throw std::runtime_error("CharArrayReader::reset: Stream is closed");
        }
        DLOG(INFO) << fmt::format("CharArrayReader reset - position reset to {}", marked_pos_);
        pos_ = marked_pos_;
    }

    auto CharArrayReader::close() -> void {
        DLOG(INFO) << "CharArrayReader closing stream";
        closed_ = true;
        buf_.clear();
        pos_ = 0;
        marked_pos_ = 0;
        count_ = 0;
    }

    auto CharArrayReader::isClosed() const noexcept -> bool {
        return closed_;
    }
}
