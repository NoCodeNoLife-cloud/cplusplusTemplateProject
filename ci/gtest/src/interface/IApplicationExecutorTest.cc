/**
 * @file IApplicationExecutorTest.cc
 * @brief Unit tests for IApplicationExecutor interface
 * @details Tests cover execution with argc/argv and vector-of-strings arguments.
 */

#include <gtest/gtest.h>

#include "interface/task/IApplicationExecutor.hpp"

using namespace common::interfaces::task;

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
            for (int i = 0; i < argc; i++)
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

class IApplicationExecutorTest : public testing::Test
{
protected:
    void SetUp() override
    {
        executor_.reset(new MockExecutor());
    }

    void TearDown() override
    {
        executor_.reset();
    }

    std::unique_ptr<MockExecutor> executor_;
};

TEST_F(IApplicationExecutorTest, ExecuteWithArgcArgv)
{
    char arg1[] = "program";
    char arg2[] = "--help";
    char* argv[] = {arg1, arg2};

    bool result = executor_->execute(2, argv);

    EXPECT_TRUE(result);
    EXPECT_EQ(executor_->argcArg_, 2);
    ASSERT_EQ(executor_->lastArgs_.size(), 2);
    EXPECT_EQ(executor_->lastArgs_[0], "program");
    EXPECT_EQ(executor_->lastArgs_[1], "--help");
}

TEST_F(IApplicationExecutorTest, ExecuteWithVectorArgs)
{
    std::vector<std::string> args = {"app", "-v", "config.yaml"};

    bool result = executor_->execute(args);

    EXPECT_TRUE(result);
    EXPECT_EQ(executor_->argcArg_, 3);
    ASSERT_EQ(executor_->lastArgs_.size(), 3);
    EXPECT_EQ(executor_->lastArgs_[0], "app");
    EXPECT_EQ(executor_->lastArgs_[1], "-v");
    EXPECT_EQ(executor_->lastArgs_[2], "config.yaml");
}

TEST_F(IApplicationExecutorTest, ExecuteReturnsFalseOnFailure)
{
    executor_->returnValue_ = false;
    std::vector<std::string> args = {"fail"};

    bool result = executor_->execute(args);

    EXPECT_FALSE(result);
}

TEST_F(IApplicationExecutorTest, ExecuteWithEmptyArgs)
{
    std::vector<std::string> args;

    bool result = executor_->execute(args);

    EXPECT_TRUE(result);
    EXPECT_EQ(executor_->argcArg_, 0);
    EXPECT_TRUE(executor_->lastArgs_.empty());
}

TEST_F(IApplicationExecutorTest, ExecuteWithArgcZero)
{
    auto result = executor_->execute(0, nullptr);

    EXPECT_TRUE(result);
    EXPECT_EQ(executor_->argcArg_, 0);
    EXPECT_TRUE(executor_->lastArgs_.empty());
}

TEST_F(IApplicationExecutorTest, ExecuteWithMultipleArgsPreservesOrder)
{
    std::vector<std::string> args = {"first", "second", "third", "fourth"};

    executor_->execute(args);

    ASSERT_EQ(executor_->lastArgs_.size(), 4);
    EXPECT_EQ(executor_->lastArgs_[0], "first");
    EXPECT_EQ(executor_->lastArgs_[1], "second");
    EXPECT_EQ(executor_->lastArgs_[2], "third");
    EXPECT_EQ(executor_->lastArgs_[3], "fourth");
}

TEST_F(IApplicationExecutorTest, ExecuteDoesNotThrow)
{
    std::vector<std::string> args = {"test"};
    EXPECT_NO_THROW(executor_->execute(args));
}

TEST_F(IApplicationExecutorTest, ExecuteReusesExecutor)
{
    executor_->execute({"first"});
    executor_->execute({"second"});

    ASSERT_EQ(executor_->lastArgs_.size(), 1);
    EXPECT_EQ(executor_->lastArgs_[0], "second");
}
