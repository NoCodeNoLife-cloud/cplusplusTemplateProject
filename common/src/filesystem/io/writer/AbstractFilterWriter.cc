#include "src/filesystem/io/writer/AbstractFilterWriter.hpp"

#include <fmt/format.h>
#include <stdexcept>

namespace common::filesystem {
    AbstractFilterWriter::AbstractFilterWriter(std::unique_ptr<AbstractWriter> outputWriter) : output_writer_(std::move(outputWriter)) {
        if (!output_writer_) {
throw std::invalid_argument("Output writer cannot be null");
        }
}

    AbstractFilterWriter::~AbstractFilterWriter() = default;

    void AbstractFilterWriter::write(const char c) {
        checkOutputStream();
        output_writer_->write(c);
    }

    void AbstractFilterWriter::write(const std::vector<char> &cBuf, const size_t off, const size_t len) {
        checkOutputStream();
        if (off + len > cBuf.size()) {
            throw std::out_of_range("Buffer overflow");
        }
        output_writer_->write(cBuf, off, len);
    }

    void AbstractFilterWriter::write(const std::vector<char> &cBuf) {
        checkOutputStream();
        output_writer_->write(cBuf);
    }

    void AbstractFilterWriter::write(const std::string &str, const size_t off, const size_t len) {
        checkOutputStream();
        if (off + len > str.size()) {
            throw std::out_of_range("String overflow");
        }
        output_writer_->write(str, off, len);
    }

    void AbstractFilterWriter::write(const std::string &str) {
        checkOutputStream();
        output_writer_->write(str);
    }

    void AbstractFilterWriter::flush() {
        checkOutputStream();
        output_writer_->flush();
    }

    void AbstractFilterWriter::close() {
        checkOutputStream();
flush();
        output_writer_->close();
    }

    void AbstractFilterWriter::checkOutputStream() const {
        if (!output_writer_) {
throw std::runtime_error("Output stream is not available");
        }
    }
}
