/**
 * @file ByteArrayOutputStream.cc
 * @brief ByteArrayOutputStream implementation â€?dynamic byte vector management
 * @details Implements in-memory byte output: write() appends to the internal
 *          vector (doubling capacity when full), toByteArray() returns a copy,
 *          toString() for text, writeTo() copies to another stream, and
 *          reset() clears for reuse.
 */

#include <cppforge/io/writer/ByteArrayOutputStream.hpp>

#include <stdexcept>
#include <fmt/format.h>

namespace cppforge::io::writer
{
    ByteArrayOutputStream::ByteArrayOutputStream() : buf_(32)
    {
    }

    ByteArrayOutputStream::ByteArrayOutputStream(const size_t size)
    {
        if (size == 0)
        {
            throw std::invalid_argument("Size must be greater than zero");
        }
        buf_.resize(size);
    }

    void ByteArrayOutputStream::write(const std::byte b)
    {
        ensureCapacity(1);
        buf_[count_++] = b;
    }

    void ByteArrayOutputStream::write(const std::vector<std::byte>& buffer, const size_t offset, const size_t len)
    {
        if (len == 0)
        {
            return;
        }

        if (offset > buffer.size() || len > buffer.size() - offset)
        {
            throw std::out_of_range("Buffer offset/length out of range");
        }

        ensureCapacity(len);
        std::copy_n(buffer.begin() + static_cast<std::vector<std::byte>::difference_type>(offset), len, buf_.begin() + static_cast<std::vector<std::byte>::difference_type>(count_));
        count_ += len;
    }

    void ByteArrayOutputStream::write(const std::byte* buffer, const size_t length)
    {
        if (length == 0)
        {
            return;
        }

        if (!buffer)
        {
            throw std::invalid_argument("Buffer cannot be null");
        }

        ensureCapacity(length);
        std::copy_n(buffer, length, buf_.begin() + static_cast<std::vector<std::byte>::difference_type>(count_));
        count_ += length;
    }

    void ByteArrayOutputStream::writeTo(AbstractOutputStream& out) const
    {
        if (count_ > 0)
        {
            out.write(buf_, 0, count_);
        }
    }

    void ByteArrayOutputStream::reset()
    {
        count_ = 0;
    }

    std::vector<std::byte> ByteArrayOutputStream::toByteArray() const
    {
        return {buf_.begin(), buf_.begin() + static_cast<std::vector<std::byte>::difference_type>(count_)};
    }

    size_t ByteArrayOutputStream::size() const
    {
        return count_;
    }

    std::string ByteArrayOutputStream::toString() const
    {
        return {reinterpret_cast<const char*>(buf_.data()), count_};
    }

    void ByteArrayOutputStream::flush()
    {
        // No operation for ByteArrayOutputStream.
    }

    void ByteArrayOutputStream::close()
    {
        // No operation for ByteArrayOutputStream.
    }

    size_t ByteArrayOutputStream::capacity() const
    {
        return buf_.size();
    }

    bool ByteArrayOutputStream::isClosed() const
    {
        return false; // ByteArrayOutputStream is never closed
    }

    void ByteArrayOutputStream::ensureCapacity(const size_t additionalCapacity)
    {
        if (count_ + additionalCapacity > buf_.size())
        {
            const size_t minCapacity = count_ + additionalCapacity;
            const size_t oldCapacity = buf_.size();
            size_t newCapacity = buf_.size() * 2;
            if (newCapacity < minCapacity)
            {
                newCapacity = minCapacity;
            }
            buf_.resize(newCapacity);
        }
    }
}
