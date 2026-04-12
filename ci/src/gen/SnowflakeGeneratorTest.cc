/**
 * @file SnowflakeGeneratorTest.cc
 * @brief Unit tests for the SnowflakeGenerator class
 * @details Tests cover core Snowflake ID generation functionality including
 *          ID uniqueness, ordering, component validation, and error handling.
 */

#include <gtest/gtest.h>
#include "gen/SnowflakeGenerator.hpp"
#include <stdexcept>
#include <set>
#include <vector>
#include <thread>
#include <algorithm>

using namespace common::gen;

/**
 * @brief Test constructor with valid machine and datacenter IDs
 * @details Verifies that valid IDs (0-31) are accepted
 */
TEST(SnowflakeGeneratorTest, Constructor_ValidIds) {
    EXPECT_NO_THROW(SnowflakeGenerator generator(0, 0));
    EXPECT_NO_THROW(SnowflakeGenerator generator(31, 31));
    EXPECT_NO_THROW(SnowflakeGenerator generator(15, 20));
}

/**
 * @brief Test constructor with invalid machine ID throws exception
 * @details Verifies proper error handling for machine_id out of range
 */
TEST(SnowflakeGeneratorTest, Constructor_InvalidMachineId_ThrowsException) {
    EXPECT_THROW(SnowflakeGenerator generator(-1, 0), std::invalid_argument);
    EXPECT_THROW(SnowflakeGenerator generator(32, 0), std::invalid_argument);
    EXPECT_THROW(SnowflakeGenerator generator(100, 0), std::invalid_argument);
}

/**
 * @brief Test constructor with invalid datacenter ID throws exception
 * @details Verifies proper error handling for datacenter_id out of range
 */
TEST(SnowflakeGeneratorTest, Constructor_InvalidDatacenterId_ThrowsException) {
    EXPECT_THROW(SnowflakeGenerator generator(0, -1), std::invalid_argument);
    EXPECT_THROW(SnowflakeGenerator generator(0, 32), std::invalid_argument);
    EXPECT_THROW(SnowflakeGenerator generator(0, 100), std::invalid_argument);
}

/**
 * @brief Test NextId generates positive IDs
 * @details Verifies that generated IDs are positive values
 */
TEST(SnowflakeGeneratorTest, NextId_GeneratesPositiveIds) {
    SnowflakeGenerator generator(1, 1);

    for (int i = 0; i < 100; ++i) {
        const int64_t id = generator.NextId();
        EXPECT_GT(id, 0);
    }
}

/**
 * @brief Test NextId generates unique IDs
 * @details Verifies that all generated IDs are unique within a batch
 */
TEST(SnowflakeGeneratorTest, NextId_GeneratesUniqueIds) {
    SnowflakeGenerator generator(1, 1);
    std::set<int64_t> uniqueIds;

    for (int i = 0; i < 1000; ++i) {
        const int64_t id = generator.NextId();
        uniqueIds.insert(id);
    }

    // All IDs should be unique
    EXPECT_EQ(uniqueIds.size(), 1000);
}

/**
 * @brief Test NextId generates monotonically increasing IDs
 * @details Verifies that IDs are generated in ascending order
 */
TEST(SnowflakeGeneratorTest, NextId_MonotonicallyIncreasing) {
    SnowflakeGenerator generator(1, 1);

    int64_t previousId = 0;
    for (int i = 0; i < 100; ++i) {
        const int64_t currentId = generator.NextId();
        EXPECT_GT(currentId, previousId);
        previousId = currentId;
    }
}

/**
 * @brief Test NextId with different machine IDs produces different sequences
 * @details Verifies that different machine configurations produce different IDs
 */
TEST(SnowflakeGeneratorTest, NextId_DifferentMachineIds_ProduceDifferentIds) {
    SnowflakeGenerator generator1(1, 1);
    SnowflakeGenerator generator2(2, 1);

    const auto id1 = generator1.NextId();
    const auto id2 = generator2.NextId();

    // IDs should be different due to different machine IDs
    EXPECT_NE(id1, id2);
}

/**
 * @brief Test NextId with different datacenter IDs produces different sequences
 * @details Verifies that different datacenter configurations produce different IDs
 */
TEST(SnowflakeGeneratorTest, NextId_DifferentDatacenterIds_ProduceDifferentIds) {
    SnowflakeGenerator generator1(1, 1);
    SnowflakeGenerator generator2(1, 2);

    const auto id1 = generator1.NextId();
    const auto id2 = generator2.NextId();

    // IDs should be different due to different datacenter IDs
    EXPECT_NE(id1, id2);
}

/**
 * @brief Test rapid ID generation handles sequence overflow
 * @details Verifies that generating many IDs quickly handles sequence number correctly
 */
TEST(SnowflakeGeneratorTest, NextId_RapidGeneration_HandlesSequenceOverflow) {
    SnowflakeGenerator generator(1, 1);
    std::set<int64_t> uniqueIds;

    // Generate enough IDs to potentially overflow sequence (max_sequence = 4095)
    for (int i = 0; i < 5000; ++i) {
        const int64_t id = generator.NextId();
        uniqueIds.insert(id);
    }

    // All IDs should still be unique even with sequence overflow
    EXPECT_EQ(uniqueIds.size(), 5000);
}

