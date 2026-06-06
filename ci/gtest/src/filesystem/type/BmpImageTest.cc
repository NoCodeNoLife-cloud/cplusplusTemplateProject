/**
 * @file BmpImageTest.cc
 * @brief Unit tests for the BmpImage class
 * @details Tests cover BMP creation, pixel manipulation, and file save/load round-trip.
 */

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "filesystem/type/BmpImage.hpp"

using namespace common::filesystem;

class BmpImageTest : public testing::Test
{
protected:
    std::string tmpPath_;

    void SetUp() override
    {
        tmpPath_ = std::tmpnam(nullptr);
    }

    void TearDown() override
    {
        std::remove(tmpPath_.c_str());
    }
};

TEST_F(BmpImageTest, CreateValidImage)
{
    BmpImage img(10, 10);
    EXPECT_EQ(img.getWidth(), 10);
    EXPECT_EQ(img.getHeight(), 10);
}

TEST_F(BmpImageTest, CreateInvalidWidth)
{
    EXPECT_THROW(BmpImage(0, 10), std::invalid_argument);
    EXPECT_THROW(BmpImage(-1, 10), std::invalid_argument);
}

TEST_F(BmpImageTest, CreateInvalidHeight)
{
    EXPECT_THROW(BmpImage(10, 0), std::invalid_argument);
    EXPECT_THROW(BmpImage(10, -1), std::invalid_argument);
}

TEST_F(BmpImageTest, SetAndGetPixel)
{
    BmpImage img(3, 3);
    img.setPixel(0, 0, 255, 0, 0);
    img.setPixel(1, 1, 0, 255, 0);
    img.setPixel(2, 2, 0, 0, 255);

    uint8_t r, g, b;
    EXPECT_TRUE(img.getPixel(0, 0, r, g, b));
    EXPECT_EQ(r, 255);
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 0);

    EXPECT_TRUE(img.getPixel(1, 1, r, g, b));
    EXPECT_EQ(r, 0);
    EXPECT_EQ(g, 255);
    EXPECT_EQ(b, 0);

    EXPECT_TRUE(img.getPixel(2, 2, r, g, b));
    EXPECT_EQ(r, 0);
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 255);
}

TEST_F(BmpImageTest, GetPixelOutOfBounds)
{
    BmpImage img(5, 5);
    uint8_t r, g, b;
    EXPECT_FALSE(img.getPixel(-1, 0, r, g, b));
    EXPECT_FALSE(img.getPixel(0, -1, r, g, b));
    EXPECT_FALSE(img.getPixel(5, 0, r, g, b));
    EXPECT_FALSE(img.getPixel(0, 5, r, g, b));
}

TEST_F(BmpImageTest, SetPixelOutOfBoundsDoesNotCrash)
{
    BmpImage img(5, 5);
    EXPECT_NO_THROW(img.setPixel(-1, 0, 255, 0, 0));
    EXPECT_NO_THROW(img.setPixel(0, -1, 255, 0, 0));
    EXPECT_NO_THROW(img.setPixel(10, 0, 255, 0, 0));
    EXPECT_NO_THROW(img.setPixel(0, 10, 255, 0, 0));
}

TEST_F(BmpImageTest, SaveAndLoadRoundTrip)
{
    {
        BmpImage img(2, 2);
        img.setPixel(0, 0, 255, 0, 0);
        img.setPixel(1, 0, 0, 255, 0);
        img.setPixel(0, 1, 0, 0, 255);
        img.setPixel(1, 1, 128, 128, 128);
        img.save(tmpPath_);
    }

    BmpImage loaded(tmpPath_);
    EXPECT_EQ(loaded.getWidth(), 2);
    EXPECT_EQ(loaded.getHeight(), 2);

    uint8_t r, g, b;
    EXPECT_TRUE(loaded.getPixel(0, 0, r, g, b));
    EXPECT_EQ(r, 255);
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 0);

    EXPECT_TRUE(loaded.getPixel(1, 0, r, g, b));
    EXPECT_EQ(r, 0);
    EXPECT_EQ(g, 255);
    EXPECT_EQ(b, 0);

    EXPECT_TRUE(loaded.getPixel(0, 1, r, g, b));
    EXPECT_EQ(r, 0);
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 255);
}

TEST_F(BmpImageTest, LoadNonExistentFile)
{
    EXPECT_THROW(BmpImage("nonexistent_file.bmp"), std::runtime_error);
}

TEST_F(BmpImageTest, LoadInvalidFile)
{
    {
        std::ofstream f(tmpPath_);
        f << "not a bmp file" << std::endl;
    }
    EXPECT_THROW(BmpImage{tmpPath_}, std::runtime_error);
}

TEST_F(BmpImageTest, DefaultPixelsAreBlack)
{
    BmpImage img(4, 4);
    uint8_t r, g, b;
    EXPECT_TRUE(img.getPixel(2, 2, r, g, b));
    EXPECT_EQ(r, 0);
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 0);
}
