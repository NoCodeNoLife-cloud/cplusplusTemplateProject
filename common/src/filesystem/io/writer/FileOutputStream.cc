#include "src/filesystem/io/writer/FileOutputStream.hpp"

#include <fmt/format.h>

namespace common::filesystem {
    FileOutputStream::FileOutputStream(const std::string &name, const bool append) {
        if (std::filesystem::exists(name) && std::filesystem::is_directory(name)) {
throw std::ios_base::failure("FileNotFoundException: Path is a directory.");
        }
file_stream_.open(name, std::ios::binary | (append ? std::ios::app : std::ios::trunc));
        if (!file_stream_.is_open()) {
throw std::ios_base::failure("FileNotFoundException: Unable to open or create file.");
        }
        file_name_ = name;
}

    FileOutputStream::FileOutputStream(const char *name, const bool append) : FileOutputStream(std::string(name), append) {
    }

    FileOutputStream::FileOutputStream(const std::filesystem::path &file, const bool append) : FileOutputStream(file.string(), append) {
    }

    FileOutputStream::~FileOutputStream() {
        if (file_stream_.is_open()) {
            file_stream_.close();
        }
    }

    auto FileOutputStream::write(std::byte b) -> void {
        checkStreamWritable("Cannot write to closed or unwritable stream.");
        checkStreamState();
        file_stream_.put(static_cast<char>(b));
        checkStreamState();
    }

    auto FileOutputStream::write(const std::vector<std::byte> &buffer) -> void {
        AbstractOutputStream::write(buffer);
    }

    auto FileOutputStream::write(const std::vector<std::byte> &buffer, const size_t offset, const size_t len) -> void {
        AbstractOutputStream::write(buffer, offset, len);
    }

    auto FileOutputStream::write(const std::byte *buffer, const size_t length) -> void {
        if (length == 0) {
            return;
        }

        if (!buffer) {
throw std::invalid_argument("Buffer cannot be null");
        }
checkStreamWritable("Cannot write to closed or unwritable stream.");
        checkStreamState();

        file_stream_.write(reinterpret_cast<const char *>(buffer), static_cast<std::streamsize>(length));

        checkStreamState();
    }

    auto FileOutputStream::flush() -> void {
        checkStreamWritable("Cannot flush closed or unwritable stream.");
        checkStreamState();
file_stream_.flush();
        checkStreamState();
    }

    auto FileOutputStream::close() -> void {
        checkStreamWritable("Cannot close closed or unwritable stream.");
file_stream_.close();
    }

    [[nodiscard]] auto FileOutputStream::isClosed() const -> bool {
        return !file_stream_.is_open();
    }

    auto FileOutputStream::checkStreamWritable(const std::string &message) const -> void {
        if (!file_stream_.is_open()) {
            throw std::ios_base::failure(message);
        }
    }

    auto FileOutputStream::checkStreamState() const -> void {
        if (file_stream_.bad()) {
throw std::ios_base::failure("IOException: Stream is in bad state.");
        }
        if (file_stream_.fail()) {
throw std::ios_base::failure("IOException: Stream operation failed.");
        }
    }
}
