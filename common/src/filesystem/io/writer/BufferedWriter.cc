#include "src/filesystem/io/writer/BufferedWriter.hpp"


#include <fmt/format.h>
#include <stdexcept>

namespace common::filesystem {
    BufferedWriter::BufferedWriter(std::unique_ptr<std::ofstream> os, const size_t size) : output_stream_(std::move(os)), buffer_size_(size) {
        if (!output_stream_) {
            throw std::runtime_error("Output stream is null.");
        }
        if (!output_stream_->is_open()) {
            throw std::runtime_error("Output stream is not open.");
        }
        buffer_.reserve(buffer_size_);
    }

    BufferedWriter::~BufferedWriter() {
        try {
            close();
        } catch (...) {
            // Ignore exceptions during destruction
        }
    }

    auto BufferedWriter::write(const std::string &str) -> void {
        if (str.size() > buffer_size_) {
            flush();
            *output_stream_ << str;
        } else {
            for (const char c: str) {
                buffer_.push_back(c);
            }
            checkAndFlush();
        }
    }

    auto BufferedWriter::write(const std::vector<char> &cBuf, const size_t off, const size_t len) -> void {
        if (len == 0) {
            return;
        }

        if (off + len > cBuf.size()) {
            throw std::out_of_range("Offset and length are out of the bounds of the buffer.");
        }

        if (len > buffer_size_) {
            flush();
            output_stream_->write(cBuf.data() + off, static_cast<std::streamsize>(len));
        } else {
            for (size_t i = 0; i < len; ++i) {
                buffer_.push_back(cBuf[off + i]);
                checkAndFlush();
            }
        }
    }

    auto BufferedWriter::flush() -> void {
        if (!buffer_.empty()) {
            output_stream_->write(buffer_.data(), static_cast<std::streamsize>(buffer_.size()));
            buffer_.clear();
        }
        output_stream_->flush();
    }

    auto BufferedWriter::close() -> void {
        if (output_stream_ && output_stream_->is_open()) {
            flush();
            output_stream_->close();
        }
    }

    auto BufferedWriter::append(const char c) -> BufferedWriter & {
        buffer_.push_back(c);
        checkAndFlush();
        return *this;
    }

    auto BufferedWriter::append(const std::string &str) -> BufferedWriter & {
        for (const char c: str) {
            buffer_.push_back(c);
            checkAndFlush();
        }
        return *this;
    }

    auto BufferedWriter::append(const std::string &str, const size_t start, const size_t end) -> BufferedWriter & {
        if (start < str.length() && end <= str.length() && start < end) {
            for (size_t i = start; i < end; ++i) {
                buffer_.push_back(str[i]);
                checkAndFlush();
            }
        }
        return *this;
    }

    auto BufferedWriter::append(const std::string_view str) -> BufferedWriter & {
        for (const char c: str) {
            buffer_.push_back(c);
            checkAndFlush();
        }
        return *this;
    }

    auto BufferedWriter::append(const char *str) -> BufferedWriter & {
        if (str) {
            for (size_t i = 0; str[i] != '\0'; ++i) {
                buffer_.push_back(str[i]);
                checkAndFlush();
            }
        }
        return *this;
    }

    auto BufferedWriter::append(const std::initializer_list<char> chars) -> BufferedWriter & {
        for (const char c: chars) {
            buffer_.push_back(c);
            checkAndFlush();
        }
        return *this;
    }

    auto BufferedWriter::append(const char *chars, const size_t count) -> BufferedWriter & {
        if (chars) {
            for (size_t i = 0; i < count; ++i) {
                buffer_.push_back(chars[i]);
                checkAndFlush();
            }
        }
        return *this;
    }

    auto BufferedWriter::append(const char c, const size_t count) -> BufferedWriter & {
        for (size_t i = 0; i < count; ++i) {
            buffer_.push_back(c);
            checkAndFlush();
        }
        return *this;
    }

    auto BufferedWriter::newLine() -> BufferedWriter & {
        return append('\n');
    }

    auto BufferedWriter::toString() const -> std::string {
        return {buffer_.begin(), buffer_.end()};
    }

    auto BufferedWriter::isClosed() const noexcept -> bool {
        return output_stream_ == nullptr || !output_stream_->is_open();
    }

    auto BufferedWriter::checkAndFlush() -> void {
        if (buffer_.size() >= buffer_size_) {
            flush();
        }
    }
}
