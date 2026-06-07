/**
 * @file BufferedOutputStream.cc
 * @brief BufferedOutputStream class implementation
 * @details This file contains the implementation of the BufferedOutputStream class methods for Common library utilities.
 */

#include "filesystem/io/writer/BufferedOutputStream.hpp"

#include <algorithm>
#include <stdexcept>

namespace common::filesystem::io::writer
{
    BufferedOutputStream::BufferedOutputStream(std::unique_ptr<AbstractOutputStream> out) : BufferedOutputStream(std::move(out), DEFAULT_BUFFER_SIZE)
    {
    }

    BufferedOutputStream::BufferedOutputStream(std::unique_ptr<AbstractOutputStream> out, const size_t size) : FilterOutputStream(std::move(out)), bufferSize_(size), buffer_(size)
    {
        if (!output_stream_)
        {
            throw std::invalid_argument("Output stream cannot be null");
        }
        if (size == 0)
        {
            throw std::invalid_argument("Buffer size must be greater than 0");
        }
    }

    BufferedOutputStream::~BufferedOutputStream()
    {
        try
        {
            flush();
        }
        catch (...)
        {
            // Ignore exceptions during destruction
        }
    }

    void BufferedOutputStream::write(const std::byte b)
    {
        if (closed_)
        {
            return;
        }
        if (buffer_position_ >= bufferSize_)
        {
            flushBuffer();
        }
        buffer_[buffer_position_++] = b;
    }

    void BufferedOutputStream::write(const std::vector<std::byte>& data, const size_t offset, const size_t len)
    {
        if (len == 0 || closed_)
        {
            return;
        }

        if (offset > data.size() || len > data.size() - offset)
        {
            throw std::out_of_range("Data offset/length out of range");
        }

        if (len >= bufferSize_)
        {
            flushBuffer();
            output_stream_->write(data, offset, len);
            return;
        }

        size_t bytesWritten = 0;
        while (bytesWritten < len)
        {
            if (buffer_position_ == bufferSize_)
            {
                flushBuffer();
            }
            const size_t bytesToCopy = std::min(len - bytesWritten, bufferSize_ - buffer_position_);
            std::memcpy(&buffer_[buffer_position_], &data[offset + bytesWritten], bytesToCopy);
            buffer_position_ += bytesToCopy;
            bytesWritten += bytesToCopy;
        }
    }

    void BufferedOutputStream::write(const std::byte* buffer, const size_t length)
    {
        if (length == 0 || closed_)
        {
            return;
        }

        if (!buffer)
        {
            throw std::invalid_argument("Buffer cannot be null");
        }

        if (length >= bufferSize_)
        {
            flushBuffer();
            output_stream_->write(buffer, length);
            return;
        }

        size_t bytesWritten = 0;
        while (bytesWritten < length)
        {
            if (buffer_position_ == bufferSize_)
            {
                flushBuffer();
            }
            const size_t bytesToCopy = std::min(length - bytesWritten, bufferSize_ - buffer_position_);
            std::memcpy(&buffer_[buffer_position_], buffer + bytesWritten, bytesToCopy);
            buffer_position_ += bytesToCopy;
            bytesWritten += bytesToCopy;
        }
    }

    void BufferedOutputStream::flush()
    {
        flushBuffer();
        if (output_stream_)
        {
            output_stream_->flush();
        }
    }

    void BufferedOutputStream::close()
    {
        if (closed_)
        {
            return;
        }
        closed_ = true;
        try
        {
            flush();
        }
        catch (...)
        {
            // Ignore flush error during close, proceed with closing
        }
        if (output_stream_)
        {
            output_stream_->close();
        }
    }

    [[nodiscard]] size_t BufferedOutputStream::getBufferSize() const
    {
        return bufferSize_;
    }

    [[nodiscard]] size_t BufferedOutputStream::getBufferedDataSize() const
    {
        return buffer_position_;
    }

    [[nodiscard]] bool BufferedOutputStream::isClosed() const
    {
        return closed_;
    }

    void BufferedOutputStream::flushBuffer()
    {
        if (buffer_position_ > 0 && output_stream_)
        {
            output_stream_->write(buffer_, 0, buffer_position_);
            buffer_position_ = 0;
        }
    }
}
