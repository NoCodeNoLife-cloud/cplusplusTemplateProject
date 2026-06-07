/**
 * @file CharArrayWriter.cc
 * @brief CharArrayWriter class implementation
 * @details This file contains the implementation of the CharArrayWriter class methods for Common library utilities.
 */

#include "filesystem/io/writer/CharArrayWriter.hpp"

#include <stdexcept>

namespace common::filesystem
{
    CharArrayWriter::CharArrayWriter(const int32_t initialSize)
    {
        if (initialSize < 0)
        {
            throw std::invalid_argument("initialSize must be non-negative");
        }
        buf_.resize(static_cast<size_t>(initialSize));
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

        if (off > cBuf.size() || len > cBuf.size() - off)
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

        if (off > str.size() || len > str.size() - off)
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
            ensureCapacity(count_ + str.size());
            std::copy_n(str.data(), str.size(), buf_.begin() + static_cast<std::ptrdiff_t>(count_));
            count_ += str.size();
        }
        return *this;
    }

    CharArrayWriter& CharArrayWriter::append(const char* str)
    {
        if (str)
        {
            const size_t len = std::char_traits<char>::length(str);
            ensureCapacity(count_ + len);
            std::copy_n(str, len, buf_.begin() + static_cast<std::ptrdiff_t>(count_));
            count_ += len;
        }
        return *this;
    }

    CharArrayWriter& CharArrayWriter::append(const std::initializer_list<char> chars)
    {
        if (chars.size() > 0)
        {
            ensureCapacity(count_ + chars.size());
            std::copy_n(chars.begin(), chars.size(), buf_.begin() + static_cast<std::ptrdiff_t>(count_));
            count_ += chars.size();
        }
        return *this;
    }

    CharArrayWriter& CharArrayWriter::append(const char* chars, const size_t count)
    {
        if (chars && count > 0)
        {
            ensureCapacity(count_ + count);
            std::copy_n(chars, count, buf_.begin() + static_cast<std::ptrdiff_t>(count_));
            count_ += count;
        }
        return *this;
    }

    CharArrayWriter& CharArrayWriter::append(const char c, const size_t count)
    {
        if (count > 0)
        {
            ensureCapacity(count_ + count);
            std::fill_n(buf_.begin() + static_cast<std::ptrdiff_t>(count_), count, c);
            count_ += count;
        }
        return *this;
    }

    CharArrayWriter& CharArrayWriter::append(const std::span<const char> chars)
    {
        if (!chars.empty())
        {
            ensureCapacity(count_ + chars.size());
            std::copy_n(chars.data(), chars.size(), buf_.begin() + static_cast<std::ptrdiff_t>(count_));
            count_ += chars.size();
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
    }

    void CharArrayWriter::close()
    {
    }

    bool CharArrayWriter::isClosed() const
    {
        return false;
    }

    void CharArrayWriter::ensureCapacity(const size_t minCapacity)
    {
        if (minCapacity > buf_.size())
        {
            buf_.resize(minCapacity);
        }
    }
}
