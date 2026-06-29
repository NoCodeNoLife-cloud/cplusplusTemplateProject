/**
 * @file FilterOutputStream.cc
 * @brief FilterOutputStream implementation â€?delegating all calls to underlying
 * @details Implements the byte-output decorator: every public method delegates
 *          to the stored AbstractOutputStream pointer after ensureOpen checks.
 *          Subclasses override only the methods they need to transform.
 */

#include <stdexcept>

#include "io/writer/FilterOutputStream.hpp"

namespace cppforge::io::writer
{
    FilterOutputStream::FilterOutputStream(std::shared_ptr<AbstractOutputStream> outputStream) : output_stream_(std::move(outputStream))
    {
        if (!output_stream_)
        {
            throw std::invalid_argument("Output stream cannot be null");
        }
    }

    void FilterOutputStream::ensureStream() const
    {
        if (!output_stream_)
        {
            throw std::runtime_error("Output stream is not available");
        }
    }

    void FilterOutputStream::ensureOpen() const
    {
        if (closed_)
        {
            throw std::runtime_error("Output stream is closed");
        }
    }

    void FilterOutputStream::write(const std::byte b)
    {
        ensureStream();
        ensureOpen();
        output_stream_->write(b);
    }

    void FilterOutputStream::write(const std::vector<std::byte>& buffer)
    {
        ensureStream();
        ensureOpen();
        output_stream_->write(buffer);
    }

    void FilterOutputStream::write(const std::vector<std::byte>& buffer, const size_t offset, const size_t len)
    {
        ensureStream();
        ensureOpen();
        if (offset > buffer.size() || len > buffer.size() - offset)
        {
            throw std::out_of_range("Buffer overflow");
        }
        output_stream_->write(buffer, offset, len);
    }

    void FilterOutputStream::write(const std::byte* buffer, const size_t length)
    {
        ensureStream();
        ensureOpen();
        if (length == 0)
        {
            return;
        }
        if (!buffer)
        {
            throw std::invalid_argument("Buffer cannot be null");
        }
        output_stream_->write(buffer, length);
    }

    void FilterOutputStream::flush()
    {
        ensureStream();
        ensureOpen();
        output_stream_->flush();
    }

    void FilterOutputStream::close()
    {
        if (closed_)
        {
            return;
        }
        ensureStream();
        closed_ = true;
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
        return closed_ || !output_stream_ || output_stream_->isClosed();
    }
}
