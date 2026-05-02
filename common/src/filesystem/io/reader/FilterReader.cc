#include "src/filesystem/io/reader/FilterReader.hpp"

#include <fmt/format.h>
#include <stdexcept>

namespace common::filesystem {
    FilterReader::FilterReader(std::shared_ptr<AbstractReader> reader) : in_(std::move(reader)) {
    }

    auto FilterReader::close() -> void {
        if (!in_) {
throw std::runtime_error("Input stream is not available");
        }
in_->close();
    }

    auto FilterReader::mark(const size_t readAheadLimit) -> void {
        if (!in_) {
            throw std::runtime_error("Input stream is not available");
        }
        in_->mark(readAheadLimit);
    }

    auto FilterReader::markSupported() const -> bool {
        if (!in_) {
            return false;
        }
        return in_->markSupported();
    }

    auto FilterReader::read() -> int {
        if (!in_) {
throw std::runtime_error("Input stream is not available");
        }
        return in_->read();
    }

    auto FilterReader::read(std::vector<char> &cBuf, const size_t off, const size_t len) -> int {
        if (!in_) {
            throw std::runtime_error("Input stream is not available");
        }

        if (off > cBuf.size() || len > cBuf.size() - off) {
            throw std::out_of_range("Buffer overflow detected.");
        }
        return in_->read(cBuf, off, len);
    }

    auto FilterReader::read(std::vector<char> &cBuf) -> int {
        if (!in_) {
            throw std::runtime_error("Input stream is not available");
        }
        return in_->read(cBuf);
    }

    auto FilterReader::ready() const -> bool {
        if (!in_) {
            return false;
        }
        return in_->ready();
    }

    auto FilterReader::reset() -> void {
        if (!in_) {
            throw std::runtime_error("Input stream is not available");
        }
        in_->reset();
    }

    auto FilterReader::skip(const size_t n) -> size_t {
        if (!in_) {
            throw std::runtime_error("Input stream is not available");
        }
        return in_->skip(n);
    }

    auto FilterReader::isClosed() const -> bool {
        if (!in_) {
            return true;
        }
        return in_->isClosed();
    }
}
