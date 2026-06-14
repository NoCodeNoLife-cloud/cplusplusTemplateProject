#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

#include "io/reader/Scanner.hpp"

using namespace common::io::reader;

/// @brief Test fixture for Scanner tests.
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

/** @brief Test scanning the next integer token. @details Verifies that nextInt() reads and parses a decimal integer from the input. */
TEST_F(ScannerTest, NextInt)
{
    EXPECT_EQ(scanner_->nextInt(), 42);
}

/** @brief Test scanning the next double token. @details Verifies that nextDouble() reads and parses a floating-point value after an integer. */
TEST_F(ScannerTest, NextDouble)
{
    (void)scanner_->nextInt();
    EXPECT_DOUBLE_EQ(scanner_->nextDouble(), 3.14);
}

/** @brief Test scanning the next line. @details Verifies that nextLine() returns the remaining tokens on the current line as a single string. */
TEST_F(ScannerTest, NextLine)
{
    (void)scanner_->nextInt();
    (void)scanner_->nextDouble();
    EXPECT_EQ(scanner_->nextLine(), " hello");
}

/** @brief Test token extraction via getNextToken. @details Verifies that getNextToken() extracts whitespace-delimited tokens one at a time and returns false when exhausted. */
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

/** @brief Test splitting tokens by a delimiter. @details Verifies that nextTokens() splits the input on the specified delimiter and returns all tokens. */
TEST_F(ScannerTest, NextTokens)
{
    const auto tokens = scanner_->nextTokens(' ');
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0], "42");
    EXPECT_EQ(tokens[1], "3.14");
    EXPECT_EQ(tokens[2], "hello");
}

/** @brief Test nextInt with boundary integer values. @details Verifies that nextInt() correctly parses INT_MAX and INT_MIN values. */
TEST_F(ScannerTest, NextIntMinMax)
{
    std::stringstream ss("2147483647 -2147483648");
    Scanner s(ss);
    EXPECT_EQ(s.nextInt(), 2147483647);
    EXPECT_EQ(s.nextInt(), -2147483648);
}

/** @brief Test nextInt with an overflow value. @details Verifies that nextInt() throws std::out_of_range when the integer exceeds representable range. */
TEST_F(ScannerTest, NextIntOverflow)
{
    std::stringstream ss("999999999999");
    Scanner s(ss);
    EXPECT_THROW((void)s.nextInt(), std::out_of_range);
}

/** @brief Test nextInt with invalid input. @details Verifies that nextInt() throws std::invalid_argument when the token is not a valid integer. */
TEST_F(ScannerTest, NextIntInvalidInput)
{
    std::stringstream ss("abc");
    Scanner s(ss);
    EXPECT_THROW((void)s.nextInt(), std::invalid_argument);
}

/** @brief Test nextInt at end of input. @details Verifies that nextInt() throws std::runtime_error when no tokens remain. */
TEST_F(ScannerTest, NextIntAtEnd)
{
    std::stringstream ss("");
    Scanner s(ss);
    EXPECT_THROW((void)s.nextInt(), std::runtime_error);
}

/** @brief Test nextDouble at end of input. @details Verifies that nextDouble() throws std::runtime_error when no tokens remain. */
TEST_F(ScannerTest, NextDoubleAtEnd)
{
    std::stringstream ss("");
    Scanner s(ss);
    EXPECT_THROW((void)s.nextDouble(), std::runtime_error);
}

/** @brief Test nextLine on empty input. @details Verifies that nextLine() returns an empty string when the input stream is empty. */
TEST_F(ScannerTest, NextLineEmpty)
{
    std::stringstream ss("");
    Scanner s(ss);
    EXPECT_TRUE(s.nextLine().empty());
}
