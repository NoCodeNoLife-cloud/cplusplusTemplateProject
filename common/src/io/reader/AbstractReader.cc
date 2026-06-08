/**
* @file AbstractReader.cc
 * @brief AbstractReader class implementation
 * @details This file contains the implementation of the AbstractReader class methods for Common library utilities.
 */

#include "io/reader/AbstractReader.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>
#include <fmt/format.h>

namespace common::io::reader
{
    std::optional<char> AbstractReader::read()
    {
        std::vector<char> buffer(1);
        if (const int bytesRead = read(buffer, 0, 1); bytesRead <= 0)
        {
            return std::nullopt;
        }
        return static_cast<unsigned char>(buffer[0]);
    }

    int AbstractReader::read(std::vector<char>& cBuf)
    {
        return read(cBuf, 0, cBuf.size());
    }

    bool AbstractReader::markSupported() const
    {
        return false;
    }

    bool AbstractReader::ready() const
    {
        return false;
    }

    size_t AbstractReader::skip(const size_t n)
    {
        if (n == 0)
        {
            return 0;
        }

        size_t skipped = 0;
        while (skipped < n)
        {
            constexpr size_t BUFFER_SIZE = 1024;
            std::vector<char> buffer(BUFFER_SIZE);
            const size_t toRead = std::min(n - skipped, BUFFER_SIZE);
            const int readCount = read(buffer, 0, toRead);
            if (readCount <= 0) break;
            skipped += static_cast<size_t>(readCount);
        }
        return skipped;
    }
}