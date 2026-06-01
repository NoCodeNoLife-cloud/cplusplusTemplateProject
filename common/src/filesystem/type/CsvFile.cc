/**
 * @file CsvFile.cc
 * @brief CsvFile class implementation
 * @details This file contains the implementation of the CsvFile class methods for Common library utilities.
 */

#include "filesystem/type/CsvFile.hpp"

#include <fmt/format.h>
#include <glog/logging.h>

namespace common::filesystem
{
    CsvFile::CsvFile(const std::string& file_path)  : file_path_(file_path)
    {
        try
        {
            csv_doc_ = rapidcsv::Document(file_path);
            is_valid_ = true;
        }
        catch (const std::exception& e)
        {
            DLOG(WARNING) << fmt::format("Failed to load CSV file: {} - {}", file_path, e.what());
            // Error silently handled as the function is marked
            // Client code should check validity through other methods
            is_valid_ = false;
        }
    }

    uint64_t CsvFile::getRowCount() const
    {
        if (!is_valid_) return 0;
        return csv_doc_.GetRowCount();
    }

    uint64_t CsvFile::getColumnCount() const
    {
        if (!is_valid_) return 0;
        return csv_doc_.GetColumnCount();
    }

    bool CsvFile::insertRow(const uint64_t insertIndex, const std::vector<std::string>& item)
    {
        if (!is_valid_) return false;
        if (insertIndex > getRowCount())
        {
            return false;
        }
        if (getColumnCount() != 0 && item.size() != getColumnCount())
        {
            return false;
        }
        try
        {
            csv_doc_.InsertRow(insertIndex, item);
            return true;
        }
        catch (const std::exception& e)
        {
            return false;
        }
    }

    bool CsvFile::pushBack(const std::vector<std::string>& item)
    {
        return insertRow(getRowCount(), item);
    }

    bool CsvFile::save(const std::string& path)
    {
        if (!is_valid_) return false;
        try
        {
            if (path.empty())
            {
                csv_doc_.Save();
            }
            else
            {
                csv_doc_.Save(path);
            }
            return true;
        }
        catch (const std::exception& e)
        {
            return false;
        }
    }
}
