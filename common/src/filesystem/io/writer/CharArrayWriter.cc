/**
 * @file CharArrayWriter.cc
 * @brief CharArrayWriter class implementation
 * @details This file contains the implementation of the CharArrayWriter class methods for Common library utilities.
 */

#include "src/filesystem/io/writer/CharArrayWriter.hpp"

#include <fmt/format.h>
#include <stdexcept>

namespace common::filesystem
{
    CharArrayWriter::CharArrayWriter(const int32_t initialSize)
    {
        if (initialSize < 0)
        {
            throw std::invalid_argument("initialSize must be non-negative");
        }
        buf_.reserve(static_cast<size_t>(initialSize));
    }

    void CharArrayWriter::write(const char c)
    {
        ensureCapacity(count_ + 1);
        buf_[count_] = c;
        ++count_;
    }

    void CharArrayWriter::write(const std::vector<char>& cBuf, const size_t off, const size_t len)
    {
        if (len == 0)
        {
            return;
        }

        if (off + len > cBuf.size())
        {
            throw std::out_of_range("Invalid offset or length");
        }

        ensureCapacity(count_ + len);
        std::copy_n(cBuf.begin() + static_cast<std::ptrdiff_t>(off), len, buf_.begin() + static_cast<std::ptrdiff_t>(count_));
        count_ += len;
    }

    void CharArrayWriter::write(const std::vector<char>& cBuf)
    {
        write(cBuf, 0, cBuf.size());
    }

    void CharArrayWriter::write(const std::string& str, const size_t off, const size_t len)
    {
        if (len == 0)
        {
            return;
        }

        if (off + len > str.size())
        {
            throw std::out_of_range("Invalid offset or length");
        }

        ensureCapacity(count_ + len);
        std::copy_n(str.begin() + static_cast<std::ptrdiff_t>(off), len, buf_.begin() + static_cast<std::ptrdiff_t>(count_));
        count_ += len;
    }

    void CharArrayWriter::write(const std::string& str)
    {
        write(str, 0, str.size());
    }

    void CharArrayWriter::writeTo(AbstractWriter& out) const
    {
        if (count_ > 0)
        {
            out.write(buf_, 0, count_);
        }
    }

    CharArrayWriter& CharArrayWriter::append(const char c)
    {
        write(c);
        return *this;
    }

    CharArrayWriter& CharArrayWriter::append(const std::string& csq)
    {
        write(csq);
        return *this;
    }

    CharArrayWriter& CharArrayWriter::append(const std::string& csq, const size_t start, const size_t end)
    {
        if (start <= end && start <= csq.length())
        {
            const size_t safe_end = std::min(end, csq.length());
            write(csq, start, safe_end - start);
        }
        return *this;
    }

    CharArrayWriter& CharArrayWriter::append(const std::string_view str)
    {
        if (!str.empty())
        {
            write(std::string(str));
        }
        return *this;
    }

    CharArrayWriter& CharArrayWriter::append(const char* str)
    {
        if (str)
        {
            write(std::string(str));
        }
        return *this;
    }

    CharArrayWriter& CharArrayWriter::append(const std::initializer_list<char> chars)
    {
        if (chars.begin() != chars.end()) // Check if initializer_list is not empty
        {
            const auto vec = std::vector(chars);
            write(vec, 0, vec.size());
        }
        return *this;
    }

    CharArrayWriter& CharArrayWriter::append(const char* chars, const size_t count)
    {
        if (chars && count > 0)
        {
            const std::vector vec(chars, chars + count);
            write(vec);
        }
        return *this;
    }

    CharArrayWriter& CharArrayWriter::append(const char c, const size_t count)
    {
        if (count > 0)
        {
            const std::vector buf(count, c);
            write(buf);
        }
        return *this;
    }

    void CharArrayWriter::reset()
    {
        count_ = 0;
    }

    std::vector<char> CharArrayWriter::toCharArray() const
    {
        return {buf_.begin(), buf_.begin() + static_cast<std::vector<char>::difference_type>(count_)};
    }

    size_t CharArrayWriter::size() const
    {
        return count_;
    }

    std::string CharArrayWriter::toString() const
    {
        return {buf_.begin(), buf_.begin() + static_cast<std::vector<char>::difference_type>(count_)};
    }

    void CharArrayWriter::flush()
    {
        // No operation for CharArrayWriter.
    }

    void CharArrayWriter::close()
    {
        // No operation for CharArrayWriter.
    }

    bool CharArrayWriter::isClosed() const
    {
        // CharArrayWriter is never considered closed.
        return false;
    }

    void CharArrayWriter::ensureCapacity(const size_t minCapacity)
    {
        if (minCapacity > buf_.capacity())
        {
            buf_.resize(minCapacity);
        }
    }
}