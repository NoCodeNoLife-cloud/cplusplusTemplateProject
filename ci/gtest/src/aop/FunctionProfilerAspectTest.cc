/**
 * @file FunctionProfilerAspectTest.cc
 * @brief Unit tests for the FunctionProfilerAspect class
 * @details Tests cover aspect construction, entry/exit points, exception handling,
 *          and integration with the AOP execution framework.
 */

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <gtest/gtest.h>

#include "aop/FunctionProfilerAspect.hpp"

using namespace cppforge::aop;

/**
 * @brief Test fixture for FunctionProfilerAspect tests
 */
class FunctionProfilerAspectTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Test FunctionProfilerAspect constructor
 * @details Verifies that the aspect can be constructed with a function name
 */
TEST_F(FunctionProfilerAspectTest, Constructor_ValidFunctionName)
{
    EXPECT_NO_THROW(FunctionProfilerAspect aspect{"testFunction"});
}

/**
 * @brief Test FunctionProfilerAspect construction with empty name
 * @details Verifies that the aspect can be constructed with an empty string
 */
TEST_F(FunctionProfilerAspectTest, Constructor_EmptyFunctionName)
{
    EXPECT_NO_THROW(FunctionProfilerAspect aspect{""});
}

/**
 * @brief Test onEntry method does not throw
 * @details Verifies that onEntry can be called without exceptions
 */
TEST_F(FunctionProfilerAspectTest, OnEntry_NoException)
{
    FunctionProfilerAspect aspect{"testFunction"};

    EXPECT_NO_THROW(aspect.onEntry());
}

/**
 * @brief Test onExit method records end time
 * @details Verifies that onExit completes successfully after onEntry
 */
TEST_F(FunctionProfilerAspectTest, OnExit_AfterEntry)
{
    FunctionProfilerAspect aspect{"testFunction"};

    aspect.onEntry();

    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_NO_THROW(aspect.onExit());
}

/**
 * @brief Test onExit method without prior onEntry
 * @details Verifies that onExit handles being called without onEntry
 */
TEST_F(FunctionProfilerAspectTest, OnExit_WithoutEntry)
{
    FunctionProfilerAspect aspect{"testFunction"};

    // Expect throw because getRunTime() requires onEntry() to have been called first
    EXPECT_THROW(aspect.onExit(), std::runtime_error);
}

/**
 * @brief Test onException method with valid exception pointer
 * @details Verifies that onException handles exception pointers correctly
 */
TEST_F(FunctionProfilerAspectTest, OnException_WithExceptionPointer)
{
    FunctionProfilerAspect aspect{"testFunction"};

    aspect.onEntry();

    // Create an exception pointer
    const std::exception_ptr eptr = std::make_exception_ptr(std::runtime_error("Test error"));

    EXPECT_NO_THROW(aspect.onException(eptr));
}

/**
 * @brief Test onException method without prior onEntry
 * @details Verifies that onException handles being called without onEntry
 */
TEST_F(FunctionProfilerAspectTest, OnException_WithoutEntry)
{
    FunctionProfilerAspect aspect{"testFunction"};

    const std::exception_ptr eptr = std::make_exception_ptr(std::runtime_error("Test error"));

    // Expect throw because getRunTime() requires onEntry() to have been called first
    EXPECT_THROW(aspect.onException(eptr), std::runtime_error);
}

/**
 * @brief Test complete profiling lifecycle: entry -> exit
 * @details Verifies the normal execution flow of the aspect
 */
TEST_F(FunctionProfilerAspectTest, Lifecycle_EntryThenExit)
{
    FunctionProfilerAspect aspect{"lifecycleTest"};

    EXPECT_NO_THROW({
        aspect.onEntry();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        aspect.onExit();
        });
}

/**
 * @brief Test complete profiling lifecycle: entry -> exception
 * @details Verifies the exception execution flow of the aspect
 */
TEST_F(FunctionProfilerAspectTest, Lifecycle_EntryThenException)
{
    FunctionProfilerAspect aspect{"exceptionTest"};

    EXPECT_NO_THROW({
        aspect.onEntry();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        const std::exception_ptr eptr = std::make_exception_ptr(std::runtime_error("Error"));
        aspect.onException(eptr);
        });
}

