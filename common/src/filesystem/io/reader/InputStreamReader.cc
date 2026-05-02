#include "src/filesystem/io/reader/InputStreamReader.hpp"

#include <fmt/format.h>

namespace common::filesystem {
    InputStreamReader::InputStreamReader(std::shared_ptr<AbstractReader> input) : reader_(std::move(input)) {
        if (!reader_) {
throw std::invalid_argument("InputStreamReader::InputStreamReader: Input stream cannot be null");
        }
}

    InputStreamReader::InputStreamReader(std::shared_ptr<AbstractReader> input, const std::string &charsetName) : reader_(std::move(input)) {
        if (!reader_) {
throw std::invalid_argument("InputStreamReader::InputStreamReader: Input stream cannot be null");
        }
        if (charsetName != "UTF-8") {
throw std::invalid_argument("InputStreamReader::InputStreamReader: Only UTF-8 charset is supported in this implementation");
        }
}

    auto InputStreamReader::read() -> int {
        if (closed_ || !reader_) {
throw std::runtime_error("InputStreamReader::read: Input stream is not available");
        }

        // UTF-8 character can be 1-4 bytes, read one byte at a time to determine character boundaries
        const int firstByte = reader_->read();
        if (firstByte == -1) {
return -1;
        }

        // Table-driven approach to determine UTF-8 character length
        // UTF-8 byte patterns:
        // 0xxxxxxx -> 1 byte (ASCII)
        // 110xxxxx -> 2 bytes
        // 1110xxxx -> 3 bytes  
        // 11110xxx -> 4 bytes
        // Others are invalid as start bytes
        constexpr struct {
            unsigned char mask;
            unsigned char pattern;
            int additional_bytes;
        } utf8_length_table[] = {
            {0x80, 0x00, 0}, // ASCII (0xxxxxxx)
            {0xE0, 0xC0, 1}, // 2-byte (110xxxxx)
            {0xF0, 0xE0, 2}, // 3-byte (1110xxxx)
            {0xF8, 0xF0, 3}, // 4-byte (11110xxx)
        };

        const auto firstByteUnsigned = static_cast<unsigned char>(firstByte);
        int additionalBytes = -1;

        for (const auto &[mask, pattern, additional_bytes]: utf8_length_table) {
            if ((firstByteUnsigned & mask) == pattern) {
                additionalBytes = additional_bytes;
                break;
            }
        }

        if (additionalBytes == -1) {
            // Invalid UTF-8 first byte
throw std::runtime_error("InputStreamReader::read: Invalid UTF-8 sequence");
        }

        // Read additional bytes if needed
        std::vector<char> fullByteBuffer(1 + additionalBytes);
        fullByteBuffer[0] = static_cast<char>(firstByte);

        for (int i = 0; i < additionalBytes; ++i) {
            const int nextByte = reader_->read();
            if (nextByte == -1) {
throw std::runtime_error("InputStreamReader::read: Incomplete UTF-8 sequence");
            }
            if (const auto nextByteUnsigned = static_cast<unsigned char>(nextByte); (nextByteUnsigned & 0xC0) != 0x80) {
throw std::runtime_error("InputStreamReader::read: Invalid UTF-8 sequence");
            }
            fullByteBuffer[1 + i] = static_cast<char>(nextByte);
        }

        try {
            const std::string byteStr(fullByteBuffer.data(), fullByteBuffer.size());
            const std::u32string chars = converter_.from_bytes(byteStr);
            if (chars.empty()) {
                return -1;
            }
return static_cast<unsigned char>(chars[0]);
        } catch (const std::exception &ex) {
throw std::runtime_error("InputStreamReader::read: Failed to decode byte to character - " + std::string(ex.what()));
        }
    }

    auto InputStreamReader::read(std::vector<char> &cBuf, const size_t off, const size_t len) -> int {
        if (closed_ || !reader_) {
            throw std::runtime_error("InputStreamReader::read(buffer,off,len): Input stream is not available");
        }

        if (off > cBuf.size() || len > cBuf.size() - off) {
            throw std::out_of_range("InputStreamReader::read: Buffer overflow");
        }

        if (len == 0) {
            return 0;
        }

        // For simplicity, read one character at a time
        // A more optimized implementation would read multiple bytes and decode them together
        size_t totalCharsRead = 0;
        for (size_t i = 0; i < len; ++i) {
            const int ch = read();
            if (ch == -1) {
                return totalCharsRead > 0 ? static_cast<int>(totalCharsRead) : -1;
            }
            cBuf[off + i] = static_cast<char>(ch);
            ++totalCharsRead;
        }

        return static_cast<int>(totalCharsRead);
    }

    auto InputStreamReader::ready() const -> bool {
        if (closed_ || !reader_) {
            throw std::runtime_error("InputStreamReader::ready: Input stream is not available");
        }
        return reader_->ready();
    }

    auto InputStreamReader::close() -> void {
        closed_ = true;
        if (reader_) {
            reader_->close();
        }
    }

    auto InputStreamReader::markSupported() const -> bool {
        return false;
    }

    auto InputStreamReader::mark(const size_t readAheadLimit) -> void {
        static_cast<void>(readAheadLimit); // NOLINT(readability-unused-parameter) - Unused parameter in interface
        throw std::runtime_error("InputStreamReader::mark: Mark not supported");
    }

    auto InputStreamReader::reset() -> void {
        throw std::runtime_error("InputStreamReader::reset: Reset not supported");
    }

    auto InputStreamReader::isClosed() const -> bool {
        return closed_ || !reader_ || reader_->isClosed();
    }
}
