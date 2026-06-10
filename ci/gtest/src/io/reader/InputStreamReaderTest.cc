#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "io/reader/InputStreamReader.hpp"
#include "io/reader/StringReader.hpp"

using namespace common::io::reader;

class InputStreamReaderTest : public testing::Test
{
protected:
    std::shared_ptr<StringReader> inner_;
    std::unique_ptr<InputStreamReader> reader_;

    void SetUp() override
    {
        // "h€llo" �?�?(U+20AC) is 3 bytes in UTF-8: E2 82 AC
        inner_ = std::make_shared<StringReader>(std::string("h\xE2\x82\xAC"
                                                             "llo"));
        reader_ = std::make_unique<InputStreamReader>(inner_);
    }
};

TEST_F(InputStreamReaderTest, ReadByteByByte)
{
    EXPECT_EQ(reader_->read(), 'h');
    EXPECT_EQ(reader_->read(), static_cast<char>(0xE2));
    EXPECT_EQ(reader_->read(), static_cast<char>(0x82));
    EXPECT_EQ(reader_->read(), static_cast<char>(0xAC));
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'o');
    EXPECT_FALSE(reader_->read().has_value());
}

TEST_F(InputStreamReaderTest, ReadIntoBuffer)
{
    std::vector<char> buf(3);
    const auto n = reader_->read(buf, 0, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[0], 'h');
    EXPECT_EQ(buf[1], static_cast<char>(0xE2));
    EXPECT_EQ(buf[2], static_cast<char>(0x82));
}

TEST_F(InputStreamReaderTest, CharsetValidation)
{
    EXPECT_THROW(InputStreamReader(inner_, "ISO-8859-1"), std::invalid_argument);
}

TEST_F(InputStreamReaderTest, MarkNotSupported)
{
    EXPECT_FALSE(reader_->markSupported());
}

TEST_F(InputStreamReaderTest, Close)
{
    reader_->close();
    EXPECT_TRUE(reader_->isClosed());
    EXPECT_THROW(reader_->read(), std::runtime_error);
}

TEST_F(InputStreamReaderTest, ReadEmptyStream)
{
    const auto inner = std::make_shared<StringReader>("");
    InputStreamReader empty(inner);
    EXPECT_FALSE(empty.read().has_value());
}

TEST_F(InputStreamReaderTest, Read4ByteSequence)
{
    // U+1F600 (😀) is 4 bytes in UTF-8: F0 9F 98 80
    const auto inner = std::make_shared<StringReader>(std::string("\xF0\x9F\x98\x80"));
    InputStreamReader isr(inner);
    EXPECT_EQ(isr.read(), static_cast<char>(0xF0));
    EXPECT_EQ(isr.read(), static_cast<char>(0x9F));
    EXPECT_EQ(isr.read(), static_cast<char>(0x98));
    EXPECT_EQ(isr.read(), static_cast<char>(0x80));
    EXPECT_FALSE(isr.read().has_value());
}

// ============================================================================
// Additional Boundary Condition Tests
// ============================================================================

/**
 * @brief Test reading into a full buffer
 * @details Read multiple bytes at once
 */
TEST_F(InputStreamReaderTest, ReadIntoFullBuffer)
{
    std::vector<char> buf(7);
    const auto n = reader_->read(buf, 0, 7);
    EXPECT_EQ(n, 7);
    EXPECT_EQ(buf[0], 'h');
    EXPECT_EQ(buf[1], static_cast<char>(0xE2));
    EXPECT_EQ(buf[2], static_cast<char>(0x82));
    EXPECT_EQ(buf[3], static_cast<char>(0xAC));
    EXPECT_EQ(buf[4], 'l');
    EXPECT_EQ(buf[5], 'l');
    EXPECT_EQ(buf[6], 'o');
}

/**
 * @brief Test read with zero length
 * @edge Reading zero bytes returns immediately
 */
