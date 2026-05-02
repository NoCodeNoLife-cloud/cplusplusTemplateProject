#include "src/filesystem/io/reader/AbstractInputStream.hpp"


#include <fmt/format.h>
#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>
#include <cstdint>

namespace common::filesystem {
    auto AbstractInputStream::mark(std::int32_t readLimit) -> void {
        throw std::runtime_error("Mark operation not supported by this input stream implementation");
    }

    auto AbstractInputStream::markSupported() const -> bool {
        return false;
    }

    auto AbstractInputStream::read(std::vector<std::byte> &buffer) -> size_t {
        return read(buffer, 0, buffer.size());
    }

    auto AbstractInputStream::read(std::vector<std::byte> &buffer, const std::size_t offset, const std::size_t len) -> size_t {
        // Check for buffer overflow
        if (offset > buffer.size() || len > buffer.size() - offset) {
            throw std::out_of_range("Buffer offset/length out of range");
        }

        std::size_t bytesRead = 0;
        for (std::size_t i = 0; i < len; ++i) {
            const std::byte byte = read();
            // Check for end of stream
            if (static_cast<int>(byte) == -1) {
                break;
            }
            buffer[offset + i] = byte;
            ++bytesRead;
        }
        return bytesRead;
    }

    auto AbstractInputStream::reset() -> void {
        throw std::runtime_error("Reset operation not supported by this input stream implementation");
    }

    auto AbstractInputStream::skip(const std::size_t n) -> size_t {
        std::size_t skipped = 0;
        // Prevent potential overflow
        const std::size_t maxSkip = std::min(n, static_cast<std::size_t>(std::numeric_limits<int>::max()));

        for (std::size_t i = 0; i < maxSkip; ++i) {
            if (static_cast<int>(read()) == -1) {
                break;
            }
            ++skipped;
        }
        return skipped;
    }
}
