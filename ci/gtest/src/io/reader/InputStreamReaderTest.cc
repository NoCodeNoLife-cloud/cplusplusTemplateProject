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
        // "hâ‚¬llo" â€?â‚?(U+20AC) is 3 bytes in UTF-8: E2 82 AC
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
    // U+1F600 (ðŸ˜€) is 4 bytes in UTF-8: F0 9F 98 80
    const auto inner = std::make_shared<StringReader>(std::string("\xF0\x9F\x98\x80"));
    InputStreamReader isr(inner);
    EXPECT_EQ(isr.read(), static_cast<char>(0xF0));
    EXPECT_EQ(isr.read(), static_cast<char>(0x9F));
    EXPECT_EQ(isr.read(), static_cast<char>(0x98));
    EXPECT_EQ(isr.read(), static_cast<char>(0x80));
    EXPECT_FALSE(isr.read().has_value());
}
