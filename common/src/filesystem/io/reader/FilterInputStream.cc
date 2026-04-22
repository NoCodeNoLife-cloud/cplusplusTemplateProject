#include "src/filesystem/io/reader/FilterInputStream.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <cstddef>
#include <stdexcept>

namespace common::filesystem {
    FilterInputStream::FilterInputStream(std::unique_ptr<AbstractInputStream> inputStream) noexcept : input_stream_(std::move(inputStream)) {
    }

    auto FilterInputStream::validateInputStream() const -> void {
        if (!input_stream_) {
            DLOG(ERROR) << "FilterInputStream validation failed - input stream is not available";
            throw std::runtime_error("FilterInputStream::validateInputStream: Input stream is not available");
        }
    }

    auto FilterInputStream::available() -> size_t {
        validateInputStream();
        return input_stream_->available();
    }

    auto FilterInputStream::mark(const int32_t readLimit) -> void {
        validateInputStream();
        input_stream_->mark(readLimit);
    }

    auto FilterInputStream::markSupported() const noexcept -> bool {
        if (!input_stream_) {
            return false;
        }
        return input_stream_->markSupported();
    }

    auto FilterInputStream::read() -> std::byte {
        validateInputStream();
        return input_stream_->read();
    }

    auto FilterInputStream::read(std::vector<std::byte> &buffer) -> size_t {
        validateInputStream();
        return input_stream_->read(buffer);
    }

    auto FilterInputStream::read(std::vector<std::byte> &buffer, const size_t offset, const size_t len) -> size_t {
        validateInputStream();
        return input_stream_->read(buffer, offset, len);
    }

    auto FilterInputStream::reset() -> void {
        validateInputStream();
        input_stream_->reset();
    }

    auto FilterInputStream::skip(const size_t n) -> size_t {
        validateInputStream();
        return input_stream_->skip(n);
    }

    auto FilterInputStream::close() -> void {
        if (input_stream_) {
            input_stream_->close();
        }
    }

    auto FilterInputStream::isClosed() const noexcept -> bool {
        if (!input_stream_) {
            return true;
        }
        return input_stream_->isClosed();
    }
}
