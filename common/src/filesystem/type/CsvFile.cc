#include "src/filesystem/type/CsvFile.hpp"

#include <glog/logging.h>
#include <fmt/format.h>

namespace common::filesystem {
    CsvFile::CsvFile(const std::string &file_path) noexcept : file_path_(file_path) {
        try {
            DLOG(INFO) << fmt::format("CsvFile loading - path: {}", file_path);
            csv_doc_ = rapidcsv::Document(file_path);
            is_valid_ = true;
            DLOG(INFO) << fmt::format("CsvFile loaded successfully - rows={}, columns={}", getRowCount(), getColumnCount());
        } catch (const std::exception &e) {
            // Error silently handled as the function is marked noexcept
            // Client code should check validity through other methods
            DLOG(ERROR) << fmt::format("CsvFile initialization failed - error loading file: {}, reason: {}", file_path, e.what());
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
            DLOG(ERROR) << fmt::format("CsvFile insertRow failed - index out of range: index={}, row_count={}", insertIndex, getRowCount());
            return false;
        }
        if (getColumnCount() != 0 && item.size() != getColumnCount()) {
            DLOG(ERROR) << fmt::format("CsvFile insertRow failed - column count mismatch: expected={}, actual={}", getColumnCount(), item.size());
            return false;
        }
        try {
            DLOG(INFO) << fmt::format("CsvFile insertRow - inserting at index {}", insertIndex);
            csv_doc_.InsertRow(insertIndex, item);
            return true;
        } catch (const std::exception &e) {
            DLOG(ERROR) << fmt::format("CsvFile insertRow failed - exception: {}", e.what());
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
                DLOG(INFO) << fmt::format("CsvFile save - saving to original path: {}", file_path_);
                csv_doc_.Save();
            } else {
                DLOG(INFO) << fmt::format("CsvFile save - saving to new path: {}", path);
                csv_doc_.Save(path);
            }
            return true;
        } catch (const std::exception &e) {
            DLOG(ERROR) << fmt::format("CsvFile save failed - exception: {}", e.what());
            return false;
        }
    }
}
