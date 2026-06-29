/**
 * @file StringReader.cc
 * @brief StringReader implementation â€?in-memory character stream from string
 * @details Implements character reading from a std::string with position
 *          tracking, mark/reset, and close behaviour (string clear).
 */

#include "io/reader/StringReader.hpp"

#include <algorithm>
#include <stdexcept>
#include <fmt/format.h>

namespace cppforge::io::reader
{
    StringReader::StringReader(std::string s) : source_(std::move(s)), position_(0), mark_position_(0), mark_set_(false), closed_(false)
    {
    }

    void StringReader::close()
    {
        closed_ = true;
        source_.clear();
        position_ = 0;
        mark_position_ = 0;
        mark_set_ = false;
    }

    void StringReader::mark(const size_t readAheadLimit)
    {
        if (closed_)
        {
            throw std::runtime_error("StringReader::mark: Stream is closed");
        }
        mark_position_ = position_;
        mark_set_ = true;
        static_cast<void>(readAheadLimit); // Unused parameter
    }

    bool StringReader::markSupported() const
    {
        return true;
    }

    std::optional<char> StringReader::read()
    {
        if (closed_ || position_ >= source_.size())
        {
            return std::nullopt;
        }
        return source_[position_++];
    }

    int StringReader::read(std::vector<char>& cBuf, const size_t off, const size_t len)
    {
        if (closed_)
        {
            throw std::runtime_error("StringReader::read: Stream is closed");
        }

        if (off > cBuf.size() || len > cBuf.size() - off)
        {
            throw std::invalid_argument("StringReader::read: Offset is out of bounds of the buffer");
        }

        if (position_ >= source_.size())
        {
            return -1; // EOF
        }

        const size_t available = source_.size() - position_;
        const size_t actualRead = std::min({len, available, cBuf.size() - off});
        std::copy_n(source_.begin() + static_cast<std::ptrdiff_t>(position_), actualRead, cBuf.begin() + static_cast<std::ptrdiff_t>(off));
        position_ += actualRead;
        return static_cast<int>(actualRead > 0 ? actualRead : -1);
    }

    bool StringReader::ready() const
    {
        return !closed_ && position_ < source_.size();
    }

    void StringReader::reset()
    {
        if (closed_)
        {
            throw std::runtime_error("StringReader::reset: Stream is closed");
        }
        if (!mark_set_)
        {
            position_ = 0;
        }
        else
        {
            position_ = mark_position_;
        }
    }

    size_t StringReader::skip(const size_t ns)
    {
        if (closed_)
        {
            return 0;
        }
        const size_t charsToSkip = std::min(ns, source_.size() - position_);
        position_ += charsToSkip;
        return charsToSkip;
    }

    bool StringReader::isClosed() const
    {
        return closed_;
    }
}