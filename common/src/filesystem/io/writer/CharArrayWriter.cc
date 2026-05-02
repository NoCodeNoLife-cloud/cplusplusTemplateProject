#include "src/filesystem/io/writer/CharArrayWriter.hpp"


#include <fmt/format.h>
#include <stdexcept>

namespace common::filesystem {
    CharArrayWriter::CharArrayWriter(const int32_t initialSize) {
        if (initialSize < 0) {
            throw std::invalid_argument("initialSize must be non-negative");
        }
        buf_.reserve(static_cast<size_t>(initialSize));
    }

    auto CharArrayWriter::write(const char c) -> void {
        ensureCapacity(count_ + 1);
        buf_[count_] = c;
        ++count_;
    }

    auto CharArrayWriter::write(const std::vector<char> &cBuf, const size_t off, const size_t len) -> void {
        if (len == 0) {
            return;
        }

        if (off + len > cBuf.size()) {
            throw std::out_of_range("Invalid offset or length");
        }

        ensureCapacity(count_ + len);
        std::copy_n(cBuf.begin() + static_cast<std::ptrdiff_t>(off), len, buf_.begin() + static_cast<std::ptrdiff_t>(count_));
        count_ += len;
    }

    auto CharArrayWriter::write(const std::vector<char> &cBuf) -> void {
        write(cBuf, 0, cBuf.size());
    }

    auto CharArrayWriter::write(const std::string &str, const size_t off, const size_t len) -> void {
        if (len == 0) {
            return;
        }

        if (off + len > str.size()) {
            throw std::out_of_range("Invalid offset or length");
        }

        ensureCapacity(count_ + len);
        std::copy_n(str.begin() + static_cast<std::ptrdiff_t>(off), len, buf_.begin() + static_cast<std::ptrdiff_t>(count_));
        count_ += len;
    }

    auto CharArrayWriter::write(const std::string &str) -> void {
        write(str, 0, str.size());
    }

    auto CharArrayWriter::writeTo(AbstractWriter &out) const -> void {
        if (count_ > 0) {
            out.write(buf_, 0, count_);
        }
    }

    auto CharArrayWriter::append(const char c) -> CharArrayWriter & {
        write(c);
        return *this;
    }

    auto CharArrayWriter::append(const std::string &csq) -> CharArrayWriter & {
        write(csq);
        return *this;
    }

    auto CharArrayWriter::append(const std::string &csq, const size_t start, const size_t end) -> CharArrayWriter & {
        if (start <= end && start <= csq.length()) {
            const size_t safe_end = std::min(end, csq.length());
            write(csq, start, safe_end - start);
        }
        return *this;
    }

    auto CharArrayWriter::append(const std::string_view str) -> CharArrayWriter & {
        if (!str.empty()) {
            write(std::string(str));
        }
        return *this;
    }

    auto CharArrayWriter::append(const char *str) -> CharArrayWriter & {
        if (str) {
            write(std::string(str));
        }
        return *this;
    }

    auto CharArrayWriter::append(const std::initializer_list<char> chars) -> CharArrayWriter & {
        if (chars.begin() != chars.end()) // Check if initializer_list is not empty
        {
            const auto vec = std::vector(chars);
            write(vec, 0, vec.size());
        }
        return *this;
    }

    auto CharArrayWriter::append(const char *chars, const size_t count) -> CharArrayWriter & {
        if (chars && count > 0) {
            const std::vector vec(chars, chars + count);
            write(vec);
        }
        return *this;
    }

    auto CharArrayWriter::append(const char c, const size_t count) -> CharArrayWriter & {
        if (count > 0) {
            const std::vector buf(count, c);
            write(buf);
        }
        return *this;
    }

    auto CharArrayWriter::reset() -> void {
        count_ = 0;
    }

    auto CharArrayWriter::toCharArray() const -> std::vector<char> {
        return {buf_.begin(), buf_.begin() + static_cast<std::vector<char>::difference_type>(count_)};
    }

    auto CharArrayWriter::size() const -> size_t {
        return count_;
    }

    auto CharArrayWriter::toString() const -> std::string {
        return {buf_.begin(), buf_.begin() + static_cast<std::vector<char>::difference_type>(count_)};
    }

    auto CharArrayWriter::flush() -> void {
        // No operation for CharArrayWriter.
    }

    auto CharArrayWriter::close() -> void {
        // No operation for CharArrayWriter.
    }

    auto CharArrayWriter::isClosed() const -> bool {
        // CharArrayWriter is never considered closed.
        return false;
    }

    auto CharArrayWriter::ensureCapacity(const size_t minCapacity) -> void {
        if (minCapacity > buf_.capacity()) {
            buf_.resize(minCapacity);
        }
    }
}
