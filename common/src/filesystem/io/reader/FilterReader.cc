/**
 * @file FilterReader.cc
 * @brief FilterReader class implementation
 * @details This file contains the implementation of the FilterReader class methods for Common library utilities.
 */

#include "src/filesystem/io/reader/FilterReader.hpp"

#include <stdexcept>

namespace common::filesystem
{
    FilterReader::FilterReader(std::shared_ptr<AbstractReader> reader) : in_(std::move(reader))
    {
    }

    void FilterReader::close()
    {
        if (!in_)
        {
            throw std::runtime_error("Input stream is not available");
        }
        in_->close();
    }

    void FilterReader::mark(const size_t readAheadLimit)
    {
        if (!in_)
        {
            throw std::runtime_error("Input stream is not available");
        }
        in_->mark(readAheadLimit);
    }

    bool FilterReader::markSupported() const
    {
        if (!in_)
        {
            return false;
        }
        return in_->markSupported();
    }

    int FilterReader::read()
    {
        if (!in_)
        {
            throw std::runtime_error("Input stream is not available");
        }
        return in_->read();
    }

    int FilterReader::read(std::vector<char>& cBuf, const size_t off, const size_t len)
    {
        if (!in_)
        {
            throw std::runtime_error("Input stream is not available");
        }

        if (off > cBuf.size() || len > cBuf.size() - off)
        {
            throw std::out_of_range("Buffer overflow detected.");
        }
        return in_->read(cBuf, off, len);
    }

    int FilterReader::read(std::vector<char>& cBuf)
    {
        if (!in_)
        {
            throw std::runtime_error("Input stream is not available");
        }
        return in_->read(cBuf);
    }

    bool FilterReader::ready() const
    {
        if (!in_)
        {
            return false;
        }
        return in_->ready();
    }

    void FilterReader::reset()
    {
        if (!in_)
        {
            throw std::runtime_error("Input stream is not available");
        }
        in_->reset();
    }

    size_t FilterReader::skip(const size_t n)
    {
        if (!in_)
        {
            throw std::runtime_error("Input stream is not available");
        }
        return in_->skip(n);
    }

    bool FilterReader::isClosed() const
    {
        if (!in_)
        {
            return true;
        }
        return in_->isClosed();
    }
}