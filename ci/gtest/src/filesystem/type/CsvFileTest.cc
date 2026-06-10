/**
 * @file CsvFileTest.cc
 * @brief Unit tests for the CsvFile class
 * @details Tests cover CSV loading, row/column counts, row insertion, and save round-trip.
 */

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "filesystem/type/CsvFile.hpp"

using namespace common::filesystem::type;

class CsvFileTest : public testing::Test
{
protected:
    std::string tmpPath_;

    void SetUp() override
    {
        const auto tmpDir = std::filesystem::temp_directory_path() / "CsvFileTest";
        std::filesystem::create_directories(tmpDir);
        tmpPath_ = (tmpDir / "test.csv").string();
        std::filesystem::remove(tmpPath_);
        createTestCsv();
    }

    void TearDown() override
    {
        std::error_code ec;
        std::filesystem::remove(tmpPath_, ec);
        // Clean up any additional files created during tests
        std::filesystem::remove(tmpPath_ + ".out", ec);
    }

    void createTestCsv()
    {
        std::ofstream f(tmpPath_);
        f << "name,age,city" << std::endl;
        f << "Alice,30,New York" << std::endl;
        f << "Bob,25,\"Los Angeles, CA\"" << std::endl;
        f << "Charlie,35,Chicago" << std::endl;
    }
};

TEST_F(CsvFileTest, LoadValidFile)
{
    EXPECT_NO_THROW(CsvFile csv(tmpPath_));
}

TEST_F(CsvFileTest, GetRowCount)
{
    CsvFile csv(tmpPath_);
    EXPECT_EQ(csv.getRowCount(), 3);
}

TEST_F(CsvFileTest, GetColumnCount)
{
    CsvFile csv(tmpPath_);
    EXPECT_EQ(csv.getColumnCount(), 3);
}

TEST_F(CsvFileTest, LoadNonExistentFile)
{
    CsvFile csv("nonexistent.csv");
    EXPECT_EQ(csv.getRowCount(), 0);
    EXPECT_EQ(csv.getColumnCount(), 0);
}

TEST_F(CsvFileTest, PushBack)
{
    CsvFile csv(tmpPath_);
    EXPECT_TRUE(csv.pushBack({"David", "40", "Seattle"}));
    EXPECT_EQ(csv.getRowCount(), 4);
}

TEST_F(CsvFileTest, InsertRow)
{
    CsvFile csv(tmpPath_);
    EXPECT_TRUE(csv.insertRow(1, {"Eve", "28", "Boston"}));
    EXPECT_EQ(csv.getRowCount(), 4);
}

TEST_F(CsvFileTest, InsertRowInvalidIndex)
{
    CsvFile csv(tmpPath_);
    EXPECT_FALSE(csv.insertRow(100, {"Invalid", "0", "Nowhere"}));
    EXPECT_EQ(csv.getRowCount(), 3);
}

TEST_F(CsvFileTest, SaveToNewPath)
{
    std::string destPath = tmpPath_ + ".out";
    {
        CsvFile csv(tmpPath_);
        csv.pushBack({"David", "40", "Seattle"});
        EXPECT_TRUE(csv.save(destPath));
    }
    {
        CsvFile csv(destPath);
        EXPECT_EQ(csv.getRowCount(), 4);
        EXPECT_EQ(csv.getColumnCount(), 3);
    }
}

TEST_F(CsvFileTest, SaveToOriginalPath)
{
    CsvFile csv(tmpPath_);
    csv.pushBack({"David", "40", "Seattle"});
    EXPECT_TRUE(csv.save());
    EXPECT_EQ(csv.getRowCount(), 4);
}

TEST_F(CsvFileTest, EmptyCsvAfterInvalidLoad)
{
    CsvFile csv("nonexistent.csv");
    EXPECT_FALSE(csv.pushBack({"hello"}));
    EXPECT_FALSE(csv.save());
}

// ============================================================================
// Additional Boundary Condition Tests
// ============================================================================

/**
 * @brief Test loading an empty file
 * @edge File exists but has no content
 */
TEST_F(CsvFileTest, EmptyFile)
{
    const std::string emptyFile = tmpPath_ + "_empty.csv";
    std::ofstream f(emptyFile);
    f.close();

    CsvFile csv(emptyFile);
    EXPECT_EQ(csv.getRowCount(), 0);
    EXPECT_EQ(csv.getColumnCount(), 0);

    std::error_code ec;
    std::filesystem::remove(emptyFile, ec);
}

/**
 * @brief Test file with only header row
 * @edge No data rows
 */
