/**
 * @file InputStreamReader.cc
 * @brief InputStreamReader class implementation
 * @details This file contains the implementation of the InputStreamReader class methods for Common library utilities.
 */

#include "filesystem/io/reader/InputStreamReader.hpp"

#include <fmt/format.h>

namespace common::filesystem
{
    InputStreamReader::InputStreamReader(std::shared_ptr<AbstractReader> input) : reader_(std::move(input))
    {
        if (!reader_)
        {
            throw std::invalid_argument("InputStreamReader::InputStreamReader: Input stream cannot be null");
        }
    }

    InputStreamReader::InputStreamReader(std::shared_ptr<AbstractReader> input, const std::string& charsetName) : reader_(std::move(input))
    {
        if (!reader_)
        {
            throw std::invalid_argument("InputStreamReader::InputStreamReader: Input stream cannot be null");
        }
        if (charsetName != "UTF-8")
        {
            throw std::invalid_argument("InputStreamReader::InputStreamReader: Only UTF-8 charset is supported in this implementation");
        }
    }

    int InputStreamReader::read()
    {
        if (closed_ || !reader_)
        {
            throw std::runtime_error("InputStreamReader::read: Input stream is not available");
        }

        // UTF-8 character can be 1-4 bytes, read one byte at a time to determine character boundaries
        const int firstByte = reader_->read();
        if (firstByte == -1)
        {
            return -1;
        }

        // Table-driven approach to determine UTF-8 character length
        // UTF-8 byte patterns:
        // 0xxxxxxx -> 1 byte (ASCII)
        // 110xxxxx -> 2 bytes
        // 1110xxxx -> 3 bytes
        // 11110xxx -> 4 bytes
        // Others are invalid as start bytes
        constexpr struct
        {
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

        for (const auto& [mask, pattern, additional_bytes] : utf8_length_table)
        {
            if ((firstByteUnsigned & mask) == pattern)
            {
                additionalBytes = additional_bytes;
                break;
            }
        }

        if (additionalBytes == -1)
        {
            // Invalid UTF-8 first byte
            throw std::runtime_error("InputStreamReader::read: Invalid UTF-8 sequence");
        }

        // Decode UTF-8 codepoint from the validated bytes
        unsigned char bytes[4] = {firstByteUnsigned, 0, 0, 0};
        for (int i = 0; i < additionalBytes; ++i)
        {
            const int nextByte = reader_->read();
            if (nextByte == -1)
            {
                throw std::runtime_error("InputStreamReader::read: Incomplete UTF-8 sequence");
            }
            const auto nextByteUnsigned = static_cast<unsigned char>(nextByte);
            if ((nextByteUnsigned & 0xC0) != 0x80)
            {
                throw std::runtime_error("InputStreamReader::read: Invalid UTF-8 sequence");
            }
            bytes[1 + i] = nextByteUnsigned;
        }

        char32_t codepoint;
        switch (additionalBytes)
        {
            case 0:
                codepoint = bytes[0];
                break;
            case 1:
                codepoint = (static_cast<char32_t>(bytes[0] & 0x1F) << 6) |
                            (static_cast<char32_t>(bytes[1] & 0x3F));
                break;
            case 2:
                codepoint = (static_cast<char32_t>(bytes[0] & 0x0F) << 12) |
                            (static_cast<char32_t>(bytes[1] & 0x3F) << 6) |
                            (static_cast<char32_t>(bytes[2] & 0x3F));
                break;
            case 3:
                codepoint = (static_cast<char32_t>(bytes[0] & 0x07) << 18) |
                            (static_cast<char32_t>(bytes[1] & 0x3F) << 12) |
                            (static_cast<char32_t>(bytes[2] & 0x3F) << 6) |
                            (static_cast<char32_t>(bytes[3] & 0x3F));
                break;
            default:
                throw std::runtime_error("InputStreamReader::read: Invalid UTF-8 sequence length");
        }
        return static_cast<int>(codepoint);
    }

    int InputStreamReader::read(std::vector<char>& cBuf, const size_t off, const size_t len)
    {
        if (closed_ || !reader_)
        {
            throw std::runtime_error("InputStreamReader::read(buffer,off,len): Input stream is not available");
        }

        if (off > cBuf.size() || len > cBuf.size() - off)
        {
            throw std::out_of_range("InputStreamReader::read: Buffer overflow");
        }

        if (len == 0)
        {
            return 0;
        }

        // For simplicity, read one character at a time
        // A more optimized implementation would read multiple bytes and decode them together
        size_t totalCharsRead = 0;
        for (size_t i = 0; i < len; ++i)
        {
            const int ch = read();
            if (ch == -1)
            {
                return totalCharsRead > 0 ? static_cast<int>(totalCharsRead) : -1;
            }
            cBuf[off + i] = static_cast<char>(ch);
            ++totalCharsRead;
        }

        return static_cast<int>(totalCharsRead);
    }

    bool InputStreamReader::ready() const
    {
        if (closed_ || !reader_)
        {
            throw std::runtime_error("InputStreamReader::ready: Input stream is not available");
        }
        return reader_->ready();
    }

    void InputStreamReader::close()
    {
        closed_ = true;
        if (reader_)
        {
            reader_->close();
        }
    }

    bool InputStreamReader::markSupported() const
    {
        return false;
    }

    void InputStreamReader::mark(const size_t readAheadLimit)
    {
        static_cast<void>(readAheadLimit); // NOLINT(readability-unused-parameter) - Unused parameter in interface
        throw std::runtime_error("InputStreamReader::mark: Mark not supported");
    }

    void InputStreamReader::reset()
    {
        throw std::runtime_error("InputStreamReader::reset: Reset not supported");
    }

    bool InputStreamReader::isClosed() const
    {
        return closed_ || !reader_ || reader_->isClosed();
    }
}