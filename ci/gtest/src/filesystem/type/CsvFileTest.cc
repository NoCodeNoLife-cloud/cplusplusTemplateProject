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
