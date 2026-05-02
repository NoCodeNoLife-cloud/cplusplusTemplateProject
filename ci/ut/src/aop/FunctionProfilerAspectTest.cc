/**
 * @file FunctionProfilerAspectTest.cc
 * @brief Unit tests for the FunctionProfilerAspect class
 * @details Tests cover aspect construction, entry/exit points, exception handling,
 *          and integration with the AOP execution framework.
 */

#include <gtest/gtest.h>
#include "aop/FunctionProfilerAspect.hpp"
#include <string>
#include <stdexcept>
#include <thread>
#include <chrono>

using namespace common::aop;

/**
 * @brief Test FunctionProfilerAspect constructor
 * @details Verifies that the aspect can be constructed with a function name
 */
TEST(FunctionProfilerAspectTest, Constructor_ValidFunctionName) {
    EXPECT_NO_THROW(FunctionProfilerAspect aspect{"testFunction"});
}

/**
 * @brief Test FunctionProfilerAspect construction with empty name
 * @details Verifies that the aspect can be constructed with an empty string
 */
TEST(FunctionProfilerAspectTest, Constructor_EmptyFunctionName) {
    EXPECT_NO_THROW(FunctionProfilerAspect aspect{""});
}

/**
 * @brief Test onEntry method does not throw
 * @details Verifies that onEntry can be called without exceptions
 */
TEST(FunctionProfilerAspectTest, OnEntry_NoException) {
    FunctionProfilerAspect aspect{"testFunction"};
    
    EXPECT_NO_THROW(aspect.onEntry());
}

/**
 * @brief Test onExit method records end time
 * @details Verifies that onExit completes successfully after onEntry
 */
