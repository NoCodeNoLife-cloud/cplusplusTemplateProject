#include "src/filesystem/io/reader/FileInputStream.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace common::filesystem {
    FileInputStream::FileInputStream(const std::string &name) {
        DLOG(INFO) << fmt::format("FileInputStream opening file: {}", name);
        if (!std::filesystem::exists(name)) {
            DLOG(ERROR) << fmt::format("FileInputStream failed - file does not exist: {}", name);
            throw std::invalid_argument("FileInputStream::FileInputStream: File does not exist - " + name);
        }
        if (std::filesystem::is_directory(name)) {
            DLOG(ERROR) << fmt::format("FileInputStream failed - path is a directory: {}", name);
            throw std::invalid_argument("FileInputStream::FileInputStream: Path is a directory - " + name);
        }
        file_stream_.open(name, std::ios::binary);
        if (!file_stream_.is_open()) {
            DLOG(ERROR) << fmt::format("FileInputStream failed - unable to open file: {}", name);
            throw std::ios_base::failure("FileInputStream::FileInputStream: Unable to open file - " + name);
        }
        file_name_ = name;
        DLOG(INFO) << fmt::format("FileInputStream successfully opened: {}", name);
    }

    FileInputStream::FileInputStream(const char *name) : FileInputStream(std::string(name)) {
    }

    FileInputStream::FileInputStream(const std::filesystem::path &file) : FileInputStream(file.string()) {
    }

    FileInputStream::~FileInputStream() {
        close();
    }

    auto FileInputStream::isValid() const noexcept -> bool {
        return !closed_ && file_stream_.good();
    }

    auto FileInputStream::validateBufferParams(const std::vector<std::byte> &buffer, const size_t offset, const size_t len) -> void {
        if (offset > buffer.size() || len > buffer.size() - offset) {
            throw std::out_of_range("FileInputStream::validateBufferParams: Invalid buffer, offset, or length - offset: " + std::to_string(offset) + ", len: " + std::to_string(len) + ", buffer_size: " + std::to_string(buffer.size()));
        }
    }

    auto FileInputStream::read() -> std::byte {
        if (!isValid()) {
            DLOG(INFO) << "FileInputStream read - stream not valid or closed";
            return static_cast<std::byte>(-1);
        }

        std::byte byte{};
        if (file_stream_.read(reinterpret_cast<char *>(&byte), 1)) {
            return byte;
        }
        DLOG(INFO) << "FileInputStream read - end of file reached";
        return static_cast<std::byte>(-1);
    }

    auto FileInputStream::read(std::vector<std::byte> &buffer) -> size_t {
        return read(buffer, 0, buffer.size());
    }

    auto FileInputStream::read(std::vector<std::byte> &buffer, const size_t offset, const size_t len) -> size_t {
        validateBufferParams(buffer, offset, len);

        if (!isValid()) {
            DLOG(INFO) << "FileInputStream read - stream not valid or closed";
            return 0;
        }

        DLOG(INFO) << fmt::format("FileInputStream read - requested {} bytes at offset {}", len, offset);
        file_stream_.read(reinterpret_cast<char *>(buffer.data() + offset), static_cast<std::streamsize>(len));
        const auto bytes_read = file_stream_.gcount();
        DLOG(INFO) << fmt::format("FileInputStream read completed - bytes read: {}", bytes_read);
        return static_cast<size_t>(bytes_read);
    }

    auto FileInputStream::skip(const size_t n) -> size_t {
        if (!isValid()) {
            return 0;
        }

        const auto current_pos = file_stream_.tellg();
        if (current_pos == std::streampos(-1)) {
            return 0;
        }

        file_stream_.seekg(static_cast<std::streamoff>(n), std::ios::cur);
        if (!file_stream_.good()) {
            // Restore the original position if seek failed
            file_stream_.seekg(current_pos, std::ios::beg);
            return 0;
        }

        const auto new_pos = file_stream_.tellg();
        if (new_pos == std::streampos(-1)) {
            return 0;
        }

        const auto skipped = new_pos - current_pos;
        return static_cast<size_t>(skipped);
    }

    auto FileInputStream::available() -> size_t {
        if (!isValid()) {
            return 0;
        }

        const auto current = file_stream_.tellg();
        if (current == std::streampos(-1)) {
            return 0;
        }

        file_stream_.seekg(0, std::ios::end);
        const auto end = file_stream_.tellg();
        file_stream_.seekg(current, std::ios::beg);

        if (end == std::streampos(-1)) {
            return 0;
        }

        const auto available_bytes = end - current;
        return static_cast<size_t>(std::max(available_bytes, static_cast<std::streamoff>(0)));
    }

    auto FileInputStream::close() -> void {
        if (file_stream_.is_open()) {
            DLOG(INFO) << fmt::format("FileInputStream closing file: {}", file_name_);
            file_stream_.close();
        }
        closed_ = true;
    }

    [[nodiscard]] auto FileInputStream::isClosed() const -> bool {
        return closed_;
    }

    [[nodiscard]] auto FileInputStream::markSupported() const -> bool {
        return false;
    }
}