TEST_F(InputStreamReaderTest, ReadZeroLength)
{
    std::vector<char> buf(3);
    const auto n = reader_->read(buf, 0, 0);
    EXPECT_EQ(n, 0);
}

/**
 * @brief Test skip operation
 * @details Verify skip delegates correctly
 */
TEST_F(InputStreamReaderTest, Skip)
{
    const auto skipped = reader_->skip(2);
    EXPECT_EQ(skipped, 2);
    EXPECT_EQ(reader_->read(), static_cast<char>(0x82));
}

/**
 * @brief Test skip beyond end of stream
 * @edge Skipping beyond available data
 */
TEST_F(InputStreamReaderTest, SkipBeyondEnd)
{
    const auto skipped = reader_->skip(100);
    EXPECT_EQ(skipped, 7);
    EXPECT_FALSE(reader_->read().has_value());
}

/**
 * @brief Test read buffer with offset
 * @details Reading into buffer at specific offset
 */
TEST_F(InputStreamReaderTest, ReadIntoBufferWithOffset)
{
    std::vector<char> buf(10);
    const auto n = reader_->read(buf, 2, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[2], 'h');
    EXPECT_EQ(buf[3], static_cast<char>(0xE2));
    EXPECT_EQ(buf[4], static_cast<char>(0x82));
}

/**
 * @brief Test 2-byte UTF-8 sequence
 * @details Verify 2-byte character handling
 */
TEST_F(InputStreamReaderTest, Read2ByteSequence)
{
    // U+00E9 (é) is 2 bytes in UTF-8: C3 A9
    const auto inner = std::make_shared<StringReader>(std::string("\xC3\xA9"));
    InputStreamReader isr(inner);
    EXPECT_EQ(isr.read(), static_cast<char>(0xC3));
    EXPECT_EQ(isr.read(), static_cast<char>(0xA9));
    EXPECT_FALSE(isr.read().has_value());
}

/**
 * @brief Test mixed 1/2/3/4-byte UTF-8 sequences
 * @details Mixed character widths
 */
TEST_F(InputStreamReaderTest, MixedCharacterWidths)
{
    // 'a' (1) + é(2) + €(3) + 😀(4) = 10 bytes
    const auto inner = std::make_shared<StringReader>(
        std::string("a\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80"));
    InputStreamReader isr(inner);
    EXPECT_EQ(isr.read(), 'a');
    EXPECT_EQ(isr.read(), static_cast<char>(0xC3));
    EXPECT_EQ(isr.read(), static_cast<char>(0xA9));
    EXPECT_EQ(isr.read(), static_cast<char>(0xE2));
    EXPECT_EQ(isr.read(), static_cast<char>(0x82));
    EXPECT_EQ(isr.read(), static_cast<char>(0xAC));
    EXPECT_EQ(isr.read(), static_cast<char>(0xF0));
    EXPECT_EQ(isr.read(), static_cast<char>(0x9F));
    EXPECT_EQ(isr.read(), static_cast<char>(0x98));
    EXPECT_EQ(isr.read(), static_cast<char>(0x80));
    EXPECT_FALSE(isr.read().has_value());
}

/**
 * @brief Test read buffer beyond stream size
 * @edge Reading more bytes than available
 */
TEST_F(InputStreamReaderTest, ReadBeyondEnd)
{
    std::vector<char> buf(100);
    const auto n = reader_->read(buf, 0, 100);
    EXPECT_EQ(n, 7);
}

/**
 * @brief Test large stream
 * @edge Large data read
 */
TEST_F(InputStreamReaderTest, LargeStream)
{
    const std::string large(10000, 'x');
    const auto inner = std::make_shared<StringReader>(large);
    InputStreamReader isr(inner);
    std::vector<char> buf(10000);
    const auto n = isr.read(buf, 0, 10000);
    EXPECT_EQ(n, 10000);
    EXPECT_EQ(buf[0], 'x');
    EXPECT_EQ(buf[9999], 'x');
}
