/**
 * @file FilterOutputStream.cc
 * @brief FilterOutputStream class implementation
 * @details This file contains the implementation of the FilterOutputStream class methods for Common library utilities.
 */

#include <stdexcept>

#include "filesystem/io/writer/FilterOutputStream.hpp"

namespace common::filesystem
{
    FilterOutputStream::FilterOutputStream(std::shared_ptr<AbstractOutputStream> outputStream) : output_stream_(std::move(outputStream))
    {
    }

    void FilterOutputStream::ensureStream() const
    {
        if (!output_stream_)
        {
            throw std::runtime_error("Output stream is not available");
        }
    }

    void FilterOutputStream::write(const std::byte b)
    {
        ensureStream();
        output_stream_->write(b);
    }

    void FilterOutputStream::write(const std::vector<std::byte>& buffer)
    {
        ensureStream();
        output_stream_->write(buffer);
    }

    void FilterOutputStream::write(const std::vector<std::byte>& buffer, const size_t offset, const size_t len)
    {
        ensureStream();
        if (offset > buffer.size() || len > buffer.size() - offset)
        {
            throw std::out_of_range("Buffer overflow");
        }
        output_stream_->write(buffer, offset, len);
    }

    void FilterOutputStream::write(const std::byte* buffer, const size_t length)
    {
        ensureStream();
        if (!buffer)
        {
            throw std::invalid_argument("Buffer cannot be null");
        }
        output_stream_->write(buffer, length);
    }

    void FilterOutputStream::flush()
    {
        ensureStream();
        output_stream_->flush();
    }

    void FilterOutputStream::close()
    {
        ensureStream();
        try
        {
            flush();
        }
        catch (...)
        {
            // Ignore flush error during close, proceed with closing
        }
        output_stream_->close();
    }

    bool FilterOutputStream::isClosed() const
    {
        return !output_stream_ || output_stream_->isClosed();
    }
}
