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

using namespace common::system;

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

TEST_F(FunctionProfilerTest, DefaultConstructor_NotStarted)
{
    const FunctionProfiler profiler("test");
    EXPECT_THROW(profiler.getRunTimeMs(), std::runtime_error);
    EXPECT_THROW(profiler.getRunTimeSec(), std::runtime_error);
    EXPECT_THROW(profiler.getRunTime(), std::runtime_error);
}

TEST_F(FunctionProfilerTest, AutoStart_StartsImmediately)
{
    FunctionProfiler profiler("auto", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    profiler.recordEnd();

    EXPECT_NO_THROW(profiler.getRunTimeMs());
    EXPECT_GE(profiler.getRunTimeMs(), 0.0);
}

TEST_F(FunctionProfilerTest, RecordStartEnd_Success)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();
    profiler.recordEnd();

    EXPECT_NO_THROW(profiler.getRunTimeMs());
}

TEST_F(FunctionProfilerTest, GetRunTimeMs_WithoutStart_Throws)
{
    FunctionProfiler profiler("test");
    profiler.recordEnd();

    EXPECT_THROW(profiler.getRunTimeMs(), std::runtime_error);
}

TEST_F(FunctionProfilerTest, GetRunTimeMs_WithoutEnd_Throws)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();

    EXPECT_THROW(profiler.getRunTimeMs(), std::runtime_error);
}

TEST_F(FunctionProfilerTest, GetRunTimeSec_WithoutStart_Throws)
{
    FunctionProfiler profiler("test");
    EXPECT_THROW(profiler.getRunTimeSec(), std::runtime_error);
}

TEST_F(FunctionProfilerTest, GetRunTimeSec_WithoutEnd_Throws)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();

    EXPECT_THROW(profiler.getRunTimeSec(), std::runtime_error);
}

TEST_F(FunctionProfilerTest, GetRunTime_WithoutStart_Throws)
{
    FunctionProfiler profiler("test");
    EXPECT_THROW(profiler.getRunTime(), std::runtime_error);
}

TEST_F(FunctionProfilerTest, GetRunTime_WithoutEnd_Throws)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();

    EXPECT_THROW(profiler.getRunTime(), std::runtime_error);
}

TEST_F(FunctionProfilerTest, GetRunTimeMs_PositiveValue)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    profiler.recordEnd();

    const auto elapsed = profiler.getRunTimeMs();
    EXPECT_GE(elapsed, 5.0);
}

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

TEST_F(FunctionProfilerTest, GetRunTime_FormatPattern)
{
    FunctionProfiler profiler("fmtTest");
    profiler.recordStart();
    profiler.recordEnd();

    const auto result = profiler.getRunTime();
    const std::regex pattern(R"(.+ finished in \d+\.\d{3} s \(\d+\.\d{3} ms\))");
    EXPECT_TRUE(std::regex_match(result, pattern));
}

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

TEST_F(FunctionProfilerTest, GetRunTimeMs_ReturnsMilliseconds)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    profiler.recordEnd();

    const auto elapsed = profiler.getRunTimeMs();
    EXPECT_GE(elapsed, 45.0);
}

TEST_F(FunctionProfilerTest, GetRunTimeSec_ReturnsSeconds)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    profiler.recordEnd();

    const auto elapsed = profiler.getRunTimeSec();
    EXPECT_GE(elapsed, 0.045);
}

TEST_F(FunctionProfilerTest, RecordEnd_AfterStart_SetsEnded)
{
    FunctionProfiler profiler("test");
    profiler.recordStart();
    profiler.recordEnd();

    EXPECT_NO_THROW(profiler.getRunTimeMs());
}

TEST_F(FunctionProfilerTest, RecordStart_ResetsEnded)
{
    FunctionProfiler profiler("test");

    profiler.recordStart();
    profiler.recordEnd();
    EXPECT_NO_THROW(profiler.getRunTimeMs());

    profiler.recordStart();
    EXPECT_THROW(profiler.getRunTimeMs(), std::runtime_error);
}

TEST_F(FunctionProfilerTest, NearZeroDuration_DoesNotThrow)
{
    FunctionProfiler profiler("instant");
    profiler.recordStart();
    profiler.recordEnd();

    EXPECT_NO_THROW(profiler.getRunTimeMs());
    EXPECT_GE(profiler.getRunTimeMs(), 0.0);
}

TEST_F(FunctionProfilerTest, GetRunTime_WithEmptyFunctionName)
{
    FunctionProfiler profiler("");
    profiler.recordStart();
    profiler.recordEnd();

    const auto result = profiler.getRunTime();
    EXPECT_FALSE(result.empty());
}

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
