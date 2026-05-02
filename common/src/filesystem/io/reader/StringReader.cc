#include "src/filesystem/io/reader/StringReader.hpp"

#include <fmt/format.h>
#include <algorithm>
#include <stdexcept>

namespace common::filesystem {
    StringReader::StringReader(std::string s) : source_(std::move(s)), position_(0), mark_position_(0), mark_set_(false), closed_(false) {
    }

    auto StringReader::close() noexcept -> void {
        closed_ = true;
        source_.clear();
        position_ = 0;
        mark_position_ = 0;
        mark_set_ = false;
    }

    auto StringReader::mark(const size_t readAheadLimit) -> void {
        if (closed_) {
            throw std::runtime_error("StringReader::mark: Stream is closed");
        }
        mark_position_ = position_;
        mark_set_ = true;
        static_cast<void>(readAheadLimit); // Unused parameter
    }

    auto StringReader::markSupported() const noexcept -> bool {
        return true;
    }

    auto StringReader::read() -> int {
        if (closed_ || position_ >= source_.size()) {
            return -1; // EOF
        }
        return static_cast<unsigned char>(source_[position_++]);
    }

    auto StringReader::read(std::vector<char> &cBuf, const size_t off, const size_t len) -> int {
        if (closed_) {
            throw std::runtime_error("StringReader::read: Stream is closed");
        }

        if (off > cBuf.size() || len > cBuf.size() - off) {
            throw std::invalid_argument("StringReader::read: Offset is out of bounds of the buffer");
        }

        if (position_ >= source_.size()) {
            return -1; // EOF
        }

        const size_t maxRead = std::min(len, source_.size() - position_);
        const size_t actualRead = std::min(maxRead, cBuf.size() - off);

        for (size_t i = 0; i < actualRead; ++i) {
            if (position_ < source_.size()) {
                cBuf[off + i] = source_[position_++];
            } else {
                break;
            }
        }
        return static_cast<int>(actualRead > 0 ? actualRead : -1);
    }

    auto StringReader::ready() const noexcept -> bool {
        return !closed_ && position_ < source_.size();
    }

    auto StringReader::reset() -> void {
        if (closed_) {
            throw std::runtime_error("StringReader::reset: Stream is closed");
        }
        if (!mark_set_) {
            position_ = 0;
        } else {
            position_ = mark_position_;
        }
    }

    auto StringReader::skip(const size_t ns) noexcept -> size_t {
        if (closed_) {
            return 0;
        }
        const size_t charsToSkip = std::min(ns, source_.size() - position_);
        position_ += charsToSkip;
        return charsToSkip;
    }

    auto StringReader::isClosed() const noexcept -> bool {
        return closed_;
    }
}
