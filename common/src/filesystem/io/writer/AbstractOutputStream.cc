/**
 * @file AbstractOutputStream.cc
 * @brief AbstractOutputStream class implementation
 * @details This file contains the implementation of the AbstractOutputStream class methods for Common library utilities.
 */

#include "filesystem/io/writer/AbstractOutputStream.hpp"

#include <fmt/format.h>

namespace common::filesystem
{
    void AbstractOutputStream::write(const std::vector<std::byte>& buffer)
    {
        if (!buffer.empty())
        {
            write(buffer, 0, buffer.size());
        }
    }

    void AbstractOutputStream::write(const std::vector<std::byte>& buffer, const size_t offset, const size_t len)
    {
        if (offset + len > buffer.size())
        {
            throw std::out_of_range("Buffer offset/length out of range");
        }

        for (size_t i = 0; i < len; ++i)
        {
            write(buffer[offset + i]);
        }
    }
}
