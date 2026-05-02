#include "src/filesystem/io/writer/OutputStreamWriter.hpp"

#include <fmt/format.h>

namespace common::filesystem {
    OutputStreamWriter::OutputStreamWriter(std::unique_ptr<AbstractWriter> outputStream, const std::string &charsetName) : output_writer_(std::move(outputStream)), charset_(charsetName), closed_(false) {
        if (charsetName != "UTF-8") {
throw std::invalid_argument("Unsupported encoding: " + charsetName);
        }
}

    OutputStreamWriter::OutputStreamWriter(std::unique_ptr<AbstractWriter> outputStream) : OutputStreamWriter(std::move(outputStream), "UTF-8") {
    }

    OutputStreamWriter::~OutputStreamWriter() = default;

    auto OutputStreamWriter::getEncoding() const -> std::string {
        return closed_ ? "" : charset_;
    }

    auto OutputStreamWriter::write(const char c) -> void {
        checkIfClosed();
        write(std::string(1, c));
    }

    auto OutputStreamWriter::write(const std::vector<char> &cBuf) -> void {
        if (!cBuf.empty()) {
            write(cBuf, 0, cBuf.size());
        }
    }

    auto OutputStreamWriter::write(const std::vector<char> &cBuf, const size_t off, const size_t len) -> void {
        if (len == 0) {
            return;
        }

        checkIfClosed();
        if (off + len > cBuf.size()) {
throw std::out_of_range("Offset and length exceed buffer size");
        }
output_writer_->write(cBuf, off, len);
        checkOutputStream();
    }

    auto OutputStreamWriter::write(const std::string &str) -> void {
        if (!str.empty()) {
            write(std::vector(str.begin(), str.end()));
        }
    }

    auto OutputStreamWriter::write(const std::string &str, const size_t off, const size_t len) -> void {
        if (len == 0) {
            return;
        }

        if (off + len > str.size()) {
throw std::out_of_range("Offset and length exceed string size");
        }
write(std::vector(str.begin() + static_cast<std::string::difference_type>(off), str.begin() + static_cast<std::string::difference_type>(off + len)));
    }

    auto OutputStreamWriter::flush() -> void {
        checkIfClosed();
output_writer_->flush();
        checkOutputStream();
    }

    auto OutputStreamWriter::close() -> void {
        if (closed_) {
return;
        }
flush();
        closed_ = true;
    }

    auto OutputStreamWriter::isClosed() const -> bool {
        return closed_;
    }

    auto OutputStreamWriter::append(const char c) -> AbstractWriter & {
        write(c);
        return *this;
    }

    auto OutputStreamWriter::append(const std::string &csq) -> AbstractWriter & {
        write(csq);
        return *this;
    }

    auto OutputStreamWriter::append(const std::string &csq, const size_t start, const size_t end) -> AbstractWriter & {
        if (start <= end && start <= csq.length()) {
            const size_t safe_end = std::min(end, csq.length());
            write(csq, start, safe_end - start);
        }
        return *this;
    }

    auto OutputStreamWriter::toString() const -> std::string {
        checkIfClosed();
        return output_writer_->toString();
    }

    auto OutputStreamWriter::checkIfClosed() const -> void {
        if (closed_) {
throw std::ios_base::failure("Stream is closed");
        }
    }

    auto OutputStreamWriter::checkOutputStream() const -> void {
        if (!output_writer_) {
throw std::ios_base::failure("Failed to write to stream");
        }
    }
}
