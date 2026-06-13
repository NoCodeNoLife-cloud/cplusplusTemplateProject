/**
 * @file FilterInputStream.cc
 * @brief FilterInputStream implementation — delegating all calls to underlying
 * @details Implements the decorator: every public method delegates to the
 *          stored AbstractInputStream pointer.  Subclasses override only the
 *          methods they need to transform.
 */

#include "io/reader/FilterInputStream.hpp"

#include <cstddef>
#include <stdexcept>
#include <fmt/format.h>

namespace common::io::reader
{
    FilterInputStream::FilterInputStream(std::unique_ptr<AbstractInputStream> inputStream)  : input_stream_(std::move(inputStream))
    {
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
        const auto byte = input_stream_->read();
        if (input_stream_->isEof())
        {
            setEof();
        }
        return byte;
    }

    size_t FilterInputStream::read(std::vector<std::byte>& buffer)
    {
        validateInputStream();
        const auto n = input_stream_->read(buffer);
        if (input_stream_->isEof())
        {
            setEof();
        }
        return n;
    }

    size_t FilterInputStream::read(std::vector<std::byte>& buffer, const size_t offset, const size_t len)
    {
        validateInputStream();
        const auto n = input_stream_->read(buffer, offset, len);
        if (input_stream_->isEof())
        {
            setEof();
        }
        return n;
    }

    void FilterInputStream::reset()
    {
        validateInputStream();
        input_stream_->reset();
    }

    int64_t FilterInputStream::skip(const int64_t n)
    {
        validateInputStream();
        const auto skipped = input_stream_->skip(n);
        if (input_stream_->isEof())
        {
            setEof();
        }
        return skipped;
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

    void FilterInputStream::validateInputStream() const
    {
        if (!input_stream_)
        {
            throw std::runtime_error("FilterInputStream::validateInputStream: Input stream is not available");
        }
    }
}
