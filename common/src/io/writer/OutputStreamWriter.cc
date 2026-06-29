/**
 * @file OutputStreamWriter.cc
 * @brief OutputStreamWriter implementation â€?char-to-byte UTF-8 encoding
 * @details Implements the character-to-byte bridge: write()/append() encode
 *          characters as multi-byte UTF-8 sequences and forward them to the
 *          underlying AbstractOutputStream.  Tracks closed state to prevent
 *          writes after close.
 */

#include "io/writer/OutputStreamWriter.hpp"

#include <algorithm>
#include <ios>
#include <stdexcept>

namespace cppforge::io::writer
{
    OutputStreamWriter::OutputStreamWriter(std::unique_ptr<AbstractWriter> outputStream, const std::string& charsetName) : output_writer_(std::move(outputStream)), charset_(charsetName), closed_(false)
    {
        if (!output_writer_)
        {
            throw std::invalid_argument("OutputStreamWriter: outputStream must not be null");
        }
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
        return charset_;
    }

    void OutputStreamWriter::write(const char c)
    {
        checkIfClosed();
        const std::vector<char> buf(1, c);
        output_writer_->write(buf, 0, 1);
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
    }

    void OutputStreamWriter::write(const std::string& str)
    {
        if (!str.empty())
        {
            checkIfClosed();
            output_writer_->write(str);
        }
    }

    void OutputStreamWriter::write(const std::string& str, const size_t off, const size_t len)
    {
        if (len == 0)
        {
            return;
        }

        checkIfClosed();
        if (off + len > str.size())
        {
            throw std::out_of_range("Offset and length exceed string size");
        }
        output_writer_->write(str, off, len);
    }

    void OutputStreamWriter::flush()
    {
        checkIfClosed();
        output_writer_->flush();
    }

    void OutputStreamWriter::close()
    {
        if (closed_)
        {
            return;
        }
        flush();
        closed_ = true;
        output_writer_->close();
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
        if (start > end)
        {
            throw std::out_of_range("start > end");
        }
        if (start > csq.length())
        {
            throw std::out_of_range("start out of bounds");
        }
        const size_t safe_end = std::min(end, csq.length());
        write(csq, start, safe_end - start);
        return *this;
    }

    std::string OutputStreamWriter::toString() const
    {
        if (closed_)
        {
            return {};
        }
        return output_writer_->toString();
    }

    void OutputStreamWriter::checkIfClosed() const
    {
        if (closed_)
        {
            throw std::ios_base::failure("Stream is closed");
        }
    }
}
