#include "src/filesystem/io/writer/AbstractOutputStream.hpp"

#include <fmt/format.h>

namespace common::filesystem {
    auto AbstractOutputStream::write(const std::vector<std::byte> &buffer) -> void {
        if (!buffer.empty()) {
            write(buffer, 0, buffer.size());
        }
    }

    auto AbstractOutputStream::write(const std::vector<std::byte> &buffer, const size_t offset, const size_t len) -> void {
        if (offset + len > buffer.size()) {
            throw std::out_of_range("Buffer offset/length out of range");
        }

        for (size_t i = 0; i < len; ++i) {
            write(buffer[offset + i]);
        }
    }
}
