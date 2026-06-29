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

using namespace cppforge::filesystem::type;

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

/**
 * @brief Test creating a BmpImage with valid dimensions
 * @details Verifies that a 10x10 BmpImage object stores the correct width and height values
 */
TEST_F(BmpImageTest, CreateValidImage)
{
    const BmpImage img(10, 10);
    EXPECT_EQ(img.getWidth(), 10);
    EXPECT_EQ(img.getHeight(), 10);
}

/**
 * @brief Test creating BmpImage with invalid width throws
 * @details Verifies that passing zero or negative width to the constructor throws std::invalid_argument
 */
TEST_F(BmpImageTest, CreateInvalidWidth)
{
    EXPECT_THROW(BmpImage(0, 10), std::invalid_argument);
    EXPECT_THROW(BmpImage(-1, 10), std::invalid_argument);
}

/**
 * @brief Test creating BmpImage with invalid height throws
 * @details Verifies that passing zero or negative height to the constructor throws std::invalid_argument
 */
TEST_F(BmpImageTest, CreateInvalidHeight)
{
    EXPECT_THROW(BmpImage(10, 0), std::invalid_argument);
    EXPECT_THROW(BmpImage(10, -1), std::invalid_argument);
}

/**
 * @brief Test setting and getting pixel RGB values
 * @details Verifies that setPixel() stores RGB values and getPixel() retrieves them correctly for multiple pixels
 */
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

/**
 * @brief Test getPixel returns false for out-of-bounds coordinates
 * @details Verifies that getPixel() returns false when requesting a pixel outside the image dimensions
 */
TEST_F(BmpImageTest, GetPixelOutOfBounds)
{
    const BmpImage img(5, 5);
    uint8_t r, g, b;
    EXPECT_FALSE(img.getPixel(-1, 0, r, g, b));
    EXPECT_FALSE(img.getPixel(0, -1, r, g, b));
    EXPECT_FALSE(img.getPixel(5, 0, r, g, b));
    EXPECT_FALSE(img.getPixel(0, 5, r, g, b));
}

/**
 * @brief Test setPixel handles out-of-bounds coordinates gracefully
 * @details Verifies that setPixel() does not crash when coordinates are outside the image boundaries
 */
TEST_F(BmpImageTest, SetPixelOutOfBoundsDoesNotCrash)
{
    BmpImage img(5, 5);
    EXPECT_NO_THROW(img.setPixel(-1, 0, 255, 0, 0));
    EXPECT_NO_THROW(img.setPixel(0, -1, 255, 0, 0));
    EXPECT_NO_THROW(img.setPixel(10, 0, 255, 0, 0));
    EXPECT_NO_THROW(img.setPixel(0, 10, 255, 0, 0));
}

/**
 * @brief Test saving and loading an image preserves pixel data
 * @details Verifies that a 2x2 image with four different pixel colors is faithfully restored after a save/load cycle
 */
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

/**
 * @brief Test loading a non-existent BMP file throws
 * @details Verifies that constructing BmpImage from a non-existent path throws std::runtime_error
 */
TEST_F(BmpImageTest, LoadNonExistentFile)
{
    EXPECT_THROW(BmpImage("nonexistent_file.bmp"), std::runtime_error);
}

/**
 * @brief Test loading an invalid BMP file throws
 * @details Verifies that constructing BmpImage from a file with non-BMP content throws std::runtime_error
 */
TEST_F(BmpImageTest, LoadInvalidFile)
{
    {
        std::ofstream f(tmpPath_);
        f << "not a bmp file" << std::endl;
    }
    EXPECT_THROW(BmpImage{tmpPath_}, std::runtime_error);
}

/**
 * @brief Test default pixel values are black (zero RGB)
 * @details Verifies that all pixels in a newly created BmpImage default to RGB(0, 0, 0)
 */
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

/**
 * @brief Test creating and manipulating a single-pixel image
 * @details Verifies that a 1x1 BMP can store and retrieve pixel data correctly
 */
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

/**
 * @brief Test save/load round-trip for a single-pixel image
 * @details Verifies that a 1x1 BMP with a specific color survives a full save and reload cycle
 */
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

/**
 * @brief Test save/load round-trip for a large 100x100 image
 * @details Verifies that a 100x100 BMP with specific pixel colors survives a full save and reload cycle
 */
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

/**
 * @brief Test creating a fully white image and verifying round-trip
 * @details Verifies that a 5x5 white image can be saved and reloaded with all pixels at RGB(255, 255, 255)
 */
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

/**
 * @brief Test overwriting an existing BMP file with a new image
 * @details Verifies that saving a different image to the same file path replaces the content correctly
 */
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

/**
 * @brief Test loading a BMP with a corrupted header throws
 * @details Verifies that a file with 'BM' signature but corrupted header data throws std::runtime_error
 */
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

/**
 * @brief Test loading a truncated BMP file throws
 * @details Verifies that a BMP file with a valid header but insufficient pixel data throws std::runtime_error
 */
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

/**
 * @brief Test save/load round-trip for a default all-black image
 * @details Verifies that a 3x3 default (all-black) BMP survives a full save and reload cycle
 */
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

/**
 * @brief Test creating a rectangular (non-square) BMP image
 * @details Verifies that an 8x4 image with a single red pixel survives a save/load round-trip
 */
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

/**
 * @brief Test save/load round-trip for a 512x512 image
 * @details Verifies that a large BMP with corner pixel colors survives a full save and reload cycle
 */
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

/**
 * @brief Test save to an invalid path returns false
 * @details Verifies that save() returns false when the target path is not writable
 */
TEST_F(BmpImageTest, SaveToInvalidPath_ReturnsFalse)
{
    const BmpImage img(10, 10);
    EXPECT_FALSE(img.save("Z:\\invalid_dir\\test.bmp"));
}

/**
 * @brief Test saving the same image to multiple file paths
 * @details Verifies that save() works correctly when called on different destination paths with the same image
 */
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

/**
 * @brief Test save/load round-trip for a linear gradient image
 * @details Verifies that a 10x10 image with a gradient pattern survives a full save and reload cycle
 */
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
