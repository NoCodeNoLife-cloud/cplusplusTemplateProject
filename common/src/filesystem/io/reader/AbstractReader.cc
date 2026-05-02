#include "src/filesystem/io/reader/AbstractReader.hpp"

#include <fmt/format.h>
#include <algorithm>
#include <vector>
#include <stdexcept>

namespace common::filesystem {
    auto AbstractReader::read() -> int {
        std::vector<char> buffer(1);
        if (const int bytesRead = read(buffer, 0, 1); bytesRead <= 0) {
return -1;
        }
        return static_cast<unsigned char>(buffer[0]);
    }

    auto AbstractReader::read(std::vector<char> &cBuf) -> int {
        return read(cBuf, 0, cBuf.size());
    }

    auto AbstractReader::markSupported() const -> bool {
        return false;
    }

    auto AbstractReader::ready() const -> bool {
        return false;
    }

    auto AbstractReader::skip(const size_t n) -> size_t {
        if (n == 0) {
            return 0;
        }

        size_t skipped = 0;
        while (skipped < n) {
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
