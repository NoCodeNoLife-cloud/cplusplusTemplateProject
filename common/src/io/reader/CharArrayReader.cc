/**
 * @file CharArrayReader.cc
 * @brief CharArrayReader implementation — in-memory character stream
 * @details Implements character reading from a std::vector<char> with
 *          offset/length bounds, position tracking, and mark/reset support.
 */

#include "io/reader/CharArrayReader.hpp"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <fmt/format.h>

namespace common::io::reader
{
    CharArrayReader::CharArrayReader(const std::vector<char>& buffer) : buf_(buffer), count_(buffer.size())
    {
    }

    CharArrayReader::CharArrayReader(const std::vector<char>& buffer, const size_t offset, const size_t length)
    {
        validateConstructorParams(buffer.size(), offset, length);
        buf_.assign(buffer.begin() + static_cast<std::ptrdiff_t>(offset), buffer.begin() + static_cast<std::ptrdiff_t>(offset + length));
        count_ = length;
    }

    CharArrayReader::~CharArrayReader() = default;

    std::optional<char> CharArrayReader::read()
    {
        if (closed_ || pos_ >= count_)
        {
            return std::nullopt;
        }
        return buf_[pos_++];
    }

    int CharArrayReader::read(std::vector<char>& b, const size_t off, const size_t len)
    {
        validateTargetBufferParams(b.size(), off, len);

        if (closed_ || pos_ >= count_)
        {
            return -1;
        }

        const size_t toRead = std::min(len, count_ - pos_);
        std::copy_n(buf_.begin() + static_cast<std::ptrdiff_t>(pos_), toRead, b.begin() + static_cast<std::ptrdiff_t>(off));
        pos_ += toRead;
        return static_cast<int>(toRead);
    }

    size_t CharArrayReader::skip(const size_t n)
    {
        if (closed_) return 0;

        const size_t skipped = std::min(n, count_ - pos_);
        pos_ += skipped;
        return skipped;
    }

    bool CharArrayReader::ready() const
    {
        return !closed_ && pos_ < count_;
    }

    bool CharArrayReader::markSupported() const
    {
        return true;
    }

    void CharArrayReader::mark(const size_t readAheadLimit)
    {
        if (closed_)
        {
            throw std::runtime_error("CharArrayReader::mark: Stream is closed");
        }
        marked_pos_ = pos_;
    }

    void CharArrayReader::reset()
    {
        if (closed_)
        {
            throw std::runtime_error("CharArrayReader::reset: Stream is closed");
        }
        pos_ = marked_pos_;
    }

    void CharArrayReader::close()
    {
        closed_ = true;
        buf_.clear();
        pos_ = 0;
        marked_pos_ = 0;
        count_ = 0;
    }

    bool CharArrayReader::isClosed() const
    {
        return closed_;
    }

    void CharArrayReader::validateConstructorParams(const size_t buffer_size, const size_t offset, const size_t length)
    {
        if (offset > buffer_size || offset + length > buffer_size)
        {
            throw std::invalid_argument("CharArrayReader::CharArrayReader: Invalid offset or length");
        }
    }

    void CharArrayReader::validateTargetBufferParams(const size_t target_buffer_size, const size_t offset, const size_t length)
    {
        if (offset > target_buffer_size || length > target_buffer_size - offset)
        {
            throw std::out_of_range("CharArrayReader::read: Invalid offset or length for target buffer");
        }
    }
}
