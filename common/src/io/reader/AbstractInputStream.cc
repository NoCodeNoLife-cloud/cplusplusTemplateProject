/**
 * @file AbstractInputStream.cc
 * @brief AbstractInputStream implementation â€?default read/skip/available logic
 * @details Provides default implementations for AbstractInputStream methods:
 *          single-byte read() delegates to array read(), skip() via read(),
 *          and available() defaults to 0.  Subclasses override for efficiency.
 */

#include "io/reader/AbstractInputStream.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>
#include <fmt/format.h>

namespace cppforge::io::reader
{
    void AbstractInputStream::mark(std::int32_t readLimit)
    {
        throw std::runtime_error("Mark operation not supported by this input stream implementation");
    }

    bool AbstractInputStream::markSupported() const
    {
        return false;
    }

    size_t AbstractInputStream::read(std::vector<std::byte>& buffer)
    {
        return read(buffer, 0, buffer.size());
    }

    size_t AbstractInputStream::read(std::vector<std::byte>& buffer, const std::size_t offset, const std::size_t len)
    {
        // Check for buffer overflow
        if (offset > buffer.size() || len > buffer.size() - offset)
        {
            throw std::out_of_range("Buffer offset/length out of range");
        }

        eof_ = false;
        std::size_t bytesRead = 0;
        for (std::size_t i = 0; i < len; ++i)
        {
            if (eof_)
            {
                break;
            }
            buffer[offset + i] = read();
            ++bytesRead;
        }
        return bytesRead;
    }

    void AbstractInputStream::reset()
    {
        throw std::runtime_error("Reset operation not supported by this input stream implementation");
    }

    int64_t AbstractInputStream::skip(const int64_t n)
    {
        if (n <= 0) return 0;
        eof_ = false;
        int64_t skipped = 0;
        for (int64_t i = 0; i < n && !eof_; ++i)
        {
            read();
            ++skipped;
        }
        return skipped;
    }
}