/**
 * @file File.hpp
 * @brief File class declaration
 * @details This header defines the File class that provides functionality for Common library utilities.
 */

#pragma once
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

namespace common::filesystem
{
    class File final
    {
    public:
        explicit File(const std::string& path);
        explicit File(std::filesystem::path path);
        explicit File(const char* path);
        ~File();

        [[nodiscard]] bool canExecute() const;
        [[nodiscard]] bool canRead() const;
        [[nodiscard]] bool canWrite() const;
        [[nodiscard]] bool exists() const;
        [[nodiscard]] bool isFile() const;
        [[nodiscard]] bool isHidden() const;
        [[nodiscard]] bool isAbsolute() const;

        [[nodiscard]] bool createNewFile() const;
        [[nodiscard]] bool deleteFile() const;
        [[nodiscard]] bool renameTo(const File& dest) const;
        [[nodiscard]] bool copyTo(const File& dest) const;

        [[nodiscard]] int64_t length() const;
        [[nodiscard]] int64_t lastModified() const;
        [[nodiscard]] bool setLastModified(int64_t time) const;
        [[nodiscard]] bool setReadOnly() const;

        [[nodiscard]] std::string getAbsolutePath() const;
        [[nodiscard]] File getAbsoluteFile() const;
        [[nodiscard]] std::string getName() const;
        [[nodiscard]] std::string getExtension() const;
        [[nodiscard]] std::string getParent() const;
        [[nodiscard]] File getParentFile() const;
        [[nodiscard]] std::string getPath() const;

        [[nodiscard]] int64_t getTotalSpace() const;
        [[nodiscard]] int64_t getUsableSpace() const;
        [[nodiscard]] std::string getSizeString() const;

        [[nodiscard]] size_t hashCode() const;
        [[nodiscard]] std::string toURI() const;

        static void printFilesWithDepth(const std::filesystem::path& file_path);
        static std::string getFileMD5(const std::filesystem::path& filePath);

    private:
        std::filesystem::path file_path_{};
    };
}