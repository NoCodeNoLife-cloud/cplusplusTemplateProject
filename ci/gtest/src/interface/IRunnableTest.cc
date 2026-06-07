/**
 * @file IRunnableTest.cc
 * @brief Unit tests for IRunnable interface
 * @details Tests cover run with return values, void return, and arguments.
 */

#include <string>
#include <gtest/gtest.h>

#include "interface/IRunnable.hpp"

using namespace common::interfaces;

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

class IRunnableTest : public testing::Test
{
protected:
};

TEST_F(IRunnableTest, IntRunnableReturnsCorrectValue)
{
    MockIntRunnable runnable;
    auto result = runnable.run(3, 4);

    EXPECT_EQ(result, 7);
}

TEST_F(IRunnableTest, IntRunnableWithDifferentValues)
{
    MockIntRunnable runnable;

    EXPECT_EQ(runnable.run(0, 0), 0);
    EXPECT_EQ(runnable.run(-1, 1), 0);
    EXPECT_EQ(runnable.run(100, 200), 300);
}

TEST_F(IRunnableTest, VoidRunnableModifiesArgument)
{
    MockVoidRunnable runnable;
    std::string result;

    runnable.run(result);

    EXPECT_EQ(result, "executed");
}

TEST_F(IRunnableTest, StringRunnableReturnsFormattedString)
{
    MockStringRunnable runnable;

    auto result = runnable.run("world");

    EXPECT_EQ(result, "hello world");
}

TEST_F(IRunnableTest, StringRunnableWithEmptyString)
{
    MockStringRunnable runnable;

    auto result = runnable.run("");

    EXPECT_EQ(result, "hello ");
}

TEST_F(IRunnableTest, ReusableRunnable)
{
    MockIntRunnable runnable;

    EXPECT_EQ(runnable.run(1, 2), 3);
    EXPECT_EQ(runnable.run(10, 20), 30);
    EXPECT_EQ(runnable.run(100, 200), 300);
}

TEST_F(IRunnableTest, IntRunnableIsNoDiscard)
{
    MockIntRunnable runnable;
    [[maybe_unused]] auto result = runnable.run(5, 5);
    EXPECT_EQ(result, 10);
}
