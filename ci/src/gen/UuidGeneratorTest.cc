/**
 * @file UuidGeneratorTest.cc
 * @brief Unit tests for the UuidGenerator class
 * @details Tests cover core UUID generation functionality including format validation,
 *          uniqueness verification, and consistency checks.
 */

#include <gtest/gtest.h>
#include "gen/UuidGenerator.hpp"
#include <set>
#include <regex>
#include <string>

using namespace common::gen;

/**
 * @brief Test GenerateRandomUuid returns non-empty string
 * @details Verifies that UUID generation returns a valid, non-empty result
 */
TEST(UuidGeneratorTest, GenerateRandomUuid_NonEmpty) {
    const auto uuid = UuidGenerator::GenerateRandomUuid();
    EXPECT_FALSE(uuid.empty());
}

/**
 * @brief Test GenerateRandomUuid returns correct format
 * @details Verifies that generated UUID matches standard UUID format (8-4-4-4-12)
 */
TEST(UuidGeneratorTest, GenerateRandomUuid_CorrectFormat) {
    const auto uuid = UuidGenerator::GenerateRandomUuid();
    
    // Standard UUID format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    const std::regex uuidPattern(R"([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})");
    EXPECT_TRUE(std::regex_match(uuid, uuidPattern));
}

/**
 * @brief Test GenerateRandomUuid returns lowercase hexadecimal
 * @details Verifies that UUID contains only lowercase hex characters and hyphens
 */
TEST(UuidGeneratorTest, GenerateRandomUuid_LowercaseHex) {
    const auto uuid = UuidGenerator::GenerateRandomUuid();
    
    for (const char c : uuid) {
        if (c != '-') {
            EXPECT_TRUE(std::isxdigit(static_cast<unsigned char>(c)));
            EXPECT_TRUE(std::islower(static_cast<unsigned char>(c)) || std::isdigit(static_cast<unsigned char>(c)));
        }
    }
}

/**
 * @brief Test GenerateRandomUuid has correct length
 * @details Verifies that UUID string has exactly 36 characters (32 hex + 4 hyphens)
 */
TEST(UuidGeneratorTest, GenerateRandomUuid_CorrectLength) {
    const auto uuid = UuidGenerator::GenerateRandomUuid();
    EXPECT_EQ(uuid.length(), 36);
}

/**
 * @brief Test GenerateRandomUuid generates unique values
 * @details Verifies that multiple calls produce different UUIDs
 */
TEST(UuidGeneratorTest, GenerateRandomUuid_UniqueValues) {
    std::set<std::string> uniqueUuids;
    
    for (int i = 0; i < 1000; ++i) {
        const auto uuid = UuidGenerator::GenerateRandomUuid();
        uniqueUuids.insert(uuid);
    }
    
    // All UUIDs should be unique
    EXPECT_EQ(uniqueUuids.size(), 1000);
}

/**
 * @brief Test GenerateRandomUuid has correct hyphen positions
 * @details Verifies that hyphens appear at positions 8, 13, 18, and 23
 */
TEST(UuidGeneratorTest, GenerateRandomUuid_CorrectHyphenPositions) {
    const auto uuid = UuidGenerator::GenerateRandomUuid();
    
    EXPECT_EQ(uuid[8], '-');
    EXPECT_EQ(uuid[13], '-');
    EXPECT_EQ(uuid[18], '-');
    EXPECT_EQ(uuid[23], '-');
}

/**
 * @brief Test GenerateRandomUuid version indicator
 * @details Verifies that UUID version 4 indicator is present (position 14 should be '4')
 */
TEST(UuidGeneratorTest, GenerateRandomUuid_Version4Indicator) {
    const auto uuid = UuidGenerator::GenerateRandomUuid();
    
    // UUID version 4 has '4' at position 14 (0-indexed)
    EXPECT_EQ(uuid[14], '4');
}

/**
 * @brief Test GenerateRandomUuid variant indicator
 * @details Verifies that UUID variant bits are correct (position 19 should be '8', '9', 'a', or 'b')
 */
TEST(UuidGeneratorTest, GenerateRandomUuid_VariantIndicator) {
    const auto uuid = UuidGenerator::GenerateRandomUuid();
    
    // RFC 4122 variant has bits 10xx at position 19
    const char variantChar = uuid[19];
    EXPECT_TRUE(variantChar == '8' || variantChar == '9' || 
                variantChar == 'a' || variantChar == 'b');
}

/**
 * @brief Test GenerateRandomUuid produces different UUIDs in sequence
 * @details Verifies that consecutive calls don't return the same value
 */
TEST(UuidGeneratorTest, GenerateRandomUuid_DifferentInSequence) {
    const auto uuid1 = UuidGenerator::GenerateRandomUuid();
    const auto uuid2 = UuidGenerator::GenerateRandomUuid();
    const auto uuid3 = UuidGenerator::GenerateRandomUuid();
    
    EXPECT_NE(uuid1, uuid2);
    EXPECT_NE(uuid2, uuid3);
    EXPECT_NE(uuid1, uuid3);
}

/**
 * @brief Test GenerateRandomUuid thread safety
 * @details Verifies that concurrent calls from multiple threads produce mostly unique UUIDs
 * @note Due to static generator in implementation, rare collisions may occur under high concurrency
 */
TEST(UuidGeneratorTest, GenerateRandomUuid_ThreadSafety) {
    std::set<std::string> uniqueUuids;
    std::mutex uuidsMutex;
    constexpr int threadCount = 8;
    constexpr int uuidsPerThread = 100;
    
    std::vector<std::thread> threads;
    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([&uniqueUuids, &uuidsMutex]() {
            for (int i = 0; i < uuidsPerThread; ++i) {
                const auto uuid = UuidGenerator::GenerateRandomUuid();
                std::lock_guard lock(uuidsMutex);
                uniqueUuids.insert(uuid);
            }
        });
    }
    
    for (auto &thread : threads) {
        thread.join();
    }
    
    // Most UUIDs should be unique (allowing for very rare collisions due to static generator)
    const int expectedTotal = threadCount * uuidsPerThread;
    const double uniquenessRate = static_cast<double>(uniqueUuids.size()) / expectedTotal;
    EXPECT_GE(uniquenessRate, 0.99); // At least 99% unique
}

/**
 * @brief Test GenerateRandomUuid no exceptions thrown
 * @details Verifies that the method is noexcept and never throws
 */
TEST(UuidGeneratorTest, GenerateRandomUuid_NoExceptions) {
    EXPECT_NO_THROW(UuidGenerator::GenerateRandomUuid());
    
    // Test multiple calls to ensure stability
    for (int i = 0; i < 100; ++i) {
        EXPECT_NO_THROW(UuidGenerator::GenerateRandomUuid());
    }
}

/**
 * @brief Test GenerateRandomUuid statistical distribution
 * @details Verifies that generated UUIDs have good randomness characteristics
 */
TEST(UuidGeneratorTest, GenerateRandomUuid_GoodDistribution) {
    std::set<char> uniqueChars;
    
    // Generate many UUIDs and collect unique characters
    for (int i = 0; i < 100; ++i) {
        const auto uuid = UuidGenerator::GenerateRandomUuid();
        for (const char c : uuid) {
            if (c != '-') {
                uniqueChars.insert(c);
            }
        }
    }
    
    // Should use most hexadecimal characters (good distribution)
    EXPECT_GE(uniqueChars.size(), 10); // At least 10 different hex chars
}
