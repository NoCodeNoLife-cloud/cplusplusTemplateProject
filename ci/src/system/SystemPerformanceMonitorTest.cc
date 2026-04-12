/**
 * @file SystemPerformanceMonitorTest.cc
 * @brief Unit tests for the SystemPerformanceMonitor class
 * @details Tests cover core system performance monitoring functionality including
 *          memory usage retrieval and CPU usage measurement.
 */

#include <gtest/gtest.h>
#include "system/SystemPerformanceMonitor.hpp"
#include <chrono>

using namespace common::system;

/**
 * @brief Test GetMemoryUsage returns valid structure
 * @details Verifies that memory usage retrieval returns a properly populated structure
 */
TEST(SystemPerformanceMonitorTest, GetMemoryUsage_ReturnsValidStructure) {
    const MemoryUsage memUsage = SystemPerformanceMonitor::GetMemoryUsage();
    
    // All fields should be accessible
    EXPECT_GE(memUsage.total_memory, 0);
    EXPECT_GE(memUsage.available_memory, 0);
    EXPECT_GE(memUsage.used_memory, 0);
    EXPECT_GE(memUsage.memory_usage_percent, 0.0);
}

/**
 * @brief Test GetMemoryUsage total memory is reasonable
 * @details Ensures total physical memory is within expected bounds (at least 1GB)
 */
TEST(SystemPerformanceMonitorTest, GetMemoryUsage_TotalMemoryReasonable) {
    const MemoryUsage memUsage = SystemPerformanceMonitor::GetMemoryUsage();
    
    // Total memory should be at least 1GB (1073741824 bytes) on modern systems
    const ULONGLONG minMemory = 1073741824ULL; // 1 GB
    EXPECT_GE(memUsage.total_memory, minMemory);
}

/**
 * @brief Test GetMemoryUsage available memory does not exceed total
 * @details Verifies that available memory is less than or equal to total memory
 */
TEST(SystemPerformanceMonitorTest, GetMemoryUsage_AvailableNotExceedTotal) {
    const MemoryUsage memUsage = SystemPerformanceMonitor::GetMemoryUsage();
    
    EXPECT_LE(memUsage.available_memory, memUsage.total_memory);
}

/**
 * @brief Test GetMemoryUsage used memory calculation is correct
 * @details Verifies that used memory equals total minus available
 */
TEST(SystemPerformanceMonitorTest, GetMemoryUsage_UsedMemoryCalculation) {
    const MemoryUsage memUsage = SystemPerformanceMonitor::GetMemoryUsage();
    
    const ULONGLONG calculatedUsed = memUsage.total_memory - memUsage.available_memory;
    EXPECT_EQ(memUsage.used_memory, calculatedUsed);
}

/**
 * @brief Test GetMemoryUsage usage percentage is in valid range
 * @details Ensures memory usage percentage is between 0.0 and 100.0
 */
TEST(SystemPerformanceMonitorTest, GetMemoryUsage_PercentageInRange) {
    const MemoryUsage memUsage = SystemPerformanceMonitor::GetMemoryUsage();
    
    EXPECT_GE(memUsage.memory_usage_percent, 0.0);
    EXPECT_LE(memUsage.memory_usage_percent, 100.0);
}

/**
 * @brief Test GetMemoryUsage usage percentage calculation accuracy
 * @details Verifies that percentage matches the ratio of used to total memory
 */
TEST(SystemPerformanceMonitorTest, GetMemoryUsage_PercentageAccuracy) {
    const MemoryUsage memUsage = SystemPerformanceMonitor::GetMemoryUsage();
    
    if (memUsage.total_memory > 0) {
        const double expectedPercent = static_cast<double>(memUsage.used_memory) / 
                                       static_cast<double>(memUsage.total_memory) * 100.0;
        EXPECT_NEAR(memUsage.memory_usage_percent, expectedPercent, 0.01);
    }
}

/**
 * @brief Test GetMemoryUsage consistency across multiple calls
 * @details Verifies that consecutive calls return similar results (within reasonable variance)
 */
TEST(SystemPerformanceMonitorTest, GetMemoryUsage_ConsistentResults) {
    const MemoryUsage memUsage1 = SystemPerformanceMonitor::GetMemoryUsage();
    const MemoryUsage memUsage2 = SystemPerformanceMonitor::GetMemoryUsage();
    
    // Total memory should remain constant
    EXPECT_EQ(memUsage1.total_memory, memUsage2.total_memory);
    
    // Available memory may vary slightly but should be close (within 100MB)
    const ULONGLONG tolerance = 100ULL * 1024 * 1024; // 100 MB
    const auto diff = memUsage1.available_memory > memUsage2.available_memory ?
                      memUsage1.available_memory - memUsage2.available_memory :
                      memUsage2.available_memory - memUsage1.available_memory;
    EXPECT_LT(diff, tolerance);
}

/**
 * @brief Test GetCpuUsage with default interval returns valid structure
 * @details Verifies that CPU usage retrieval with default 1-second interval works correctly
 */
TEST(SystemPerformanceMonitorTest, GetCpuUsage_DefaultInterval_ValidResult) {
    const CpuUsage cpuUsage = SystemPerformanceMonitor::GetCpuUsage();
    
    EXPECT_GE(cpuUsage.cpu_usage_percent, 0.0);
    EXPECT_LE(cpuUsage.cpu_usage_percent, 100.0);
}

/**
 * @brief Test GetCpuUsage with custom interval returns valid structure
 * @details Verifies that CPU usage retrieval with custom interval works correctly
 */