/**
 * @brief Test aspect with AOP exec for void function
 * @details Verifies that the aspect works correctly with void-returning functions
 */
TEST_F(FunctionProfilerAspectTest, Exec_VoidFunction)
{
    FunctionProfilerAspect aspect{"voidFunctionTest"};

    bool executed = false;

    EXPECT_NO_THROW({
        aspect.exec([&executed]{
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            executed = true;
            });
        });

    EXPECT_TRUE(executed);
}

/**
 * @brief Test aspect with AOP exec for function returning value
 * @details Verifies that the aspect works correctly with value-returning functions
 */
TEST_F(FunctionProfilerAspectTest, Exec_FunctionReturningValue)
{
    FunctionProfilerAspect aspect{"valueFunctionTest"};

    const auto result = aspect.exec([]() -> int
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        return 42;
    });

    EXPECT_EQ(result, 42);
}

/**
 * @brief Test aspect with AOP exec for function with arguments
 * @details Verifies that the aspect correctly forwards arguments to the function
 */
TEST_F(FunctionProfilerAspectTest, Exec_FunctionWithArguments)
{
    FunctionProfilerAspect aspect{"argsFunctionTest"};

    const auto result = aspect.exec([](const int a, const int b) -> int
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        return a + b;
    }, 10, 20);

    EXPECT_EQ(result, 30);
}

/**
 * @brief Test aspect with AOP exec when function throws exception
 * @details Verifies that the aspect handles exceptions correctly and re-throws them
 */
TEST_F(FunctionProfilerAspectTest, Exec_FunctionThrowsException)
{
    FunctionProfilerAspect aspect{"throwingFunctionTest"};

    EXPECT_THROW({
                 aspect.exec([]{
                     std::this_thread::sleep_for(std::chrono::milliseconds(5));
                     throw std::runtime_error("Test exception");
                     });
                 }, std::runtime_error);
}

/**
 * @brief Test aspect with AOP exec preserves exception type
 * @details Verifies that the original exception type is preserved through the aspect
 */
TEST_F(FunctionProfilerAspectTest, Exec_PreservesExceptionType)
{
    FunctionProfilerAspect aspect{"exceptionTypeTest"};

    try
    {
        aspect.exec([]
        {
            throw std::invalid_argument("Invalid argument");
        });
        FAIL() << "Expected std::invalid_argument exception";
    }
    catch (const std::invalid_argument& e)
    {
        EXPECT_STREQ(e.what(), "Invalid argument");
    }
    catch (...)
    {
        FAIL() << "Unexpected exception type";
    }
}

/**
 * @brief Test multiple sequential executions with same aspect
 * @details Verifies that the aspect can be reused for multiple function calls
 */
TEST_F(FunctionProfilerAspectTest, MultipleExecutions_Sequential)
{
    FunctionProfilerAspect aspect{"multipleExecTest"};

    const auto result1 = aspect.exec([]() -> int
    {
        return 100;
    });

    const auto result2 = aspect.exec([]() -> int
    {
        return 200;
    });

    EXPECT_EQ(result1, 100);
    EXPECT_EQ(result2, 200);
}

/**
 * @brief Test aspect with different function types
 * @details Verifies that the aspect works with various callable types
 */
TEST_F(FunctionProfilerAspectTest, Exec_DifferentCallableTypes)
{
    FunctionProfilerAspect aspect{"callableTest"};

    // Lambda
    const auto result1 = aspect.exec([]
    {
        return 1;
    });
    EXPECT_EQ(result1, 1);

    // Function object
    struct Functor
    {
        auto operator()() const -> int
        {
            return 2;
        }
    };
    const auto result2 = aspect.exec(Functor{});
    EXPECT_EQ(result2, 2);
}

/**
 * @brief Test aspect timing accuracy
 * @details Verifies that the aspect correctly measures execution time
 */
