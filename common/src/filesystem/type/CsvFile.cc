/**
 * @file CsvFile.cc
 * @brief CsvFile class implementation
 * @details This file contains the implementation of the CsvFile class methods.
 */

#include "filesystem/type/CsvFile.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <fmt/format.h>
#include <glog/logging.h>

namespace
{
    std::vector<std::string> parseLine(const std::string& line)
    {
        std::vector<std::string> fields;
        std::string field;
        bool inQuotes = false;

        for (size_t i = 0; i < line.size(); ++i)
        {
            const char c = line[i];

            if (inQuotes)
            {
                if (c == '"')
                {
                    if (i + 1 < line.size() && line[i + 1] == '"')
                    {
                        field += '"';
                        ++i;
                    }
                    else
                    {
                        inQuotes = false;
                    }
                }
                else
                {
                    field += c;
                }
            }
            else
            {
                if (c == '"')
                {
                    inQuotes = true;
                }
                else if (c == ',')
                {
                    fields.push_back(field);
                    field.clear();
                }
                else
                {
                    field += c;
                }
            }
        }

        fields.push_back(field);
        return fields;
    }

    std::string escapeField(const std::string& field)
    {
        if (field.find(',') == std::string::npos &&
            field.find('"') == std::string::npos &&
            field.find('\n') == std::string::npos)
        {
            return field;
        }

        std::string escaped;
        escaped += '"';
        for (const char c : field)
        {
            if (c == '"')
            {
                escaped += "\"\"";
            }
            else
            {
                escaped += c;
            }
        }
        escaped += '"';
        return escaped;
    }
}

namespace common::filesystem::type
{
    CsvFile::CsvFile(const std::string& file_path) : file_path_(file_path)
    {
        std::ifstream file(file_path);
        if (!file)
        {
            is_valid_ = false;
            DLOG(WARNING) << fmt::format("Failed to open CSV file: {}", file_path);
            return;
        }

        std::string line;
        bool headerRead = false;
        while (std::getline(file, line))
        {
            if (line.empty())
            {
                continue;
            }
            auto fields = parseLine(line);
            if (!headerRead)
            {
                header_ = fields;
                column_count_ = static_cast<uint64_t>(fields.size());
                headerRead = true;
            }
            else
            {
                rows_.push_back(std::move(fields));
            }
        }

        is_valid_ = true;
    }

    uint64_t CsvFile::getRowCount() const
    {
        return is_valid_ ? static_cast<uint64_t>(rows_.size()) : 0;
    }

    uint64_t CsvFile::getColumnCount() const
    {
        return is_valid_ ? column_count_ : 0;
    }

    bool CsvFile::insertRow(const uint64_t insertIndex, const std::vector<std::string>& item)
    {
        if (!is_valid_ || insertIndex > rows_.size())
        {
            return false;
        }
        if (column_count_ != 0 && item.size() != column_count_)
        {
            return false;
        }

        rows_.insert(rows_.begin() + static_cast<std::ptrdiff_t>(insertIndex), item);
        if (column_count_ == 0)
        {
            column_count_ = static_cast<uint64_t>(item.size());
        }
        return true;
    }

    bool CsvFile::pushBack(const std::vector<std::string>& item)
    {
        return insertRow(static_cast<uint64_t>(rows_.size()), item);
    }

    bool CsvFile::save(const std::string& path)
    {
        if (!is_valid_)
        {
            return false;
        }

        const std::string& outPath = path.empty() ? file_path_ : path;

        std::ofstream file(outPath);
        if (!file)
        {
            return false;
        }

        for (size_t j = 0; j < header_.size(); ++j)
        {
            if (j > 0)
            {
                file << ',';
            }
            file << escapeField(header_[j]);
        }
        file << '\n';

        for (auto & row : rows_)
        {
            for (size_t j = 0; j < row.size(); ++j)
            {
                if (j > 0)
                {
                    file << ',';
                }
                file << escapeField(row[j]);
            }
            file << '\n';
        }

        return true;
    }
}
