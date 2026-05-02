#include "src/filesystem/type/CsvFile.hpp"

#include <fmt/format.h>

namespace common::filesystem {
    CsvFile::CsvFile(const std::string &file_path) noexcept : file_path_(file_path) {
        try {
csv_doc_ = rapidcsv::Document(file_path);
            is_valid_ = true;
} catch (const std::exception &e) {
            // Error silently handled as the function is marked noexcept
            // Client code should check validity through other methods
is_valid_ = false;
        }
    }

    auto CsvFile::getRowCount() const noexcept -> uint64_t {
        if (!is_valid_) return 0;
        return csv_doc_.GetRowCount();
    }

    auto CsvFile::getColumnCount() const noexcept -> uint64_t {
        if (!is_valid_) return 0;
        return csv_doc_.GetColumnCount();
    }

    auto CsvFile::insertRow(const uint64_t insertIndex, const std::vector<std::string> &item) noexcept -> bool {
        if (!is_valid_) return false;
        if (insertIndex > getRowCount()) {
return false;
        }
        if (getColumnCount() != 0 && item.size() != getColumnCount()) {
return false;
        }
        try {
csv_doc_.InsertRow(insertIndex, item);
            return true;
        } catch (const std::exception &e) {
return false;
        }
    }

    auto CsvFile::pushBack(const std::vector<std::string> &item) noexcept -> bool {
        return insertRow(getRowCount(), item);
    }

    auto CsvFile::save(const std::string &path) noexcept -> bool {
        if (!is_valid_) return false;
        try {
            if (path.empty()) {
csv_doc_.Save();
            } else {
csv_doc_.Save(path);
            }
            return true;
        } catch (const std::exception &e) {
return false;
        }
    }
}
