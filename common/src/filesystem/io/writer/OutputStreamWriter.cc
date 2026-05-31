/**
 * @file OutputStreamWriter.cc
 * @brief OutputStreamWriter class implementation
 * @details This file contains the implementation of the OutputStreamWriter class methods for Common library utilities.
 */

#include "src/filesystem/io/writer/OutputStreamWriter.hpp"

#include <fmt/format.h>

namespace common::filesystem
{
    OutputStreamWriter::OutputStreamWriter(std::unique_ptr<AbstractWriter> outputStream, const std::string& charsetName) : output_writer_(std::move(outputStream)), charset_(charsetName), closed_(false)
    {
        if (charsetName != "UTF-8")
        {
            throw std::invalid_argument("Unsupported encoding: " + charsetName);
        }
    }

    OutputStreamWriter::OutputStreamWriter(std::unique_ptr<AbstractWriter> outputStream) : OutputStreamWriter(std::move(outputStream), "UTF-8")
    {
    }

    OutputStreamWriter::~OutputStreamWriter() = default;

    std::string OutputStreamWriter::getEncoding() const
    {
        return closed_ ? "" : charset_;
    }

    void OutputStreamWriter::write(const char c)
    {
        checkIfClosed();
        write(std::string(1, c));
    }

    void OutputStreamWriter::write(const std::vector<char>& cBuf)
    {
        if (!cBuf.empty())
        {
            write(cBuf, 0, cBuf.size());
        }
    }

    void OutputStreamWriter::write(const std::vector<char>& cBuf, const size_t off, const size_t len)
    {
        if (len == 0)
        {
            return;
        }

        checkIfClosed();
        if (off + len > cBuf.size())
        {
            throw std::out_of_range("Offset and length exceed buffer size");
        }
        output_writer_->write(cBuf, off, len);
        checkOutputStream();
    }

    void OutputStreamWriter::write(const std::string& str)
    {
        if (!str.empty())
        {
            write(std::vector(str.begin(), str.end()));
        }
    }

    void OutputStreamWriter::write(const std::string& str, const size_t off, const size_t len)
    {
        if (len == 0)
        {
            return;
        }

        if (off + len > str.size())
        {
            throw std::out_of_range("Offset and length exceed string size");
        }
        write(std::vector(str.begin() + static_cast<std::string::difference_type>(off), str.begin() + static_cast<std::string::difference_type>(off + len)));
    }

    void OutputStreamWriter::flush()
    {
        checkIfClosed();
        output_writer_->flush();
        checkOutputStream();
    }

    void OutputStreamWriter::close()
    {
        if (closed_)
        {
            return;
        }
        flush();
        closed_ = true;
    }

    bool OutputStreamWriter::isClosed() const
    {
        return closed_;
    }

    AbstractWriter& OutputStreamWriter::append(const char c)
    {
        write(c);
        return *this;
    }

    AbstractWriter& OutputStreamWriter::append(const std::string& csq)
    {
        write(csq);
        return *this;
    }

    AbstractWriter& OutputStreamWriter::append(const std::string& csq, const size_t start, const size_t end)
    {
        if (start <= end && start <= csq.length())
        {
            const size_t safe_end = std::min(end, csq.length());
            write(csq, start, safe_end - start);
        }
        return *this;
    }

    std::string OutputStreamWriter::toString() const
    {
        checkIfClosed();
        return output_writer_->toString();
    }

    void OutputStreamWriter::checkIfClosed() const
    {
        if (closed_)
        {
            throw std::ios_base::failure("Stream is closed");
        }
    }

    void OutputStreamWriter::checkOutputStream() const
    {
        if (!output_writer_)
        {
            throw std::ios_base::failure("Failed to write to stream");
        }
    }
}