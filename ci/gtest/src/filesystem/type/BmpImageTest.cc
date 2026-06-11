/**
 * @file BmpImageTest.cc
 * @brief Unit tests for the BmpImage class
 * @details Tests cover BMP creation, pixel manipulation, and file save/load round-trip.
 */

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "filesystem/type/BmpImage.hpp"

using namespace common::filesystem::type;

class BmpImageTest : public testing::Test
{
protected:
    std::string tmpPath_;

    void SetUp() override
    {
        const auto tmpDir = std::filesystem::temp_directory_path() / "BmpImageTest";
        std::filesystem::create_directories(tmpDir);
        tmpPath_ = (tmpDir / "test.bmp").string();
        std::filesystem::remove(tmpPath_);
    }

    void TearDown() override
    {
        std::error_code ec;
        std::filesystem::remove(tmpPath_, ec);
    }
};

TEST_F(BmpImageTest, CreateValidImage)
{
    const BmpImage img(10, 10);
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
    const BmpImage img(5, 5);
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
        EXPECT_TRUE(img.save(tmpPath_));
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
    const BmpImage img(4, 4);
    uint8_t r, g, b;
    EXPECT_TRUE(img.getPixel(2, 2, r, g, b));
    EXPECT_EQ(r, 0);
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 0);
}

// ============================================================================
// Additional Boundary Condition Tests
// ============================================================================

TEST_F(BmpImageTest, SinglePixelImage)
{
    BmpImage img(1, 1);
    EXPECT_EQ(img.getWidth(), 1);
    EXPECT_EQ(img.getHeight(), 1);
    img.setPixel(0, 0, 128, 64, 32);
    uint8_t r, g, b;
    EXPECT_TRUE(img.getPixel(0, 0, r, g, b));
    EXPECT_EQ(r, 128);
    EXPECT_EQ(g, 64);
    EXPECT_EQ(b, 32);
}

TEST_F(BmpImageTest, SinglePixelRoundTrip)
{
    {
        BmpImage img(1, 1);
        img.setPixel(0, 0, 255, 128, 64);
        EXPECT_TRUE(img.save(tmpPath_));
    }
    const BmpImage loaded(tmpPath_);
    EXPECT_EQ(loaded.getWidth(), 1);
    EXPECT_EQ(loaded.getHeight(), 1);
    uint8_t r, g, b;
    EXPECT_TRUE(loaded.getPixel(0, 0, r, g, b));
    EXPECT_EQ(r, 255);
    EXPECT_EQ(g, 128);
    EXPECT_EQ(b, 64);
}

TEST_F(BmpImageTest, LargeImageRoundTrip)
{
    {
        BmpImage img(100, 100);
        img.setPixel(50, 50, 128, 128, 128);
        EXPECT_TRUE(img.save(tmpPath_));
    }
    const BmpImage loaded(tmpPath_);
    EXPECT_EQ(loaded.getWidth(), 100);
    EXPECT_EQ(loaded.getHeight(), 100);
    uint8_t r, g, b;
    EXPECT_TRUE(loaded.getPixel(50, 50, r, g, b));
    EXPECT_EQ(r, 128);
    EXPECT_EQ(g, 128);
    EXPECT_EQ(b, 128);
}

TEST_F(BmpImageTest, AllWhiteImage)
{
    constexpr int size = 5;
    BmpImage img(size, size);
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            img.setPixel(x, y, 255, 255, 255);
        }
    }
    EXPECT_TRUE(img.save(tmpPath_));

    const BmpImage loaded(tmpPath_);
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            uint8_t r, g, b;
            EXPECT_TRUE(loaded.getPixel(x, y, r, g, b));
            EXPECT_EQ(r, 255);
            EXPECT_EQ(g, 255);
            EXPECT_EQ(b, 255);
        }
    }
}

TEST_F(BmpImageTest, OverwriteExistingFile)
{
    {
        const BmpImage img(2, 2);
        EXPECT_TRUE(img.save(tmpPath_));
    }
    {
        const BmpImage img(4, 4);
        EXPECT_TRUE(img.save(tmpPath_));
    }
    const BmpImage loaded(tmpPath_);
    EXPECT_EQ(loaded.getWidth(), 4);
    EXPECT_EQ(loaded.getHeight(), 4);
}

TEST_F(BmpImageTest, LoadCorruptedHeader)
{
    {
        std::ofstream f(tmpPath_, std::ios::binary);
        f.put('B');
        f.put('M');
        for (int i = 0; i < 100; ++i) f.put(0x00);
    }
    EXPECT_THROW(BmpImage{tmpPath_}, std::runtime_error);
}