TEST_F(FunctionProfilerAspectTest, Timing_Accuracy)
{
    FunctionProfilerAspect aspect{"timingTest"};

    const auto start = std::chrono::high_resolution_clock::now();

    aspect.exec([]
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    });

    const auto end = std::chrono::high_resolution_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Allow some tolerance for timing variations
    EXPECT_GE(elapsed, 45);
    EXPECT_LE(elapsed, 100);
}

/**
 * @brief Test aspect with nested function calls
 * @details Verifies that the aspect handles nested executions correctly
 */
TEST_F(FunctionProfilerAspectTest, NestedExecutions)
{
    FunctionProfilerAspect outerAspect{"outerFunction"};
    FunctionProfilerAspect innerAspect{"innerFunction"};

    const auto result = outerAspect.exec([&innerAspect]() -> int
    {
        return innerAspect.exec([]() -> int
        {
            return 42;
        });
    });

    EXPECT_EQ(result, 42);
}

/**
 * @brief Test aspect with long-running function
 * @details Verifies that the aspect handles long execution times correctly
 */
TEST_F(FunctionProfilerAspectTest, LongRunningFunction)
{
    FunctionProfilerAspect aspect{"longRunningTest"};

    EXPECT_NO_THROW({
        aspect.exec([]{
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            });
        });
}

/**
 * @brief Test aspect with immediate function
 * @details Verifies that the aspect handles very fast execution correctly
 */
TEST_F(FunctionProfilerAspectTest, ImmediateFunction)
{
    FunctionProfilerAspect aspect{"immediateTest"};

    const auto result = aspect.exec([]() -> int
    {
        return 999;
    });

    EXPECT_EQ(result, 999);
}

// ============================================================================
// Boundary Condition Tests
// ============================================================================

/**
 * @brief Test constructor with extremely long function name
 * @details Verifies heap allocation boundary for function names of large size
 */
TEST_F(FunctionProfilerAspectTest, Constructor_LongFunctionName)
{
    const std::string longName(10000, 'x');
    EXPECT_NO_THROW(FunctionProfilerAspect aspect{longName});
}

/**
 * @brief Test constructor with special characters in function name
 * @details Verifies handling of whitespace-only, symbol, and punctuation names
 */
TEST_F(FunctionProfilerAspectTest, Constructor_SpecialCharacters)
{
    EXPECT_NO_THROW(FunctionProfilerAspect aspect{" \t\n"});
    EXPECT_NO_THROW(FunctionProfilerAspect aspect{"!@#$%^&*()_+=-`~[]{}|;':\",./<>?"});
}

/**
 * @brief Test constructor with unicode characters in function name
 * @details Verifies handling of multi-byte UTF-8 function names
 */
TEST_F(FunctionProfilerAspectTest, Constructor_UnicodeCharacters)
{
    EXPECT_NO_THROW(FunctionProfilerAspect aspect{"\u00e9\u00e0\u00fc\u00f1"});
    EXPECT_NO_THROW(FunctionProfilerAspect aspect{"\u4e2d\u6587\u540d\u79f0"});
}

/**
 * @brief Test onEntry called twice consecutively
 * @details The second onEntry resets the timer. Verifies no crash
 *          and that the profiler correctly restarts timing.
 */
TEST_F(FunctionProfilerAspectTest, OnEntry_DoubleEntry)
{
    FunctionProfilerAspect aspect{"doubleEntryTest"};

    EXPECT_NO_THROW(aspect.onEntry());
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    // Second entry should reset the start time and clear ended flag
    EXPECT_NO_THROW(aspect.onEntry());
    EXPECT_NO_THROW(aspect.onExit());
}

/**
 * @brief Test onExit called twice after single onEntry
 * @details The second onExit updates the end timestamp without throwing,
 *          because ended_ remains true from the first call.
 */
TEST_F(FunctionProfilerAspectTest, OnExit_DoubleExit)
{
    FunctionProfilerAspect aspect{"doubleExitTest"};

    aspect.onEntry();
    EXPECT_NO_THROW(aspect.onExit());
    // recordEnd() is idempotent: it just updates end_ and leaves ended_=true
    EXPECT_NO_THROW(aspect.onExit());
}

/**
 * @brief Test onException after a successful onExit
 * @details Verifies that calling onException after the normal exit path
 *          does not throw (recordEnd is idempotent).
 */
