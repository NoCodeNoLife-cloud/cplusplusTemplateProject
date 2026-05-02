#include "src/filesystem/io/writer/BufferedOutputStream.hpp"

#include <fmt/format.h>

namespace common::filesystem {
    BufferedOutputStream::BufferedOutputStream(std::unique_ptr<AbstractOutputStream> out) : BufferedOutputStream(std::move(out), DEFAULT_BUFFER_SIZE) {
    }

    BufferedOutputStream::BufferedOutputStream(std::unique_ptr<AbstractOutputStream> out, const size_t size) : FilterOutputStream(std::move(out)), bufferSize_(size), buffer_(size) {
        if (!output_stream_) {
throw std::invalid_argument("Output stream cannot be null");
        }
        if (size == 0) {
throw std::invalid_argument("Buffer size must be greater than 0");
        }
}

    BufferedOutputStream::~BufferedOutputStream() {
        try {
            flush();
        } catch (...) {
            // Ignore exceptions during destruction
        }
    }

    auto BufferedOutputStream::write(const std::byte b) -> void {
        if (buffer_position_ >= bufferSize_) {
            flushBuffer();
        }
        buffer_[buffer_position_++] = b;
    }

    auto BufferedOutputStream::write(const std::vector<std::byte> &data, const size_t offset, const size_t len) -> void {
        if (len == 0) {
            return;
        }

        if (offset + len > data.size()) {
throw std::out_of_range("Data offset/length out of range");
        }
size_t bytesWritten = 0;
        while (bytesWritten < len) {
            if (buffer_position_ == bufferSize_) {
                flushBuffer();
            }
            const size_t bytesToCopy = std::min(len - bytesWritten, bufferSize_ - buffer_position_);
            std::memcpy(&buffer_[buffer_position_], &data[offset + bytesWritten], bytesToCopy);
            buffer_position_ += bytesToCopy;
            bytesWritten += bytesToCopy;
        }
    }

    auto BufferedOutputStream::write(const std::byte *buffer, const size_t length) -> void {
        if (length == 0) {
            return;
        }

        if (!buffer) {
            throw std::invalid_argument("Buffer cannot be null");
        }

        size_t bytesWritten = 0;
        while (bytesWritten < length) {
            if (buffer_position_ == bufferSize_) {
                flushBuffer();
            }
            const size_t bytesToCopy = std::min(length - bytesWritten, bufferSize_ - buffer_position_);
            std::memcpy(&buffer_[buffer_position_], buffer + bytesWritten, bytesToCopy);
            buffer_position_ += bytesToCopy;
            bytesWritten += bytesToCopy;
        }
    }

    auto BufferedOutputStream::flush() -> void {
flushBuffer();
        if (output_stream_) {
            output_stream_->flush();
        }
    }

    auto BufferedOutputStream::close() -> void {
flush();
        if (output_stream_) {
            output_stream_->close();
        }
    }

    auto BufferedOutputStream::flushBuffer() -> void {
        if (buffer_position_ > 0 && output_stream_) {
output_stream_->write(buffer_, 0, buffer_position_);
            buffer_position_ = 0;
        }
    }

    [[nodiscard]] auto BufferedOutputStream::getBufferSize() const noexcept -> size_t {
        return bufferSize_;
    }

    [[nodiscard]] auto BufferedOutputStream::getBufferedDataSize() const noexcept -> size_t {
        return buffer_position_;
    }

    [[nodiscard]] auto BufferedOutputStream::isClosed() const noexcept -> bool {
        return output_stream_ == nullptr;
    }
}
