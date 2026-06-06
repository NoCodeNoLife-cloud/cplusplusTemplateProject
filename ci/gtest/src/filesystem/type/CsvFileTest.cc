/**
 * @file CsvFileTest.cc
 * @brief Unit tests for the CsvFile class
 * @details Tests cover CSV loading, row/column counts, row insertion, and save round-trip.
 */

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "filesystem/type/CsvFile.hpp"

using namespace common::filesystem;

class CsvFileTest : public testing::Test
{
protected:
    std::string tmpPath_;

    void SetUp() override
    {
        tmpPath_ = std::tmpnam(nullptr);
        createTestCsv();
    }

    void TearDown() override
    {
        std::remove(tmpPath_.c_str());
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
    std::remove(destPath.c_str());
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