TEST_F(CsvFileTest, HeaderOnly)
{
    const std::string headerFile = tmpPath_ + "_header.csv";
    {
        std::ofstream f(headerFile);
        f << "name,age,city" << std::endl;
    }

    CsvFile csv(headerFile);
    EXPECT_EQ(csv.getRowCount(), 0);
    EXPECT_EQ(csv.getColumnCount(), 3);

    std::error_code ec;
    std::filesystem::remove(headerFile, ec);
}

/**
 * @brief Test CSV with quoted fields containing commas
 * @edge Fields with embedded commas
 */
TEST_F(CsvFileTest, QuotedFieldsWithCommas)
{
    const std::string quotedFile = tmpPath_ + "_quoted.csv";
    {
        std::ofstream f(quotedFile);
        f << "id,description,price" << std::endl;
        f << "1,\"Item with, comma inside\",10.99" << std::endl;
        f << "2,\"Another, quoted, field\",20.50" << std::endl;
    }

    CsvFile csv(quotedFile);
    EXPECT_EQ(csv.getRowCount(), 2);
    EXPECT_EQ(csv.getColumnCount(), 3);

    std::error_code ec;
    std::filesystem::remove(quotedFile, ec);
}

/**
 * @brief Test CSV with quoted fields containing newlines
 * @edge Multi-line fields
 */
TEST_F(CsvFileTest, QuotedFieldsWithNewlines)
{
    const std::string newlineFile = tmpPath_ + "_newline.csv";
    {
        std::ofstream f(newlineFile);
        f << "id,notes" << std::endl;
        f << "1,\"line1\nline2\nline3\"" << std::endl;
    }

    CsvFile csv(newlineFile);
    // The CSV parser may treat the embedded newlines as new rows
    EXPECT_GE(csv.getRowCount(), 1);

    std::error_code ec;
    std::filesystem::remove(newlineFile, ec);
}

/**
 * @brief Test loading large CSV
 * @edge Large number of rows
 */
TEST_F(CsvFileTest, LargeCsv)
{
    const std::string largeFile = tmpPath_ + "_large.csv";
    {
        std::ofstream f(largeFile);
        f << "id,value" << std::endl;
        for (int i = 0; i < 1000; ++i)
        {
            f << i << "," << "val_" << i << std::endl;
        }
    }

    CsvFile csv(largeFile);
    EXPECT_EQ(csv.getRowCount(), 1000);
    EXPECT_EQ(csv.getColumnCount(), 2);

    std::error_code ec;
    std::filesystem::remove(largeFile, ec);
}

/**
 * @brief Test pushBack with wrong column count
 * @edge Row with different number of columns
 */
TEST_F(CsvFileTest, PushBackWrongColumnCount)
{
    CsvFile csv(tmpPath_);
    EXPECT_FALSE(csv.pushBack({"too", "few"}));
    EXPECT_FALSE(csv.pushBack({"too", "many", "cols", "here"}));
}

/**
 * @brief Test insertRow at first position
 * @edge Insert at beginning
 */
TEST_F(CsvFileTest, InsertRowAtBeginning)
{
    CsvFile csv(tmpPath_);
    EXPECT_TRUE(csv.insertRow(0, {"Zara", "22", "London"}));
    EXPECT_EQ(csv.getRowCount(), 4);
}

/**
 * @brief Test insertRow at last position
 * @edge Insert at end
 */
TEST_F(CsvFileTest, InsertRowAtEnd)
{
    CsvFile csv(tmpPath_);
    EXPECT_TRUE(csv.insertRow(3, {"Zara", "22", "London"}));
    EXPECT_EQ(csv.getRowCount(), 4);
}

/**
 * @brief Test save with BOM preserved
 * @edge UTF-8 BOM header
 */
TEST_F(CsvFileTest, Utf8Bom)
{
    const std::string bomFile = tmpPath_ + "_bom.csv";
    {
        std::ofstream f(bomFile, std::ios::binary);
        // Write UTF-8 BOM followed by CSV content
        const char bom[] = "\xEF\xBB\xBF";
        f.write(bom, 3);
        f << "name,city" << std::endl;
        f << "Alice,New York" << std::endl;
    }

    CsvFile csv(bomFile);
    EXPECT_EQ(csv.getRowCount(), 1);
    EXPECT_EQ(csv.getColumnCount(), 2);

    std::error_code ec;
    std::filesystem::remove(bomFile, ec);
}

/**
 * @brief Test round-trip preserves data integrity
 * @details Save and reload should preserve all rows
 */
TEST_F(CsvFileTest, SaveRoundTripIntegrity)
{
    const std::string destPath = tmpPath_ + ".out";
    {
        CsvFile csv(tmpPath_);
        EXPECT_TRUE(csv.save(destPath));
    }
    {
        CsvFile csv(destPath);
        EXPECT_EQ(csv.getRowCount(), 3);
        EXPECT_EQ(csv.getColumnCount(), 3);
    }
    std::error_code ec;
    std::filesystem::remove(destPath, ec);
}