TEST(SystemPerformanceMonitorTest, GetCpuUsage_CustomInterval_ValidResult) {
    const CpuUsage cpuUsage = SystemPerformanceMonitor::GetCpuUsage(2);
    
    EXPECT_GE(cpuUsage.cpu_usage_percent, 0.0);
    EXPECT_LE(cpuUsage.cpu_usage_percent, 100.0);
}

/**
 * @brief Test GetCpuUsage with minimum interval
 * @details Verifies that CPU usage retrieval with 1-second interval works correctly
 */
TEST(SystemPerformanceMonitorTest, GetCpuUsage_MinimumInterval) {
    const CpuUsage cpuUsage = SystemPerformanceMonitor::GetCpuUsage(1);
    
    EXPECT_GE(cpuUsage.cpu_usage_percent, 0.0);
    EXPECT_LE(cpuUsage.cpu_usage_percent, 100.0);
}

/**
 * @brief Test GetCpuUsage percentage is in valid range
 * @details Ensures CPU usage percentage is always between 0.0 and 100.0
 */
TEST(SystemPerformanceMonitorTest, GetCpuUsage_PercentageInRange) {
    const CpuUsage cpuUsage = SystemPerformanceMonitor::GetCpuUsage(1);
    
    EXPECT_GE(cpuUsage.cpu_usage_percent, 0.0);
    EXPECT_LE(cpuUsage.cpu_usage_percent, 100.0);
}

/**
 * @brief Test GetCpuUsage execution time matches interval
 * @details Verifies that the method takes approximately the specified interval time
 */
TEST(SystemPerformanceMonitorTest, GetCpuUsage_ExecutionTimeMatchesInterval) {
    const int32_t interval = 1;
    
    const auto start = std::chrono::steady_clock::now();
    const CpuUsage cpuUsage = SystemPerformanceMonitor::GetCpuUsage(interval);
    const auto end = std::chrono::steady_clock::now();
    
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    const auto expectedDuration = interval * 1000; // Convert seconds to milliseconds
    
    // Allow 20% tolerance for timing variations
    const auto tolerance = expectedDuration * 0.2;
    EXPECT_NEAR(duration, expectedDuration, tolerance);
    
    // Suppress unused variable warning
    (void)cpuUsage;
}

/**
 * @brief Test GetCpuUsage with different intervals produces valid results
 * @details Verifies that various interval values work correctly
 */
TEST(SystemPerformanceMonitorTest, GetCpuUsage_DifferentIntervals) {
    const CpuUsage cpuUsage1 = SystemPerformanceMonitor::GetCpuUsage(1);
    const CpuUsage cpuUsage2 = SystemPerformanceMonitor::GetCpuUsage(2);
    
    // Both should return valid percentages
    EXPECT_GE(cpuUsage1.cpu_usage_percent, 0.0);
    EXPECT_LE(cpuUsage1.cpu_usage_percent, 100.0);
    EXPECT_GE(cpuUsage2.cpu_usage_percent, 0.0);
    EXPECT_LE(cpuUsage2.cpu_usage_percent, 100.0);
}

/**
 * @brief Test all performance monitor methods execute without crashing
 * @details Verifies that all public methods run without exceptions
 */
TEST(SystemPerformanceMonitorTest, AllMethods_ExecuteWithoutCrash) {
    EXPECT_NO_THROW(SystemPerformanceMonitor::GetMemoryUsage());
    EXPECT_NO_THROW(SystemPerformanceMonitor::GetCpuUsage(1));
}

/**
 * @brief Test MemoryUsage struct field accessibility
 * @details Verifies that all MemoryUsage fields can be accessed and modified
 */
TEST(SystemPerformanceMonitorTest, MemoryUsage_StructFieldsAccessible) {
    MemoryUsage memUsage{};
    
    memUsage.total_memory = 16000000000ULL;
    memUsage.available_memory = 8000000000ULL;
    memUsage.used_memory = 8000000000ULL;
    memUsage.memory_usage_percent = 50.0;
    
    EXPECT_EQ(memUsage.total_memory, 16000000000ULL);
    EXPECT_EQ(memUsage.available_memory, 8000000000ULL);
    EXPECT_EQ(memUsage.used_memory, 8000000000ULL);
    EXPECT_DOUBLE_EQ(memUsage.memory_usage_percent, 50.0);
}

/**
 * @brief Test CpuUsage struct field accessibility
 * @details Verifies that CpuUsage fields can be accessed and modified
 */
TEST(SystemPerformanceMonitorTest, CpuUsage_StructFieldsAccessible) {
    CpuUsage cpuUsage{};
    
    cpuUsage.cpu_usage_percent = 75.5;
    
    EXPECT_DOUBLE_EQ(cpuUsage.cpu_usage_percent, 75.5);
}

/**
 * @brief Test MemoryUsage default initialization
 * @details Verifies that MemoryUsage struct initializes to zero values
 */
TEST(SystemPerformanceMonitorTest, MemoryUsage_DefaultInitialization) {
    const MemoryUsage memUsage{};
    
    EXPECT_EQ(memUsage.total_memory, 0ULL);
    EXPECT_EQ(memUsage.available_memory, 0ULL);
    EXPECT_EQ(memUsage.used_memory, 0ULL);
    EXPECT_DOUBLE_EQ(memUsage.memory_usage_percent, 0.0);
}

/**
 * @brief Test CpuUsage default initialization
 * @details Verifies that CpuUsage struct initializes to zero values
 */
TEST(SystemPerformanceMonitorTest, CpuUsage_DefaultInitialization) {
    const CpuUsage cpuUsage{};
    
    EXPECT_DOUBLE_EQ(cpuUsage.cpu_usage_percent, 0.0);
}