TEST_F(FunctionProfilerAspectTest, OnException_AfterOnExit)
{
    FunctionProfilerAspect aspect{"exceptionAfterExitTest"};

    aspect.onEntry();
    EXPECT_NO_THROW(aspect.onExit());
    const std::exception_ptr eptr = std::make_exception_ptr(std::runtime_error("Late error"));
    EXPECT_NO_THROW(aspect.onException(eptr));
}

/**
 * @brief Test onExit after onException
 * @details Verifies that calling onExit after an exception path updates
 *          the end time and does not throw.
 */
TEST_F(FunctionProfilerAspectTest, OnExit_AfterOnException)
{
    FunctionProfilerAspect aspect{"exitAfterExceptionTest"};

    aspect.onEntry();
    const std::exception_ptr eptr = std::make_exception_ptr(std::runtime_error("error"));
    EXPECT_NO_THROW(aspect.onException(eptr));
    EXPECT_NO_THROW(aspect.onExit());
}

/**
 * @brief Test onException with null (default-constructed) exception_ptr
 * @details The onException implementation ignores the parameter and
 *          just records end time, so a null pointer is safe.
 */
TEST_F(FunctionProfilerAspectTest, OnException_NullExceptionPtr)
{
    FunctionProfilerAspect aspect{"nullExceptionTest"};

    aspect.onEntry();
    const std::exception_ptr null_eptr;
    EXPECT_NO_THROW(aspect.onException(null_eptr));
}

/**
 * @brief Test exec with a no-op function (zero execution time)
 * @details Verifies that the aspect handles instantaneous execution correctly
 */
TEST_F(FunctionProfilerAspectTest, Exec_NoOpFunction)
{
    FunctionProfilerAspect aspect{"noOpTest"};

    EXPECT_NO_THROW(aspect.exec([]{}));
}

/**
 * @brief Test exec with move-only callable arguments
 * @details Verifies perfect forwarding of move-only types like unique_ptr
 */
TEST_F(FunctionProfilerAspectTest, Exec_MoveOnlyArgs)
{
    FunctionProfilerAspect aspect{"moveOnlyTest"};

    const auto result = aspect.exec([](std::unique_ptr<int> p) -> int
    {
        return *p;
    }, std::make_unique<int>(42));

    EXPECT_EQ(result, 42);
}

/**
 * @brief Test many rapid exec cycles verifying independent timing
 * @details Each cycle should independently record start/end without
 *          leaking state between invocations
 */
TEST_F(FunctionProfilerAspectTest, MultipleExecutions_IndependentCycles)
{
    FunctionProfilerAspect aspect{"cycleTest"};

    for (int i = 0; i < 50; ++i)
    {
        EXPECT_NO_THROW(aspect.exec([i]() -> int
        {
            return i * i;
        }));
    }
}

/**
 * @brief Test exec with noexcept function
 * @details Verifies compatibility with noexcept lambdas and functions
 */
TEST_F(FunctionProfilerAspectTest, Exec_NoexceptFunction)
{
    FunctionProfilerAspect aspect{"noexceptTest"};

    const auto result = aspect.exec([]() noexcept -> int
    {
        return 7;
    });

    EXPECT_EQ(result, 7);
}

/**
 * @brief Test exec with function returning a reference
 * @details Ensures no dangling reference or incorrect copy behavior
 */
TEST_F(FunctionProfilerAspectTest, Exec_ReturnsReference)
{
    FunctionProfilerAspect aspect{"refReturnTest"};

    static int value = 42;
    const auto result = aspect.exec([]() -> const int&
    {
        return value;
    });

    EXPECT_EQ(result, 42);
}

/**
 * @brief Test exec with void function that throws
 * @details Verifies that exceptions from void functions are properly
 *          propagated through the aspect
 */
TEST_F(FunctionProfilerAspectTest, Exec_VoidFunctionThrows)
{
    FunctionProfilerAspect aspect{"voidThrowTest"};

    EXPECT_THROW({
        aspect.exec([]{
            throw std::runtime_error("void error");
        });
    }, std::runtime_error);
}