TEST(FunctionProfilerAspectTest, OnExit_AfterEntry) {
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
TEST(FunctionProfilerAspectTest, OnExit_WithoutEntry) {
    FunctionProfilerAspect aspect{"testFunction"};
    
    // Should not throw even if onEntry was not called
    EXPECT_NO_THROW(aspect.onExit());
}

/**
 * @brief Test onException method with valid exception pointer
 * @details Verifies that onException handles exception pointers correctly
 */
TEST(FunctionProfilerAspectTest, OnException_WithExceptionPointer) {
    FunctionProfilerAspect aspect{"testFunction"};
    
    aspect.onEntry();
    
    // Create an exception pointer
    std::exception_ptr eptr = std::make_exception_ptr(std::runtime_error("Test error"));
    
    EXPECT_NO_THROW(aspect.onException(eptr));
}

/**
 * @brief Test onException method without prior onEntry
 * @details Verifies that onException handles being called without onEntry
 */
TEST(FunctionProfilerAspectTest, OnException_WithoutEntry) {
    FunctionProfilerAspect aspect{"testFunction"};
    
    std::exception_ptr eptr = std::make_exception_ptr(std::runtime_error("Test error"));
    
    EXPECT_NO_THROW(aspect.onException(eptr));
}

/**
 * @brief Test complete profiling lifecycle: entry -> exit
 * @details Verifies the normal execution flow of the aspect
 */
TEST(FunctionProfilerAspectTest, Lifecycle_EntryThenExit) {
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
TEST(FunctionProfilerAspectTest, Lifecycle_EntryThenException) {
    FunctionProfilerAspect aspect{"exceptionTest"};
    
    EXPECT_NO_THROW({
        aspect.onEntry();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        std::exception_ptr eptr = std::make_exception_ptr(std::runtime_error("Error"));
        aspect.onException(eptr);
    });
}

/**
 * @brief Test aspect with AOP exec for void function
 * @details Verifies that the aspect works correctly with void-returning functions
 */
TEST(FunctionProfilerAspectTest, Exec_VoidFunction) {
    FunctionProfilerAspect aspect{"voidFunctionTest"};
    
    bool executed = false;
    
    EXPECT_NO_THROW({
        aspect.exec([&executed]() {
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
TEST(FunctionProfilerAspectTest, Exec_FunctionReturningValue) {
    FunctionProfilerAspect aspect{"valueFunctionTest"};
    
    auto result = aspect.exec([]() -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        return 42;
    });
    
    EXPECT_EQ(result, 42);
}

/**
 * @brief Test aspect with AOP exec for function with arguments
 * @details Verifies that the aspect correctly forwards arguments to the function
 */
TEST(FunctionProfilerAspectTest, Exec_FunctionWithArguments) {
    FunctionProfilerAspect aspect{"argsFunctionTest"};
    
    auto result = aspect.exec([](int a, int b) -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        return a + b;
    }, 10, 20);
    
    EXPECT_EQ(result, 30);
}

/**
 * @brief Test aspect with AOP exec when function throws exception
 * @details Verifies that the aspect handles exceptions correctly and re-throws them
 */
TEST(FunctionProfilerAspectTest, Exec_FunctionThrowsException) {
    FunctionProfilerAspect aspect{"throwingFunctionTest"};
    
    EXPECT_THROW({
        aspect.exec([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            throw std::runtime_error("Test exception");
        });
    }, std::runtime_error);
}

/**
 * @brief Test aspect with AOP exec preserves exception type
 * @details Verifies that the original exception type is preserved through the aspect
 */
TEST(FunctionProfilerAspectTest, Exec_PreservesExceptionType) {
    FunctionProfilerAspect aspect{"exceptionTypeTest"};
    
    try {
        aspect.exec([]() {
            throw std::invalid_argument("Invalid argument");
        });
        FAIL() << "Expected std::invalid_argument exception";
    } catch (const std::invalid_argument &e) {
        EXPECT_STREQ(e.what(), "Invalid argument");
    } catch (...) {
        FAIL() << "Unexpected exception type";
    }
}

/**
 * @brief Test multiple sequential executions with same aspect
 * @details Verifies that the aspect can be reused for multiple function calls
 */
TEST(FunctionProfilerAspectTest, MultipleExecutions_Sequential) {
    FunctionProfilerAspect aspect{"multipleExecTest"};
    
    auto result1 = aspect.exec([]() -> int {
        return 100;
    });
    
    auto result2 = aspect.exec([]() -> int {
        return 200;
    });
    
    EXPECT_EQ(result1, 100);
    EXPECT_EQ(result2, 200);
}

/**
 * @brief Test aspect with different function types
 * @details Verifies that the aspect works with various callable types
 */
TEST(FunctionProfilerAspectTest, Exec_DifferentCallableTypes) {
    FunctionProfilerAspect aspect{"callableTest"};
    
    // Lambda
    auto result1 = aspect.exec([]() { return 1; });
    EXPECT_EQ(result1, 1);
    
    // Function object
    struct Functor {
        auto operator()() const -> int { return 2; }
    };
    auto result2 = aspect.exec(Functor{});
    EXPECT_EQ(result2, 2);
}

/**
 * @brief Test aspect timing accuracy
 * @details Verifies that the aspect correctly measures execution time
 */
TEST(FunctionProfilerAspectTest, Timing_Accuracy) {
    FunctionProfilerAspect aspect{"timingTest"};
    
    const auto start = std::chrono::high_resolution_clock::now();
    
    aspect.exec([]() {
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
TEST(FunctionProfilerAspectTest, NestedExecutions) {
    FunctionProfilerAspect outerAspect{"outerFunction"};
    FunctionProfilerAspect innerAspect{"innerFunction"};
    
    auto result = outerAspect.exec([&innerAspect]() -> int {
        return innerAspect.exec([]() -> int {
            return 42;
        });
    });
    
    EXPECT_EQ(result, 42);
}

/**
 * @brief Test aspect with long-running function
 * @details Verifies that the aspect handles long execution times correctly
 */
TEST(FunctionProfilerAspectTest, LongRunningFunction) {
    FunctionProfilerAspect aspect{"longRunningTest"};
    
    EXPECT_NO_THROW({
        aspect.exec([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    });
}

/**
 * @brief Test aspect with immediate function
 * @details Verifies that the aspect handles very fast execution correctly
 */
TEST(FunctionProfilerAspectTest, ImmediateFunction) {
    FunctionProfilerAspect aspect{"immediateTest"};
    
    auto result = aspect.exec([]() -> int {
        return 999;
    });
    
    EXPECT_EQ(result, 999);
}
