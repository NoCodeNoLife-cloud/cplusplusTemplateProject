/**
 * @file ConsoleTest.cc
 * @brief Unit tests for the Console class
 * @details Tests cover formatted output, input reading, stream access, and IFlushable interface compliance.
 */

#include <sstream>
#include <gtest/gtest.h>

#include "filesystem/io/Console.hpp"

using namespace common::filesystem;

/**
 * @brief Test fixture for Console tests
 */
class ConsoleTest : public testing::Test
{
protected:
    void SetUp() override
    {
        original_cout_ = std::cout.rdbuf();
        std::cout.rdbuf(cout_buffer_.rdbuf());

        original_cin_ = std::cin.rdbuf();
        std::cin.rdbuf(cin_buffer_.rdbuf());
    }

    void TearDown() override
    {
        std::cout.rdbuf(original_cout_);
        std::cin.rdbuf(original_cin_);
    }

    std::string coutOutput() const { return cout_buffer_.str(); }
    void setCinInput(const std::string& input) { cin_buffer_.str(input); }

private:
    std::stringstream cout_buffer_;
    std::stringstream cin_buffer_;
    std::streambuf* original_cout_ = nullptr;
    std::streambuf* original_cin_ = nullptr;
};

TEST_F(ConsoleTest, Format_Output)
{
    Console::format("Hello, {}!", "World");
    EXPECT_EQ(coutOutput(), "Hello, World!");
}

TEST_F(ConsoleTest, Format_MultipleArgs)
{
    Console::format("{} + {} = {}", 1, 2, 3);
    EXPECT_EQ(coutOutput(), "1 + 2 = 3");
}

TEST_F(ConsoleTest, Format_EmptyString)
{
    Console::format("");
    EXPECT_EQ(coutOutput(), "");
}

TEST_F(ConsoleTest, ReadLine_Input)
{
    setCinInput("test line\n");
    EXPECT_EQ(Console::readLine(), "test line");
}

TEST_F(ConsoleTest, ReadLine_EmptyInput)
{
    setCinInput("\n");
    EXPECT_EQ(Console::readLine(), "");
}

TEST_F(ConsoleTest, ReadLine_PromptOutput)
{
    setCinInput("hello\n");
    auto result = Console::readLine("Enter: ");
    EXPECT_EQ(coutOutput(), "Enter: ");
    EXPECT_EQ(result, "hello");
}

TEST_F(ConsoleTest, ReadLine_FormattedPrompt)
{
    setCinInput("ok\n");
    auto result = Console::readLine("{} > ", "test");
    EXPECT_EQ(coutOutput(), "test > ");
    EXPECT_EQ(result, "ok");
}

TEST_F(ConsoleTest, Flush_DoesNotCrash)
{
    Console console;
    EXPECT_NO_THROW(console.flush());
}

TEST_F(ConsoleTest, Writer_ReturnsCout)
{
    EXPECT_EQ(&Console::writer(), &std::cout);
}

TEST_F(ConsoleTest, Reader_ReturnsCin)
{
    EXPECT_EQ(&Console::reader(), &std::cin);
}

TEST_F(ConsoleTest, ImplementsIFlushable)
{
    Console console;
    EXPECT_TRUE(dynamic_cast<common::interfaces::io::IFlushable*>(&console) != nullptr);
}

TEST_F(ConsoleTest, FlushSafe_Success)
{
    Console console;
    EXPECT_TRUE(console.flushSafe());
}

TEST_F(ConsoleTest, IsFlushNeeded_ReturnsTrue)
{
    Console console;
    EXPECT_TRUE(console.isFlushNeeded());
}
