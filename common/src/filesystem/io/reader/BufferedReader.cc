#include "src/filesystem/io/reader/BufferedReader.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace common::filesystem {
    BufferedReader::BufferedReader(std::unique_ptr<AbstractReader> reader, const size_t size) : reader_(std::move(reader)), buffer_size_(size) {
        if (size == 0) {
            DLOG(ERROR) << "BufferedReader initialization failed - buffer size must be greater than 0";
            throw std::invalid_argument("Buffer size must be greater than 0");
        }
        buffer_.resize(size);
        DLOG(INFO) << fmt::format("BufferedReader initialized with buffer size: {}", size);
    }

    auto BufferedReader::close() -> void {
        reader_->close();
    }

    auto BufferedReader::mark(const size_t readAheadLimit) -> void {
        if (readAheadLimit == 0) {
            throw std::invalid_argument("Mark limit must be greater than 0");
        }
        reader_->mark(readAheadLimit);
        mark_limit_ = pos_;
    }

    auto BufferedReader::markSupported() const -> bool {
        return true;
    }

    auto BufferedReader::reset() -> void {
        reader_->reset();
        pos_ = mark_limit_;
    }

    auto BufferedReader::read() -> int {
        if (pos_ >= count_) {
            if (!fillBuffer()) {
                DLOG(INFO) << "BufferedReader read - end of stream reached";
                return -1;
            }
        }
        return static_cast<unsigned char>(buffer_[pos_++]);
    }

    auto BufferedReader::read(std::vector<char> &cBuf, const size_t off, const size_t len) -> int {
        if (off > cBuf.size() || len > cBuf.size() - off) {
            throw std::out_of_range("Buffer offset/length out of range");
        }

        if (len == 0) {
            return 0;
        }

        size_t totalBytesRead = 0;
        size_t currentOffset = off;
        size_t remainingLen = len;

        while (remainingLen > 0) {
            if (pos_ >= count_) {
                if (!fillBuffer()) {
                    break;
                }
            }

            const size_t bytesAvailable = count_ - pos_;
            const size_t bytesToRead = std::min(bytesAvailable, remainingLen);
            std::copy_n(buffer_.begin() + static_cast<std::ptrdiff_t>(pos_), bytesToRead, cBuf.begin() + static_cast<std::ptrdiff_t>(currentOffset));
            totalBytesRead += bytesToRead;
            currentOffset += bytesToRead;
            remainingLen -= bytesToRead;
            pos_ += bytesToRead;
        }

        return totalBytesRead > 0 ? static_cast<int>(totalBytesRead) : -1;
    }

    auto BufferedReader::readLine() -> std::string {
        DLOG(INFO) << "BufferedReader readLine - starting to read line";
        std::string line;

        // Pre-allocate small buffer to avoid multiple allocations for typical lines
        line.reserve(64);

        while (true) {
            if (pos_ >= count_) {
                if (!fillBuffer()) {
                    break;
                }
            }

            const char ch = buffer_[pos_++];
            if (ch == '\n') {
                break;
            }
            if (ch != '\r') {
                line += ch;
            }
        }

        DLOG(INFO) << fmt::format("BufferedReader readLine completed - line length: {}", line.length());
        return line;
    }

    auto BufferedReader::ready() const -> bool {
        return reader_->ready();
    }

    auto BufferedReader::skip(const size_t n) -> size_t {
        if (n == 0) {
            return 0;
        }

        size_t skipped = 0;
        size_t remaining = n;

        while (remaining > 0) {
            if (pos_ >= count_) {
                if (!fillBuffer()) {
                    break;
                }
            }

            const size_t bytesToSkip = std::min(count_ - pos_, remaining);
            pos_ += bytesToSkip;
            remaining -= bytesToSkip;
            skipped += bytesToSkip;
        }

        return skipped;
    }

    auto BufferedReader::fillBuffer() -> bool {
        pos_ = 0;
        const int bytesRead = reader_->read(buffer_, 0, buffer_size_);
        count_ = bytesRead > 0 ? static_cast<size_t>(bytesRead) : 0;
        if (count_ > 0) {
            DLOG(INFO) << fmt::format("BufferedReader fillBuffer - filled {} bytes", count_);
        } else {
            DLOG(INFO) << "BufferedReader fillBuffer - end of stream";
        }
        return count_ > 0;
    }

    auto BufferedReader::isClosed() const -> bool {
        return !reader_ || reader_->isClosed();
    }
}