TEST_F(BmpImageTest, LoadTruncatedFile)
{
    {
        std::ofstream f(tmpPath_, std::ios::binary);
        f.put('B');
        f.put('M');
        const uint32_t fileSize = 54;
        f.write(reinterpret_cast<const char*>(&fileSize), 4);
        for (int i = 0; i < 48; ++i) f.put(0x00);
    }
    EXPECT_THROW(BmpImage{tmpPath_}, std::runtime_error);
}

TEST_F(BmpImageTest, AllBlackImageRoundTrip)
{
    {
        const BmpImage img(3, 3);
        EXPECT_TRUE(img.save(tmpPath_));
    }
    const BmpImage loaded(tmpPath_);
    EXPECT_EQ(loaded.getWidth(), 3);
    EXPECT_EQ(loaded.getHeight(), 3);
    uint8_t r, g, b;
    EXPECT_TRUE(loaded.getPixel(1, 1, r, g, b));
    EXPECT_EQ(r, 0);
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 0);
}

TEST_F(BmpImageTest, RectangularImage)
{
    BmpImage img(8, 4);
    EXPECT_EQ(img.getWidth(), 8);
    EXPECT_EQ(img.getHeight(), 4);
    img.setPixel(7, 3, 255, 0, 0);
    EXPECT_TRUE(img.save(tmpPath_));
    const BmpImage loaded(tmpPath_);
    EXPECT_EQ(loaded.getWidth(), 8);
    EXPECT_EQ(loaded.getHeight(), 4);
}

TEST_F(BmpImageTest, VeryLargeImageRoundTrip)
{
    {
        BmpImage img(512, 512);
        img.setPixel(0, 0, 255, 0, 0);
        img.setPixel(511, 511, 0, 0, 255);
        EXPECT_TRUE(img.save(tmpPath_));
    }
    const BmpImage loaded(tmpPath_);
    EXPECT_EQ(loaded.getWidth(), 512);
    EXPECT_EQ(loaded.getHeight(), 512);
    uint8_t r, g, b;
    EXPECT_TRUE(loaded.getPixel(0, 0, r, g, b));
    EXPECT_EQ(r, 255);
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 0);
    EXPECT_TRUE(loaded.getPixel(511, 511, r, g, b));
    EXPECT_EQ(r, 0);
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 255);
}

TEST_F(BmpImageTest, SaveToInvalidPath_ReturnsFalse)
{
    const BmpImage img(10, 10);
    EXPECT_FALSE(img.save("Z:\\invalid_dir\\test.bmp"));
}

TEST_F(BmpImageTest, MultipleSavesToDifferentPaths)
{
    const auto dest1 = tmpPath_ + ".v1.bmp";
    const auto dest2 = tmpPath_ + ".v2.bmp";
    {
        BmpImage img(2, 2);
        img.setPixel(0, 0, 128, 64, 32);
        EXPECT_TRUE(img.save(dest1));
        EXPECT_TRUE(img.save(dest2));
    }
    {
        BmpImage loaded1(dest1);
        BmpImage loaded2(dest2);
        uint8_t r, g, b;
        EXPECT_TRUE(loaded1.getPixel(0, 0, r, g, b));
        EXPECT_EQ(r, 128);
        EXPECT_EQ(g, 64);
        EXPECT_EQ(b, 32);
        EXPECT_TRUE(loaded2.getPixel(0, 0, r, g, b));
        EXPECT_EQ(r, 128);
        EXPECT_EQ(g, 64);
        EXPECT_EQ(b, 32);
    }
    std::error_code ec;
    std::filesystem::remove(dest1, ec);
    std::filesystem::remove(dest2, ec);
}

TEST_F(BmpImageTest, LinearGradientRoundTrip)
{
    constexpr int size = 10;
    {
        BmpImage img(size, size);
        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                img.setPixel(x, y,
                             static_cast<uint8_t>(x * 25),
                             static_cast<uint8_t>(y * 25),
                             static_cast<uint8_t>((x + y) * 12));
            }
        }
        EXPECT_TRUE(img.save(tmpPath_));
    }
    const BmpImage loaded(tmpPath_);
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            uint8_t r, g, b;
            EXPECT_TRUE(loaded.getPixel(x, y, r, g, b));
            EXPECT_EQ(r, static_cast<uint8_t>(x * 25));
            EXPECT_EQ(g, static_cast<uint8_t>(y * 25));
        }
    }
}
