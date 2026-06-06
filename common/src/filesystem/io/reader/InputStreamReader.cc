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

    std::optional<char> InputStreamReader::read()
    {
        if (closed_ || !reader_)
        {
            throw std::runtime_error("InputStreamReader::read: Input stream is not available");
        }

        return reader_->read();
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
            const auto ch = read();
            if (!ch.has_value())
            {
                return totalCharsRead > 0 ? static_cast<int>(totalCharsRead) : -1;
            }
            cBuf[off + i] = ch.value();
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