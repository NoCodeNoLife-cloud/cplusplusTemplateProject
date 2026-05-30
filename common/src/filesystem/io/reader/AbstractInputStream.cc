/**
 * @file AbstractInputStream.cc
 * @brief AbstractInputStream class implementation
 * @details This file contains the implementation of the AbstractInputStream class methods for Common library utilities.
 */

#include "src/filesystem/io/reader/AbstractInputStream.hpp"

#include <fmt/format.h>
#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>
#include <cstdint>

namespace common::filesystem
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

        std::size_t bytesRead = 0;
        for (std::size_t i = 0; i < len; ++i)
        {
            const std::byte byte = read();
            // Check for end of stream
            if (static_cast<int>(byte) == -1)
            {
                break;
            }
            buffer[offset + i] = byte;
            ++bytesRead;
        }
        return bytesRead;
    }

    void AbstractInputStream::reset()
    {
        throw std::runtime_error("Reset operation not supported by this input stream implementation");
    }

    size_t AbstractInputStream::skip(const std::size_t n)
    {
        std::size_t skipped = 0;
        // Prevent potential overflow
        const std::size_t maxSkip = std::min(n, static_cast<std::size_t>(std::numeric_limits<int>::max()));

        for (std::size_t i = 0; i < maxSkip; ++i)
        {
            if (static_cast<int>(read()) == -1)
            {
                break;
            }
            ++skipped;
        }
        return skipped;
    }
}
