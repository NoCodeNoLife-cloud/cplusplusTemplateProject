/**
 * @file CsvFile.hpp
 * @brief CsvFile class declaration
 * @details This header defines the CsvFile class that provides functionality for reading,
 *          modifying, and saving CSV files.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace common::filesystem::type
{
    /// @brief A class to handle CSV file operations
    /// @details This class provides functionalities to read, modify and save CSV files.
    ///          It supports quoted fields, custom delimiters, and proper CSV escaping.
    class CsvFile
    {
    public:
        /// @brief Constructs a CsvFile object from a file path
        explicit CsvFile(const std::string& file_path);

        /// @brief Get the number of rows in the CSV file
        [[nodiscard]] uint64_t getRowCount() const;

        /// @brief Get the number of columns in the CSV file
        [[nodiscard]] uint64_t getColumnCount() const;

        /// @brief Insert a row at the specified index
        /// @param insertIndex The index where the row should be inserted
        /// @param item The vector of strings representing the row data
        /// @return True if successful, false otherwise
        bool insertRow(uint64_t insertIndex, const std::vector<std::string>& item);

        /// @brief Add a row to the end of the CSV file
        bool pushBack(const std::vector<std::string>& item);

        /// @brief Save the CSV file to disk
        /// @param path The path where to save the file (uses original path if empty)
        /// @return True if successful, false otherwise
        bool save(const std::string& path = {});

    private:
        std::string file_path_{};
        std::vector<std::string> header_{};
        std::vector<std::vector<std::string>> rows_{};
        uint64_t column_count_{0};
        bool is_valid_{false};
    };
}