#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "filesystem/io/reader/InputStreamReader.hpp"
#include "filesystem/io/reader/StringReader.hpp"

using namespace common::filesystem;

class InputStreamReaderTest : public testing::Test
{
protected:
    std::shared_ptr<StringReader> inner_;
    std::unique_ptr<InputStreamReader> reader_;

    void SetUp() override
    {
        // "h€llo" — € (U+20AC) is 3 bytes in UTF-8: E2 82 AC
        inner_ = std::make_shared<StringReader>(std::string("h\xE2\x82\xAC"
                                                             "llo"));
        reader_ = std::make_unique<InputStreamReader>(inner_);
    }
};

TEST_F(InputStreamReaderTest, ReadAsciiChar)
{
    EXPECT_EQ(reader_->read(), 'h');
}

TEST_F(InputStreamReaderTest, ReadMultiByteChar)
{
    reader_->read();
    auto codepoint = reader_->read();
    EXPECT_EQ(codepoint, 0x20AC);
}

TEST_F(InputStreamReaderTest, ReadMixedContent)
{
    EXPECT_EQ(reader_->read(), 'h');
    EXPECT_EQ(reader_->read(), 0x20AC);
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'l');
    EXPECT_EQ(reader_->read(), 'o');
    EXPECT_EQ(reader_->read(), -1);
}

TEST_F(InputStreamReaderTest, Read4ByteUtf8)
{
    // U+1F600 (😀) is 4 bytes in UTF-8: F0 9F 98 80
    auto inner = std::make_shared<StringReader>(std::string("\xF0\x9F\x98\x80"));
    InputStreamReader isr(inner);
    EXPECT_EQ(isr.read(), 0x1F600);
}

TEST_F(InputStreamReaderTest, ReadIntoBuffer)
{
    std::vector<char> buf(3);
    auto n = reader_->read(buf, 0, 3);
    EXPECT_EQ(n, 3);
    EXPECT_EQ(buf[0], 'h');
    EXPECT_EQ(buf[1], static_cast<char>(0x20AC));
    EXPECT_EQ(buf[2], 'l');
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
    auto inner = std::make_shared<StringReader>("");
    InputStreamReader empty(inner);
    EXPECT_EQ(empty.read(), -1);
}
