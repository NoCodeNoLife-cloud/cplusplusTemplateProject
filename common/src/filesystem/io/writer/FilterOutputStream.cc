/**
 * @file FilterOutputStream.cc
 * @brief FilterOutputStream class implementation
 * @details This file contains the implementation of the FilterOutputStream class methods for Common library utilities.
 */

#include "src/filesystem/io/writer/FilterOutputStream.hpp"

#include <fmt/format.h>

namespace common::filesystem
{
    FilterOutputStream::FilterOutputStream(std::shared_ptr<AbstractOutputStream> outputStream) : output_stream_(std::move(outputStream))
    {
    }

    void FilterOutputStream::write(const std::byte b)
    {
        if (!output_stream_)
        {
            throw std::runtime_error("Output stream is not available");
        }
        output_stream_->write(b);
    }

    void FilterOutputStream::write(const std::vector<std::byte>& buffer)
    {
        if (!output_stream_)
        {
            throw std::runtime_error("Output stream is not available");
        }
        output_stream_->write(buffer);
    }

    void FilterOutputStream::write(const std::vector<std::byte>& buffer, const size_t offset, const size_t len)
    {
        if (!output_stream_)
        {
            throw std::runtime_error("Output stream is not available");
        }
        if (offset + len > buffer.size())
        {
            throw std::out_of_range("Buffer overflow");
        }
        output_stream_->write(buffer, offset, len);
    }

    void FilterOutputStream::write(const std::byte* buffer, const size_t length)
    {
        if (!output_stream_)
        {
            throw std::runtime_error("Output stream is not available");
        }
        if (!buffer)
        {
            throw std::invalid_argument("Buffer cannot be null");
        }
        output_stream_->write(buffer, length);
    }

    void FilterOutputStream::flush()
    {
        if (!output_stream_)
        {
            throw std::runtime_error("Output stream is not available");
        }
        output_stream_->flush();
    }

    void FilterOutputStream::close()
    {
        if (!output_stream_)
        {
            throw std::runtime_error("Output stream is not available");
        }
        flush();
        output_stream_->close();
    }

    bool FilterOutputStream::isClosed() const
    {
        // A filter stream is considered closed if the underlying stream is not available
        // or if the underlying stream reports being closed
        return !output_stream_ || output_stream_->isClosed();
    }
}