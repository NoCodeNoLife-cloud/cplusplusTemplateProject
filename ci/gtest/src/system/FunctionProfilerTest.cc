/**
 * @file FunctionProfilerTest.cc
 * @brief Unit tests for the FunctionProfiler class
 * @details Tests cover construction, timing lifecycle, error handling,
 *          reuse semantics, and timing measurement correctness.
 */

#include <chrono>
#include <regex>
#include <stdexcept>
#include <thread>
#include <gtest/gtest.h>

#include "system/FunctionProfiler.hpp"

using namespace cppforge::system;

/// @brief Test fixture for FunctionProfiler tests.
class FunctionProfilerTest : public testing::Test
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
 * @brief Default constructor does not start profiling
 * @details Verifies getters throw before recordStart is called.
 */
TEST_F(FunctionProfilerTest, DefaultConstructor_NotStarted)
{
    const FunctionProfiler profiler("test");
    EXPECT_THROW(profiler.getRunTimeMs(), std::runtime_error);
    EXPECT_THROW(profiler.getRunTimeSec(), std::runtime_error);
    EXPECT_THROW(profiler.getRunTime(), std::runtime_error);
}

/**
 * @brief Auto-start mode begins immediately
 * @details Verifies auto-start constructor records time from creation.
 */
TEST_F(FunctionProfilerTest, AutoStart_StartsImmediately)
{
    FunctionProfiler profiler("auto", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    profiler.recordEnd();

    EXPECT_NO_THROW(profiler.getRunTimeMs());
    EXPECT_GE(profiler.getRunTimeMs(), 0.0);
}

/**
 * @brief Record start and end completes successfully
 * @details Verifies basic recordStart/recordEnd lifecycle.
 */
TEST_F(FunctionProfilerTest, RecordStartEnd_Success)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();
    profiler.recordEnd();

    EXPECT_NO_THROW(profiler.getRunTimeMs());
}

/**
 * @brief GetRunTimeMs throws if end called before start
 * @details Verifies error when recordEnd is called without recordStart.
 */
TEST_F(FunctionProfilerTest, GetRunTimeMs_WithoutStart_Throws)
{
    FunctionProfiler profiler("test");
    profiler.recordEnd();

    EXPECT_THROW(profiler.getRunTimeMs(), std::runtime_error);
}

/**
 * @brief GetRunTimeMs throws if end not yet called
 * @details Verifies error when querying before profiling completes.
 */
TEST_F(FunctionProfilerTest, GetRunTimeMs_WithoutEnd_Throws)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();

    EXPECT_THROW(profiler.getRunTimeMs(), std::runtime_error);
}

/**
 * @brief GetRunTimeSec throws if never started
 * @details Verifies error on getRunTimeSec without any recording.
 */
TEST_F(FunctionProfilerTest, GetRunTimeSec_WithoutStart_Throws)
{
    FunctionProfiler profiler("test");
    EXPECT_THROW(profiler.getRunTimeSec(), std::runtime_error);
}

/**
 * @brief GetRunTimeSec throws if profiling not ended
 * @details Verifies error on getRunTimeSec before recordEnd.
 */
TEST_F(FunctionProfilerTest, GetRunTimeSec_WithoutEnd_Throws)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();

    EXPECT_THROW(profiler.getRunTimeSec(), std::runtime_error);
}

/**
 * @brief GetRunTime throws if never started
 * @details Verifies error on getRunTime formatted string without recording.
 */
TEST_F(FunctionProfilerTest, GetRunTime_WithoutStart_Throws)
{
    FunctionProfiler profiler("test");
    EXPECT_THROW(profiler.getRunTime(), std::runtime_error);
}

/**
 * @brief GetRunTime throws if profiling not ended
 * @details Verifies error on formatted output before recordEnd.
 */
TEST_F(FunctionProfilerTest, GetRunTime_WithoutEnd_Throws)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();

    EXPECT_THROW(profiler.getRunTime(), std::runtime_error);
}

/**
 * @brief GetRunTimeMs returns a positive measured value
 * @details Verifies measured time is at least the sleep duration.
 */
TEST_F(FunctionProfilerTest, GetRunTimeMs_PositiveValue)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    profiler.recordEnd();

    const auto elapsed = profiler.getRunTimeMs();
    EXPECT_GE(elapsed, 5.0);
}

/**
 * @brief GetRunTimeSec is consistent with GetRunTimeMs
 * @details Verifies seconds value matches milliseconds within tolerance.
 */
TEST_F(FunctionProfilerTest, GetRunTimeSec_MatchesMs)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    profiler.recordEnd();

    const auto elapsed_ms = profiler.getRunTimeMs();
    const auto elapsed_sec = profiler.getRunTimeSec();

    EXPECT_NEAR(elapsed_sec * 1000.0, elapsed_ms, 1.0);
}

