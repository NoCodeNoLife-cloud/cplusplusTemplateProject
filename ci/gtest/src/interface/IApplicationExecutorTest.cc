/**
 * @file IApplicationExecutorTest.cc
 * @brief Unit tests for IApplicationExecutor interface
 * @details Tests cover execution with argc/argv and vector-of-strings arguments.
 */

#include <gtest/gtest.h>

#include "interface/task/IApplicationExecutor.hpp"

using namespace common::interface::task;

namespace
{
    /// @brief Mock executor that records invocations
    class MockExecutor : public IApplicationExecutor
    {
    public:
        int argcArg_ = 0;
        std::vector<std::string> lastArgs_;
        bool returnValue_ = true;

        bool execute(int argc, char* const argv[]) override
        {
            argcArg_ = argc;
            lastArgs_.clear();
            if (argv != nullptr)
                for (int i = 0; i < argc; i++)
                    if (argv[i] != nullptr)
                        lastArgs_.emplace_back(argv[i]);
            return returnValue_;
        }

        bool execute(const std::vector<std::string>& args) override
        {
            lastArgs_ = args;
            argcArg_ = static_cast<int>(args.size());
            return returnValue_;
        }
    };
}

/// @brief Test fixture for IApplicationExecutor tests.
class IApplicationExecutorTest : public testing::Test
{
protected:
    void SetUp() override
    {
        executor_ = std::make_unique<MockExecutor>();
    }

    void TearDown() override
    {
        executor_.reset();
    }

    std::unique_ptr<MockExecutor> executor_;
};

/** @brief Verifies execute(argc, argv) forwards arguments correctly. */
TEST_F(IApplicationExecutorTest, ExecuteWithArgcArgv)
{
    char arg1[] = "program";
    char arg2[] = "--help";
    char* argv[] = {arg1, arg2};

    const bool result = executor_->execute(2, argv);

    EXPECT_TRUE(result);
    EXPECT_EQ(executor_->argcArg_, 2);
    ASSERT_EQ(executor_->lastArgs_.size(), 2);
    EXPECT_EQ(executor_->lastArgs_[0], "program");
    EXPECT_EQ(executor_->lastArgs_[1], "--help");
}

/** @brief Verifies execute(vector<string>) forwards arguments correctly. */
TEST_F(IApplicationExecutorTest, ExecuteWithVectorArgs)
{
    const std::vector<std::string> args = {"app", "-v", "config.yaml"};

    const bool result = executor_->execute(args);

    EXPECT_TRUE(result);
    EXPECT_EQ(executor_->argcArg_, 3);
    ASSERT_EQ(executor_->lastArgs_.size(), 3);
    EXPECT_EQ(executor_->lastArgs_[0], "app");
    EXPECT_EQ(executor_->lastArgs_[1], "-v");
    EXPECT_EQ(executor_->lastArgs_[2], "config.yaml");
}

/** @brief Verifies execute returns false when the mock reports failure. */
TEST_F(IApplicationExecutorTest, ExecuteReturnsFalseOnFailure)
{
    executor_->returnValue_ = false;
    const std::vector<std::string> args = {"fail"};

    const bool result = executor_->execute(args);

    EXPECT_FALSE(result);
}

/** @brief Verifies execute with an empty argument vector returns true. */
TEST_F(IApplicationExecutorTest, ExecuteWithEmptyArgs)
{
    const std::vector<std::string> args;

    const bool result = executor_->execute(args);

    EXPECT_TRUE(result);
    EXPECT_EQ(executor_->argcArg_, 0);
    EXPECT_TRUE(executor_->lastArgs_.empty());
}

/** @brief Verifies execute with argc=0 and nullptr argv is safe. */
TEST_F(IApplicationExecutorTest, ExecuteWithArgcZero)
{
    const auto result = executor_->execute(0, nullptr);

    EXPECT_TRUE(result);
    EXPECT_EQ(executor_->argcArg_, 0);
    EXPECT_TRUE(executor_->lastArgs_.empty());
}

/** @brief Verifies the order of arguments is preserved. */
TEST_F(IApplicationExecutorTest, ExecuteWithMultipleArgsPreservesOrder)
{
    const std::vector<std::string> args = {"first", "second", "third", "fourth"};

    executor_->execute(args);

    ASSERT_EQ(executor_->lastArgs_.size(), 4);
    EXPECT_EQ(executor_->lastArgs_[0], "first");
    EXPECT_EQ(executor_->lastArgs_[1], "second");
    EXPECT_EQ(executor_->lastArgs_[2], "third");
    EXPECT_EQ(executor_->lastArgs_[3], "fourth");
}

/** @brief Verifies execute does not throw under normal conditions. */
TEST_F(IApplicationExecutorTest, ExecuteDoesNotThrow)
{
    const std::vector<std::string> args = {"test"};
    EXPECT_NO_THROW(executor_->execute(args));
}

/** @brief Verifies the executor can be called multiple times sequentially. */
TEST_F(IApplicationExecutorTest, ExecuteReusesExecutor)
{
    executor_->execute({"first"});
    executor_->execute({"second"});

    ASSERT_EQ(executor_->lastArgs_.size(), 1);
    EXPECT_EQ(executor_->lastArgs_[0], "second");
}