/**
 * @brief Test thread safety with concurrent ID generation
 * @details Verifies that multiple threads can safely generate unique IDs
 */
TEST(SnowflakeGeneratorTest, NextId_ThreadSafety_ConcurrentGeneration) {
    SnowflakeGenerator generator(1, 1);
    std::set<int64_t> uniqueIds;
    std::mutex idsMutex;
    constexpr int threadCount = 8;
    constexpr int idsPerThread = 100;

    std::vector<std::thread> threads;
    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([&generator, &uniqueIds, &idsMutex]() {
            for (int i = 0; i < idsPerThread; ++i) {
                const int64_t id = generator.NextId();
                std::lock_guard lock(idsMutex);
                uniqueIds.insert(id);
            }
        });
    }

    for (auto &thread: threads) {
        thread.join();
    }

    // All IDs from all threads should be unique
    const int expectedTotal = threadCount * idsPerThread;
    EXPECT_EQ(uniqueIds.size(), expectedTotal);
}

/**
 * @brief Test thread safety maintains monotonic ordering
 * @details Verifies that IDs maintain ordering even with concurrent access
 */
TEST(SnowflakeGeneratorTest, NextId_ThreadSafety_MaintainsOrdering) {
    SnowflakeGenerator generator(1, 1);
    std::vector<int64_t> generatedIds;
    std::mutex idsMutex;
    constexpr int threadCount = 4;
    constexpr int idsPerThread = 50;

    std::vector<std::thread> threads;
    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([&generator, &generatedIds, &idsMutex]() {
            for (int i = 0; i < idsPerThread; ++i) {
                const int64_t id = generator.NextId();
                std::lock_guard lock(idsMutex);
                generatedIds.push_back(id);
            }
        });
    }

    for (auto &thread: threads) {
        thread.join();
    }

    // Sort and verify all IDs are unique
    std::sort(generatedIds.begin(), generatedIds.end());
    const auto last = std::unique(generatedIds.begin(), generatedIds.end());
    const int uniqueCount = static_cast<int>(last - generatedIds.begin());

    EXPECT_EQ(uniqueCount, threadCount * idsPerThread);
}

/**
 * @brief Test boundary machine ID values
 * @details Verifies that boundary values (0 and 31) work correctly
 */
TEST(SnowflakeGeneratorTest, Constructor_BoundaryMachineIds) {
    EXPECT_NO_THROW(SnowflakeGenerator generator(0, 0));
    EXPECT_NO_THROW(SnowflakeGenerator generator(31, 0));

    SnowflakeGenerator genMin(0, 0);
    SnowflakeGenerator genMax(31, 0);

    const auto id1 = genMin.NextId();
    const auto id2 = genMax.NextId();

    EXPECT_GT(id1, 0);
    EXPECT_GT(id2, 0);
    EXPECT_NE(id1, id2);
}

/**
 * @brief Test boundary datacenter ID values
 * @details Verifies that boundary values (0 and 31) work correctly
 */
TEST(SnowflakeGeneratorTest, Constructor_BoundaryDatacenterIds) {
    EXPECT_NO_THROW(SnowflakeGenerator generator(0, 0));
    EXPECT_NO_THROW(SnowflakeGenerator generator(0, 31));

    SnowflakeGenerator genMin(0, 0);
    SnowflakeGenerator genMax(0, 31);

    const auto id1 = genMin.NextId();
    const auto id2 = genMax.NextId();

    EXPECT_GT(id1, 0);
    EXPECT_GT(id2, 0);
    EXPECT_NE(id1, id2);
}

/**
 * @brief Test ID generation consistency across multiple calls
 * @details Verifies that the generator produces consistent, valid IDs over time
 */
TEST(SnowflakeGeneratorTest, NextId_Consistency_MultipleCalls) {
    SnowflakeGenerator generator(5, 10);

    std::vector<int64_t> ids;
    for (int i = 0; i < 100; ++i) {
        ids.push_back(generator.NextId());
    }

    // Verify all IDs are positive
    for (const auto id: ids) {
        EXPECT_GT(id, 0);
    }

    // Verify all IDs are unique
    const std::set<int64_t> uniqueIds(ids.begin(), ids.end());
    EXPECT_EQ(uniqueIds.size(), ids.size());

    // Verify IDs are in ascending order
    for (size_t i = 1; i < ids.size(); ++i) {
        EXPECT_GT(ids[i], ids[i - 1]);
    }
}

/**
 * @brief Test SnowflakeOption constants are correctly defined
 * @details Verifies that configuration constants have expected values
 */
TEST(SnowflakeGeneratorTest, SnowflakeOption_ConstantsCorrect) {
    EXPECT_EQ(SnowflakeOption::machine_bits_, 10);
    EXPECT_EQ(SnowflakeOption::sequence_bits_, 12);
    EXPECT_EQ(SnowflakeOption::max_sequence_, 4095); // 2^12 - 1
    EXPECT_EQ(SnowflakeOption::max_machine_id_, 31); // 2^5 - 1
    EXPECT_EQ(SnowflakeOption::max_datacenter_id_, 31); // 2^5 - 1
}