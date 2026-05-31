/**
 * @file FilterInputStream.cc
 * @brief FilterInputStream class implementation
 * @details This file contains the implementation of the FilterInputStream class methods for Common library utilities.
 */

#include "filesystem/io/reader/FilterInputStream.hpp"

#include <fmt/format.h>
#include <cstddef>
#include <stdexcept>

namespace common::filesystem
{
    FilterInputStream::FilterInputStream(std::unique_ptr<AbstractInputStream> inputStream)  : input_stream_(std::move(inputStream))
    {
    }

    void FilterInputStream::validateInputStream() const
    {
        if (!input_stream_)
        {
            throw std::runtime_error("FilterInputStream::validateInputStream: Input stream is not available");
        }
    }

    size_t FilterInputStream::available()
    {
        validateInputStream();
        return input_stream_->available();
    }

    void FilterInputStream::mark(const int32_t readLimit)
    {
        validateInputStream();
        input_stream_->mark(readLimit);
    }

    bool FilterInputStream::markSupported() const
    {
        if (!input_stream_)
        {
            return false;
        }
        return input_stream_->markSupported();
    }

    std::byte FilterInputStream::read()
    {
        validateInputStream();
        return input_stream_->read();
    }

    size_t FilterInputStream::read(std::vector<std::byte>& buffer)
    {
        validateInputStream();
        return input_stream_->read(buffer);
    }

    size_t FilterInputStream::read(std::vector<std::byte>& buffer, const size_t offset, const size_t len)
    {
        validateInputStream();
        return input_stream_->read(buffer, offset, len);
    }

    void FilterInputStream::reset()
    {
        validateInputStream();
        input_stream_->reset();
    }

    size_t FilterInputStream::skip(const size_t n)
    {
        validateInputStream();
        return input_stream_->skip(n);
    }

    void FilterInputStream::close()
    {
        if (input_stream_)
        {
            input_stream_->close();
        }
    }

    bool FilterInputStream::isClosed() const
    {
        if (!input_stream_)
        {
            return true;
        }
        return input_stream_->isClosed();
    }
}