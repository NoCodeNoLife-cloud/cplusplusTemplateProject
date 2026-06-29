/**
 * @file AbstractOutputStream.cc
 * @brief AbstractOutputStream implementation â€?default vector-based write logic
 * @details Provides default implementations for the vector-based write()
 *          overloads: write(vector) calls write(vector, 0, size), and
 *          write(vector, offset, len) delegates to the pure-virtual
 *          write(byte*, length) after bounds-checking.
 */

#include "io/writer/AbstractOutputStream.hpp"

#include <fmt/format.h>

namespace cppforge::io::writer
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
        if (offset > buffer.size() || len > buffer.size() - offset)
        {
            throw std::out_of_range("Buffer offset/length out of range");
        }

        for (size_t i = 0; i < len; ++i)
        {
            write(buffer[offset + i]);
        }
    }
}
