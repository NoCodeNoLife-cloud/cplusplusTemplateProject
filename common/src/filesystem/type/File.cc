#include "src/filesystem/type/File.hpp"
#include <windows.h>
#include <openssl/evp.h>


#include <fmt/format.h>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace common::filesystem {
    File::File(const std::string &path) : file_path_(path) {
    }

    File::File(std::filesystem::path path) : file_path_(std::move(path)) {
    }

    File::File(const char *path) : file_path_(path) {
    }

    File::~File() = default;

    auto File::canExecute() const noexcept -> bool {
        const DWORD attributes = GetFileAttributesW(file_path_.c_str());
        return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    auto File::canRead() const noexcept -> bool {
        const std::ifstream file(file_path_);
        return file.good();
    }

    auto File::canWrite() const noexcept -> bool {
        const std::ofstream file(file_path_, std::ios::app);
        return file.good();
    }

    auto File::exists() const noexcept -> bool {
        try {
            return std::filesystem::exists(file_path_);
        } catch (...) {
            return false;
        }
    }

    auto File::isFile() const noexcept -> bool {
        try {
            return std::filesystem::is_regular_file(file_path_);
        } catch (...) {
            return false;
        }
    }

    auto File::isHidden() const noexcept -> bool {
        const DWORD attributes = GetFileAttributesW(file_path_.c_str());
        return attributes != INVALID_FILE_ATTRIBUTES && attributes & FILE_ATTRIBUTE_HIDDEN;
    }

    auto File::isAbsolute() const noexcept -> bool {
        return file_path_.is_absolute();
    }

    auto File::createNewFile() const -> bool {
        if (std::filesystem::exists(file_path_)) {
            return false;
        }
        const std::ofstream file(file_path_);
        return file.good();
    }

    auto File::deleteFile() const noexcept -> bool {
        try {
            return std::filesystem::remove(file_path_);
        } catch (...) {
            return false;
        }
    }

    auto File::renameTo(const File &dest) const noexcept -> bool {
        try {
            std::filesystem::rename(file_path_, dest.file_path_);
            return true;
        } catch (const std::filesystem::filesystem_error &) {
            return false;
        }
    }

    auto File::copyTo(const File &dest) const -> bool {
        try {
            std::filesystem::copy_file(file_path_, dest.file_path_, std::filesystem::copy_options::overwrite_existing);
            return true;
        } catch (const std::filesystem::filesystem_error &) {
            return false;
        }
    }

    auto File::length() const noexcept -> int64_t {
        try {
            if (std::filesystem::exists(file_path_) && std::filesystem::is_regular_file(file_path_)) {
                return static_cast<int64_t>(std::filesystem::file_size(file_path_));
            }
            return 0;
        } catch (...) {
            return 0;
        }
    }

    auto File::lastModified() const noexcept -> int64_t {
        try {
            const auto lastWriteTime = std::filesystem::last_write_time(file_path_);
            const auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(lastWriteTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
            return std::chrono::system_clock::to_time_t(sctp);
        } catch (...) {
            return 0;
        }
    }

    auto File::setLastModified(const int64_t time) const noexcept -> bool {
        try {
            const auto timePoint = std::chrono::system_clock::from_time_t(time);
            const auto fileTime = std::chrono::time_point_cast<std::filesystem::file_time_type::duration>(timePoint - std::chrono::system_clock::now() + std::filesystem::file_time_type::clock::now());
            std::filesystem::last_write_time(file_path_, fileTime);
            return true;
        } catch (...) {
            return false;
        }
    }

    auto File::setReadOnly() const noexcept -> bool {
        try {
            const DWORD attributes = GetFileAttributesW(file_path_.c_str());
            if (attributes == INVALID_FILE_ATTRIBUTES) {
                return false;
            }

            if (SetFileAttributesW(file_path_.c_str(), attributes | FILE_ATTRIBUTE_READONLY)) {
                return true;
            }
            return false;
        } catch (...) {
            return false;
        }
    }

    auto File::getAbsolutePath() const -> std::string {
        try {
            return std::filesystem::absolute(file_path_).string();
        } catch (...) {
            return file_path_.string();
        }
    }

    auto File::getAbsoluteFile() const -> File {
        try {
            return File(std::filesystem::absolute(file_path_).string());
        } catch (...) {
            return File(file_path_.string());
        }
    }

    auto File::getName() const noexcept -> std::string {
        return file_path_.filename().string();
    }

    auto File::getExtension() const noexcept -> std::string {
        try {
            const auto extension = file_path_.extension();
            return extension.string();
        } catch (...) {
            return std::string{};
        }
    }

    auto File::getParent() const noexcept -> std::string {
        return file_path_.parent_path().string();
    }

    auto File::getParentFile() const -> File {
        return file_path_.has_parent_path() ? File(file_path_.parent_path()) : File(std::string(""));
    }

    auto File::getPath() const noexcept -> std::string {
        return file_path_.string();
    }

    auto File::getTotalSpace() const noexcept -> int64_t {
        try {
            const auto spaceInfo = std::filesystem::space(file_path_);
            return static_cast<int64_t>(spaceInfo.capacity);
        } catch (...) {
            return 0;
        }
    }

    auto File::getUsableSpace() const noexcept -> int64_t {
        try {
            const auto spaceInfo = std::filesystem::space(file_path_);
            return static_cast<int64_t>(spaceInfo.free);
        } catch (...) {
            return 0;
        }
    }

    auto File::getSizeString() const noexcept -> std::string {
        const auto size = length();
        if (size < 0) {
            return "Unknown";
        }

        constexpr int64_t KB = 1024;
        constexpr int64_t MB = KB * 1024;
        constexpr int64_t GB = MB * 1024;

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);

        if (size >= GB) {
            oss << static_cast<double>(size) / GB << " GB";
        } else if (size >= MB) {
            oss << static_cast<double>(size) / MB << " MB";
        } else if (size >= KB) {
            oss << static_cast<double>(size) / KB << " KB";
        } else {
            oss << size << " B";
        }

        return oss.str();
    }

    auto File::hashCode() const noexcept -> size_t {
        return std::hash<std::string>{}(file_path_.string());
    }

    auto File::toURI() const noexcept -> std::string {
        return "file://" + file_path_.string();
    }

    auto File::printFilesWithDepth(const std::filesystem::path &file_path) -> void {
        if (!std::filesystem::exists(file_path) || !std::filesystem::is_directory(file_path)) {
            throw std::runtime_error("Invalid directory path: " + file_path.string());
        }

        for (auto it_entry = std::filesystem::recursive_directory_iterator(file_path); it_entry != std::filesystem::recursive_directory_iterator{}; ++it_entry) {
            const auto &entry = *it_entry;
            const auto depth = it_entry.depth();

            std::cout << std::string(static_cast<size_t>(depth * 2), ' ');

            if (entry.is_directory()) {
                std::cout << "[DIR]" << entry.path().filename().string() << std::endl;
            } else {
                std::cout << "[FILE]" << entry.path().filename().string() << std::endl;
            }
        }
    }

    auto File::getFileMD5(const std::filesystem::path &filePath) -> std::string {
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open file: " + filePath.string());
        }

        const auto mdContext = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>(EVP_MD_CTX_new(), EVP_MD_CTX_free);
        if (!mdContext) {
            throw std::runtime_error("Failed to create MD5 context");
        }

        if (EVP_DigestInit_ex(mdContext.get(), EVP_md5(), nullptr) != 1) {
            throw std::runtime_error("Failed to initialize MD5 context");
        }

        constexpr size_t bufferSize = 4096;
        std::array<char, bufferSize> buffer{};
        while (file.read(buffer.data(), bufferSize)) {
            if (const auto bytesRead = static_cast<size_t>(file.gcount()); EVP_DigestUpdate(mdContext.get(), buffer.data(), bytesRead) != 1) {
                throw std::runtime_error("MD5 update failed");
            }
        }
        if (const auto bytesRead = static_cast<size_t>(file.gcount()); bytesRead > 0) {
            if (EVP_DigestUpdate(mdContext.get(), buffer.data(), bytesRead) != 1) {
                throw std::runtime_error("MD5 update failed");
            }
        }

        std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
        unsigned int digestLength = 0;
        if (EVP_DigestFinal_ex(mdContext.get(), digest.data(), &digestLength) != 1) {
            throw std::runtime_error("MD5 finalization failed");
        }

        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (unsigned int i = 0; i < digestLength; ++i) {
            oss << std::setw(2) << static_cast<unsigned>(digest[i]);
        }

        return oss.str();
    }
}
