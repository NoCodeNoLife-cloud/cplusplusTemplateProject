#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

#include "io/reader/Scanner.hpp"

using namespace common::io::reader;

class ScannerTest : public testing::Test
{
protected:
    std::stringstream ss_;
    std::unique_ptr<Scanner> scanner_;

    void SetUp() override
    {
        ss_.str("42 3.14 hello\nworld");
        ss_.clear();
        scanner_ = std::make_unique<Scanner>(ss_);
    }
};

TEST_F(ScannerTest, NextInt)
{
    EXPECT_EQ(scanner_->nextInt(), 42);
}

TEST_F(ScannerTest, NextDouble)
{
    (void)scanner_->nextInt();
    EXPECT_DOUBLE_EQ(scanner_->nextDouble(), 3.14);
}

TEST_F(ScannerTest, NextLine)
{
    (void)scanner_->nextInt();
    (void)scanner_->nextDouble();
    EXPECT_EQ(scanner_->nextLine(), " hello");
}

TEST_F(ScannerTest, GetNextToken)
{
    std::string token;
    EXPECT_TRUE(scanner_->getNextToken(token));
    EXPECT_EQ(token, "42");
    EXPECT_TRUE(scanner_->getNextToken(token));
    EXPECT_EQ(token, "3.14");
    EXPECT_TRUE(scanner_->getNextToken(token));
    EXPECT_EQ(token, "hello");
    EXPECT_TRUE(scanner_->getNextToken(token));
    EXPECT_EQ(token, "world");
    EXPECT_FALSE(scanner_->getNextToken(token));
}

TEST_F(ScannerTest, NextTokens)
{
    const auto tokens = scanner_->nextTokens(' ');
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0], "42");
    EXPECT_EQ(tokens[1], "3.14");
    EXPECT_EQ(tokens[2], "hello");
}

TEST_F(ScannerTest, NextIntMinMax)
{
    std::stringstream ss("2147483647 -2147483648");
    Scanner s(ss);
    EXPECT_EQ(s.nextInt(), 2147483647);
    EXPECT_EQ(s.nextInt(), -2147483648);
}

TEST_F(ScannerTest, NextIntOverflow)
{
    std::stringstream ss("999999999999");
    Scanner s(ss);
    EXPECT_THROW((void)s.nextInt(), std::out_of_range);
}

TEST_F(ScannerTest, NextIntInvalidInput)
{
    std::stringstream ss("abc");
    Scanner s(ss);
    EXPECT_THROW((void)s.nextInt(), std::invalid_argument);
}

TEST_F(ScannerTest, NextIntAtEnd)
{
    std::stringstream ss("");
    Scanner s(ss);
    EXPECT_THROW((void)s.nextInt(), std::runtime_error);
}

TEST_F(ScannerTest, NextDoubleAtEnd)
{
    std::stringstream ss("");
    Scanner s(ss);
    EXPECT_THROW((void)s.nextDouble(), std::runtime_error);
}

TEST_F(ScannerTest, NextLineEmpty)
{
    std::stringstream ss("");
    Scanner s(ss);
    EXPECT_TRUE(s.nextLine().empty());
}
