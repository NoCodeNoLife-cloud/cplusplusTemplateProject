/**
 * @file ConsoleTest.cc
 * @brief Unit tests for the Console class
 * @details Tests cover formatted output, input reading, stream access, and IFlushable interface compliance.
 */

#include <sstream>
#include <gtest/gtest.h>

#include "io/Console.hpp"

using namespace cppforge::io;

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

    [[nodiscard]] std::string coutOutput() const { return cout_buffer_.str(); }
    void setCinInput(const std::string& input) { cin_buffer_.str(input); }

private:
    std::stringstream cout_buffer_;
    std::stringstream cin_buffer_;
    std::streambuf* original_cout_ = nullptr;
    std::streambuf* original_cin_ = nullptr;
};

/** @brief Test formatted output with a single argument. @details Verifies that format() replaces the {} placeholder and writes the result to stdout. */
TEST_F(ConsoleTest, Format_Output)
{
    Console::format("Hello, {}!", "World");
    EXPECT_EQ(coutOutput(), "Hello, World!");
}

/** @brief Test formatted output with multiple arguments. @details Verifies that format() replaces multiple {} placeholders in positional order. */
TEST_F(ConsoleTest, Format_MultipleArgs)
{
    Console::format("{} + {} = {}", 1, 2, 3);
    EXPECT_EQ(coutOutput(), "1 + 2 = 3");
}

/** @brief Test formatted output with an empty format string. @details Verifies that format("") produces no output and does not throw. */
TEST_F(ConsoleTest, Format_EmptyString)
{
    Console::format("");
    EXPECT_EQ(coutOutput(), "");
}

/** @brief Test reading a line from stdin. @details Verifies that readLine() returns the input string stripped of the trailing newline character. */
TEST_F(ConsoleTest, ReadLine_Input)
{
    setCinInput("test line\n");
    EXPECT_EQ(Console::readLine(), "test line");
}

/** @brief Test readLine with empty input. @details Verifies that readLine() returns an empty string when only a newline is provided. */
TEST_F(ConsoleTest, ReadLine_EmptyInput)
{
    setCinInput("\n");
    EXPECT_EQ(Console::readLine(), "");
}

/** @brief Test readLine with a prompt string. @details Verifies that the prompt text is written to stdout before reading user input from stdin. */
TEST_F(ConsoleTest, ReadLine_PromptOutput)
{
    setCinInput("hello\n");
    auto result = Console::readLine("Enter: ");
    EXPECT_EQ(coutOutput(), "Enter: ");
    EXPECT_EQ(result, "hello");
}

/** @brief Test readLine with a formatted prompt. @details Verifies that the prompt supports {} placeholder substitution for dynamic text. */
TEST_F(ConsoleTest, ReadLine_FormattedPrompt)
{
    setCinInput("ok\n");
    auto result = Console::readLine("{} > ", "test");
    EXPECT_EQ(coutOutput(), "test > ");
    EXPECT_EQ(result, "ok");
}

/** @brief Test that flush() does not throw. @details Verifies that calling flush() on a Console instance completes without any exception. */
TEST_F(ConsoleTest, Flush_DoesNotCrash)
{
    Console console;
    EXPECT_NO_THROW(console.flush());
}

/** @brief Test that writer() returns a reference to std::cout. @details Verifies Console::writer() returns the standard output stream reference. */
TEST_F(ConsoleTest, Writer_ReturnsCout)
{
    EXPECT_EQ(&Console::writer(), &std::cout);
}

/** @brief Test that reader() returns a reference to std::cin. @details Verifies Console::reader() returns the standard input stream reference. */
TEST_F(ConsoleTest, Reader_ReturnsCin)
{
    EXPECT_EQ(&Console::reader(), &std::cin);
}

/** @brief Test that Console implements the IFlushable interface. @details Verifies that a Console instance can be dynamically cast to cppforge::interface::io::IFlushable. */
TEST_F(ConsoleTest, ImplementsIFlushable)
{
    Console console;
    EXPECT_TRUE(dynamic_cast<cppforge::interface::io::IFlushable*>(&console) != nullptr);
}

/** @brief Test that flushSafe() returns true on a healthy Console. @details Verifies that flushSafe() succeeds and returns true under normal conditions. */
TEST_F(ConsoleTest, FlushSafe_Success)
{
    Console console;
    EXPECT_TRUE(console.flushSafe());
}

/** @brief Test that isFlushNeeded() returns true. @details Verifies that isFlushNeeded() correctly indicates that flushing is required after construction. */
TEST_F(ConsoleTest, IsFlushNeeded_ReturnsTrue)
{
    Console console;
    EXPECT_TRUE(console.isFlushNeeded());
}
