/**
 * @file IRunnableTest.cc
 * @brief Unit tests for IRunnable interface
 * @details Tests cover run with return values, void return, and arguments.
 */

#include <string>
#include <gtest/gtest.h>

#include <cppforge/interface/task/IRunnable.hpp>

using namespace cppforge::interface::task;

namespace
{
    /// @brief Mock runnable returning int
    class MockIntRunnable : public IRunnable<int, int, int>
    {
    public:
        int run(int a, int b) override
        {
            return a + b;
        }
    };

    /// @brief Mock runnable with void return
    class MockVoidRunnable : public IRunnable<void, std::string&>
    {
    public:
        void run(std::string& out) override
        {
            out = "executed";
        }
    };

    /// @brief Mock runnable returning string
    class MockStringRunnable : public IRunnable<std::string, std::string>
    {
    public:
        std::string run(std::string s) override
        {
            return "hello " + s;
        }
    };
}

/// @brief Test fixture for IRunnable tests.
class IRunnableTest : public testing::Test
{
protected:
};

/** @brief Tests int runnable returns sum of two arguments.
 *  @details Verifies MockIntRunnable.run(3,4) returns 7.
 */
TEST_F(IRunnableTest, IntRunnableReturnsCorrectValue)
{
    MockIntRunnable runnable;
    const auto result = runnable.run(3, 4);

    EXPECT_EQ(result, 7);
}

/** @brief Tests int runnable with various input combinations.
 *  @details Verifies correct results for zero, negative, and large values.
 */
TEST_F(IRunnableTest, IntRunnableWithDifferentValues)
{
    MockIntRunnable runnable;

    EXPECT_EQ(runnable.run(0, 0), 0);
    EXPECT_EQ(runnable.run(-1, 1), 0);
    EXPECT_EQ(runnable.run(100, 200), 300);
}

/** @brief Tests void runnable modifies output argument.
 *  @details Verifies MockVoidRunnable sets the string to "executed".
 */
TEST_F(IRunnableTest, VoidRunnableModifiesArgument)
{
    MockVoidRunnable runnable;
    std::string result;

    runnable.run(result);

    EXPECT_EQ(result, "executed");
}

/** @brief Tests string runnable formats output with prefix.
 *  @details Verifies MockStringRunnable prepends "hello " to input "world".
 */
TEST_F(IRunnableTest, StringRunnableReturnsFormattedString)
{
    MockStringRunnable runnable;

    const auto result = runnable.run("world");

    EXPECT_EQ(result, "hello world");
}

/** @brief Tests string runnable with empty input string.
 *  @details Verifies resulting string is "hello " (prefix only).
 */
TEST_F(IRunnableTest, StringRunnableWithEmptyString)
{
    MockStringRunnable runnable;

    const auto result = runnable.run("");

    EXPECT_EQ(result, "hello ");
}

/** @brief Tests runnable can be reused multiple times.
 *  @details Verifies repeated calls with different arguments produce correct results.
 */
TEST_F(IRunnableTest, ReusableRunnable)
{
    MockIntRunnable runnable;

    EXPECT_EQ(runnable.run(1, 2), 3);
    EXPECT_EQ(runnable.run(10, 20), 30);
    EXPECT_EQ(runnable.run(100, 200), 300);
}

/** @brief Tests int runnable return value is usable (nodiscard).
 *  @details Verifies the returned value is captured and equals expected sum.
 */
TEST_F(IRunnableTest, IntRunnableIsNoDiscard)
{
    MockIntRunnable runnable;
    [[maybe_unused]] const auto result = runnable.run(5, 5);
    EXPECT_EQ(result, 10);
}
