/**
 * @file IAopAspectTest.cc
 * @brief Unit tests for IAopAspect interface
 * @details Tests cover AOP lifecycle: entry, exit, exception handling, and result processing.
 */

#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

#include "interface/IAopAspect.hpp"

using namespace cppforge::interface::aop;

namespace
{
    /// @brief Mock aspect that records lifecycle events
    class MockAspect : public IAopAspect<MockAspect>
    {
    public:
        int entryCount_ = 0;
        int exitCount_ = 0;
        int exceptionCount_ = 0;
        std::string lastResult_;
        bool throwOnEntry_ = false;
        bool throwOnExit_ = false;

        void onEntry() override
        {
            entryCount_++;
            if (throwOnEntry_)
                throw std::runtime_error("entry failed");
        }

        void onExit() override
        {
            exitCount_++;
            if (throwOnExit_)
                throw std::runtime_error("exit failed");
        }

        void onException(std::exception_ptr) override
        {
            exceptionCount_++;
        }

        template <typename T>
        auto handleResult(T&& result) -> std::decay_t<T>
        {
            if constexpr (std::is_same_v<std::decay_t<T>, std::string>)
                lastResult_ = result;
            return std::forward<T>(result);
        }
    };
}

/// @brief Test fixture for IAopAspect tests.
class IAopAspectTest : public testing::Test
{
protected:
    void SetUp() override
    {
        aspect_ = std::make_unique<MockAspect>();
    }

    void TearDown() override
    {
        aspect_.reset();
    }

    std::unique_ptr<MockAspect> aspect_;
};

/** @brief Verifies onEntry and onExit are called for a void exec(). */
TEST_F(IAopAspectTest, ExecCallsOnEntryAndOnExit)
{
    aspect_->exec([]{});

    EXPECT_EQ(aspect_->entryCount_, 1);
    EXPECT_EQ(aspect_->exitCount_, 1);
    EXPECT_EQ(aspect_->exceptionCount_, 0);
}

/** @brief Verifies exec() returns the value produced by the function. */
TEST_F(IAopAspectTest, ExecReturnsValue)
{
    const auto result = aspect_->exec([](int a, int b) { return a + b; }, 3, 4);

    EXPECT_EQ(result, 7);
    EXPECT_EQ(aspect_->entryCount_, 1);
    EXPECT_EQ(aspect_->exitCount_, 1);
}

/** @brief Verifies exec() handles a string-returning function
    @details Ensures the result is forwarded and recorded by handleResult. */
TEST_F(IAopAspectTest, ExecWithStringResult)
{
    const auto result = aspect_->exec([]() -> std::string { return "hello"; });

    EXPECT_EQ(result, "hello");
    EXPECT_EQ(aspect_->lastResult_, "hello");
}

/** @brief Verifies exec() invokes a void lambda and triggers lifecycle. */
TEST_F(IAopAspectTest, ExecVoidFunction)
{
    bool called = false;
    aspect_->exec([&called] { called = true; });

    EXPECT_TRUE(called);
    EXPECT_EQ(aspect_->entryCount_, 1);
    EXPECT_EQ(aspect_->exitCount_, 1);
}

/** @brief Verifies onException is invoked when the function throws. */
TEST_F(IAopAspectTest, ExecCallsOnExceptionOnThrow)
{
    EXPECT_THROW(
        aspect_->exec([] { throw std::runtime_error("test error"); }),
        std::runtime_error
    );

    EXPECT_EQ(aspect_->entryCount_, 1);
    EXPECT_EQ(aspect_->exceptionCount_, 1);
    EXPECT_EQ(aspect_->exitCount_, 0);
}

/** @brief Verifies the original exception type propagates through exec(). */
TEST_F(IAopAspectTest, ExecPreservesExceptionType)
{
    try
    {
        aspect_->exec([] { throw std::invalid_argument("invalid"); });
        FAIL() << "Expected exception";
    }
    catch (const std::invalid_argument& e)
    {
        EXPECT_STREQ(e.what(), "invalid");
    }
}

/** @brief Verifies arguments are forwarded correctly to the function. */
TEST_F(IAopAspectTest, ExecWithArguments)
{
    const auto result = aspect_->exec([](int x, int y) { return x * y; }, 6, 7);

    EXPECT_EQ(result, 42);
}

/** @brief Verifies repeated exec() calls increment lifecycle counters. */
TEST_F(IAopAspectTest, ExecMultipleTimes)
{
    const auto r1 = aspect_->exec([] { return 1; });
    const auto r2 = aspect_->exec([] { return 2; });

    EXPECT_EQ(r1, 1);
    EXPECT_EQ(r2, 2);
    EXPECT_EQ(aspect_->entryCount_, 2);
    EXPECT_EQ(aspect_->exitCount_, 2);
}

/** @brief Verifies exec() works with lambdas that capture variables. */
TEST_F(IAopAspectTest, ExecWithLambdaCapture)
{
    int x = 10;
    const auto result = aspect_->exec([&x] { return x * 2; });

    EXPECT_EQ(result, 20);
}

/** @brief Verifies exec() does not throw for an empty function. */
TEST_F(IAopAspectTest, ExecEmptyFunctionDoesNotThrow)
{
    EXPECT_NO_THROW(aspect_->exec([]{}));
}

/** @brief Verifies onEntry exception is propagated and onExit is skipped. */
TEST_F(IAopAspectTest, ExecHandlesExceptionsFromOnEntry)
{
    aspect_->throwOnEntry_ = true;

    EXPECT_THROW(aspect_->exec([]{}), std::runtime_error);
    EXPECT_EQ(aspect_->entryCount_, 1);
    EXPECT_EQ(aspect_->exitCount_, 0);
}

/** @brief Verifies onExit exception is propagated after entry succeeds. */
TEST_F(IAopAspectTest, ExecHandlesExceptionsFromOnExit)
{
    aspect_->throwOnExit_ = true;

    EXPECT_THROW(aspect_->exec([]{}), std::runtime_error);
    EXPECT_EQ(aspect_->entryCount_, 1);
    EXPECT_EQ(aspect_->exitCount_, 1);
}