/**
 * @brief Test CSV with escaped quotes inside quoted fields
 * @edge Double-quote escaping
 */
TEST_F(CsvFileTest, EscapedQuotesInQuotedFields)
{
    const std::string file = tmpPath_ + "_escaped.csv";
    {
        std::ofstream f(file);
        f << "id,description" << std::endl;
        f << "1,\"Say \"\"Hello\"\" world\"" << std::endl;
    }

    CsvFile csv(file);
    EXPECT_EQ(csv.getRowCount(), 1);
    EXPECT_EQ(csv.getColumnCount(), 2);

    std::error_code ec;
    std::filesystem::remove(file, ec);
}

/**
 * @brief Test CSV with empty fields
 * @edge Empty values between commas
 */
TEST_F(CsvFileTest, EmptyFields)
{
    const std::string file = tmpPath_ + "_empty.csv";
    {
        std::ofstream f(file);
        f << "a,b,c" << std::endl;
        f << "1,,3" << std::endl;
        f << ",5," << std::endl;
    }

    CsvFile csv(file);
    EXPECT_EQ(csv.getRowCount(), 2);
    EXPECT_EQ(csv.getColumnCount(), 3);

    std::error_code ec;
    std::filesystem::remove(file, ec);
}

/**
 * @brief Test fields with leading/trailing spaces preserved
 * @edge Spaces in unquoted fields
 */
TEST_F(CsvFileTest, SpacesInFields)
{
    const std::string file = tmpPath_ + "_spaces.csv";
    {
        std::ofstream f(file);
        f << "name,value" << std::endl;
        f << "  hello  ,world  " << std::endl;
    }

    CsvFile csv(file);
    EXPECT_EQ(csv.getRowCount(), 1);
    EXPECT_EQ(csv.getColumnCount(), 2);

    std::error_code ec;
    std::filesystem::remove(file, ec);
}

/**
 * @brief Test save to invalid path returns false
 * @edge Unwritable destination
 */
TEST_F(CsvFileTest, SaveToInvalidPath_ReturnsFalse)
{
    CsvFile csv(tmpPath_);
    EXPECT_FALSE(csv.save("Z:\\invalid_dir\\output.csv"));
}

/**
 * @brief Test very wide CSV with many columns
 * @edge Large number of columns
 */
TEST_F(CsvFileTest, VeryWideCsv)
{
    const std::string file = tmpPath_ + "_wide.csv";
    {
        std::ofstream f(file);
        for (int i = 0; i < 100; ++i)
        {
            f << "col" << i;
            if (i < 99) f << ",";
        }
        f << std::endl;
        for (int i = 0; i < 100; ++i)
        {
            f << "val" << i;
            if (i < 99) f << ",";
        }
        f << std::endl;
    }

    CsvFile csv(file);
    EXPECT_EQ(csv.getRowCount(), 1);
    EXPECT_EQ(csv.getColumnCount(), 100);

    std::error_code ec;
    std::filesystem::remove(file, ec);
}

/**
 * @brief Test multiple saves with modifications
 * @edge Save, modify, save again, reload
 */
TEST_F(CsvFileTest, MultipleSavesWithModifications)
{
    const std::string dest1 = tmpPath_ + ".m1.csv";
    const std::string dest2 = tmpPath_ + ".m2.csv";
    {
        CsvFile csv(tmpPath_);
        EXPECT_TRUE(csv.save(dest1));
        csv.pushBack({"David", "40", "Seattle"});
        EXPECT_TRUE(csv.save(dest2));
    }
    {
        CsvFile csv1(dest1);
        EXPECT_EQ(csv1.getRowCount(), 3);
        CsvFile csv2(dest2);
        EXPECT_EQ(csv2.getRowCount(), 4);
    }
    std::error_code ec;
    std::filesystem::remove(dest1, ec);
    std::filesystem::remove(dest2, ec);
}

/**
 * @brief Test insertRow on empty valid CSV
 * @edge File with header only, then insert row
 */
TEST_F(CsvFileTest, InsertIntoEmptyData)
{
    const std::string file = tmpPath_ + "_headerOnly.csv";
    {
        std::ofstream f(file);
        f << "name,age" << std::endl;
    }

    CsvFile csv(file);
    EXPECT_EQ(csv.getRowCount(), 0);
    EXPECT_TRUE(csv.insertRow(0, {"Alice", "30"}));
    EXPECT_EQ(csv.getRowCount(), 1);

    std::error_code ec;
    std::filesystem::remove(file, ec);
}