/**
 * @brief GetRunTime formatted string contains function name
 * @details Verifies the output includes the function name and time units.
 */
TEST_F(FunctionProfilerTest, GetRunTime_ContainsFunctionName)
{
    FunctionProfiler profiler("myFunction");
    profiler.recordStart();
    profiler.recordEnd();

    const auto result = profiler.getRunTime();
    EXPECT_TRUE(result.find("myFunction") != std::string::npos);
    EXPECT_TRUE(result.find("finished in") != std::string::npos);
    EXPECT_TRUE(result.find("s") != std::string::npos);
    EXPECT_TRUE(result.find("ms") != std::string::npos);
}

/**
 * @brief GetRunTime output matches expected regex format
 * @details Verifies the formatted string matches the expected pattern.
 */
TEST_F(FunctionProfilerTest, GetRunTime_FormatPattern)
{
    FunctionProfiler profiler("fmtTest");
    profiler.recordStart();
    profiler.recordEnd();

    const auto result = profiler.getRunTime();
    const std::regex pattern(R"(.+ finished in \d+\.\d{3} s \(\d+\.\d{3} ms\))");
    EXPECT_TRUE(std::regex_match(result, pattern));
}

/**
 * @brief Reuse profiler across multiple start/end cycles
 * @details Verifies profiler can be reused after recording completes.
 */
TEST_F(FunctionProfilerTest, Reuse_MultipleStartEndCycles)
{
    FunctionProfiler profiler("reuse");

    profiler.recordStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    profiler.recordEnd();
    const auto first = profiler.getRunTimeMs();

    profiler.recordStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    profiler.recordEnd();
    const auto second = profiler.getRunTimeMs();

    EXPECT_GE(first, 0.0);
    EXPECT_GE(second, 0.0);
}

/**
 * @brief GetRunTimeMs returns milliseconds after longer sleep
 * @details Verifies correct measurement for a 50ms sleep.
 */
TEST_F(FunctionProfilerTest, GetRunTimeMs_ReturnsMilliseconds)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    profiler.recordEnd();

    const auto elapsed = profiler.getRunTimeMs();
    EXPECT_GE(elapsed, 45.0);
}

/**
 * @brief GetRunTimeSec returns seconds after longer sleep
 * @details Verifies correct seconds measurement for a 50ms sleep.
 */
TEST_F(FunctionProfilerTest, GetRunTimeSec_ReturnsSeconds)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    profiler.recordEnd();

    const auto elapsed = profiler.getRunTimeSec();
    EXPECT_GE(elapsed, 0.045);
}

/**
 * @brief RecordEnd after RecordStart marks ended state
 * @details Verifies that a complete cycle allows time retrieval.
 */
TEST_F(FunctionProfilerTest, RecordEnd_AfterStart_SetsEnded)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();
    profiler.recordEnd();

    EXPECT_NO_THROW(profiler.getRunTimeMs());
}

/**
 * @brief RecordStart resets the ended state
 * @details Verifies that a new start invalidates previous timing.
 */
TEST_F(FunctionProfilerTest, RecordStart_ResetsEnded)
{
    FunctionProfiler profiler("test");

    profiler.recordStart();
    profiler.recordEnd();
    EXPECT_NO_THROW(profiler.getRunTimeMs());

    profiler.recordStart();
    EXPECT_THROW(profiler.getRunTimeMs(), std::runtime_error);
}

/**
 * @brief Near-zero duration does not throw
 * @details Verifies that instant start/end produces a valid time.
 */
TEST_F(FunctionProfilerTest, NearZeroDuration_DoesNotThrow)
{
    FunctionProfiler profiler("instant");
    profiler.recordStart();
    profiler.recordEnd();

    EXPECT_NO_THROW(profiler.getRunTimeMs());
    EXPECT_GE(profiler.getRunTimeMs(), 0.0);
}

/**
 * @brief GetRunTime works with empty function name
 * @details Verifies no crash when the function name is an empty string.
 */
TEST_F(FunctionProfilerTest, GetRunTime_WithEmptyFunctionName)
{
    FunctionProfiler profiler("");
    profiler.recordStart();
    profiler.recordEnd();

    const auto result = profiler.getRunTime();
    EXPECT_FALSE(result.empty());
}

/**
 * @brief Multiple GetRunTimeMs calls return consistent values
 * @details Verifies repeated reads after end return the same result.
 */
TEST_F(FunctionProfilerTest, MultipleGetRunTimeCalls_Consistent)
{
    FunctionProfiler profiler("consistent");
    profiler.recordStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    profiler.recordEnd();

    const auto first = profiler.getRunTimeMs();
    const auto second = profiler.getRunTimeMs();

    EXPECT_DOUBLE_EQ(first, second);
}
