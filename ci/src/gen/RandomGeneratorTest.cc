/**
 * @file RandomGeneratorTest.cc
 * @brief Unit tests for the RandomGenerator class
 * @details Tests cover core random number generation functionality including
 *          integer, boolean, double, string, and Gaussian distribution generation,
 *          as well as seed management and error handling.
 */

#include <gtest/gtest.h>
#include "gen/RandomGenerator.hpp"
#include <stdexcept>
#include <set>
#include <algorithm>
#include <cmath>

using namespace common::gen;

/**
 * @brief Test default constructor creates valid generator
 * @details Verifies that default constructor initializes a working random generator
 */
TEST(RandomGeneratorTest, DefaultConstructor_ValidGenerator) {
    EXPECT_NO_THROW(RandomGenerator generator);
}

/**
 * @brief Test constructor with specified seed creates valid generator
 * @details Verifies that seeded constructor initializes a working random generator
 */
TEST(RandomGeneratorTest, SeededConstructor_ValidGenerator) {
    EXPECT_NO_THROW(RandomGenerator generator(42));
}

/**
 * @brief Test nextInt generates values within range
 * @details Verifies that generated integers are within [min, max] bounds
 */
TEST(RandomGeneratorTest, NextInt_WithinRange) {
    RandomGenerator generator(12345);
    
    for (int i = 0; i < 100; ++i) {
        const int value = generator.nextInt(1, 100);
        EXPECT_GE(value, 1);
        EXPECT_LE(value, 100);
    }
}

/**
 * @brief Test nextInt with negative range
 * @details Verifies that negative ranges work correctly
 */
TEST(RandomGeneratorTest, NextInt_NegativeRange) {
    RandomGenerator generator(12345);
    
    for (int i = 0; i < 100; ++i) {
        const int value = generator.nextInt(-50, -10);
        EXPECT_GE(value, -50);
        EXPECT_LE(value, -10);
    }
}

/**
 * @brief Test nextInt with min equals max
 * @details Verifies that when min == max, the same value is always returned
 */
TEST(RandomGeneratorTest, NextInt_MinEqualsMax) {
    RandomGenerator generator(12345);
    
    for (int i = 0; i < 10; ++i) {
        const int value = generator.nextInt(42, 42);
        EXPECT_EQ(value, 42);
    }
}

/**
 * @brief Test nextInt with invalid range throws exception
 * @details Verifies proper error handling when min > max
 */
TEST(RandomGeneratorTest, NextInt_InvalidRange_ThrowsException) {
    RandomGenerator generator(12345);
    EXPECT_THROW(generator.nextInt(100, 1), std::invalid_argument);
}

/**
 * @brief Test nextBool with default probability returns bool
 * @details Verifies that nextBool() returns valid boolean values
 */
TEST(RandomGeneratorTest, NextBool_Default_ReturnsBool) {
    RandomGenerator generator(12345);
    
    for (int i = 0; i < 100; ++i) {
        const bool value = generator.nextBool();
        EXPECT_TRUE(value == true || value == false);
    }
}

/**
 * @brief Test nextBool with probability 0.0 always returns false
 * @details Verifies that zero probability produces only false values
 */
TEST(RandomGeneratorTest, NextBool_ZeroProbability_AlwaysFalse) {
    RandomGenerator generator(12345);
    
    for (int i = 0; i < 100; ++i) {
        const bool value = generator.nextBool(0.0);
        EXPECT_FALSE(value);
    }
}

/**
 * @brief Test nextBool with probability 1.0 always returns true
 * @details Verifies that full probability produces only true values
 */
TEST(RandomGeneratorTest, NextBool_FullProbability_AlwaysTrue) {
    RandomGenerator generator(12345);
    
    for (int i = 0; i < 100; ++i) {
        const bool value = generator.nextBool(1.0);
        EXPECT_TRUE(value);
    }
}

/**
 * @brief Test nextBool with probability 0.5 produces mixed results
 * @details Verifies that 50% probability produces both true and false
 */
TEST(RandomGeneratorTest, NextBool_HalfProbability_MixedResults) {
    RandomGenerator generator(12345);
    
    int trueCount = 0;
    int falseCount = 0;
    
    for (int i = 0; i < 1000; ++i) {
        if (generator.nextBool(0.5)) {
            ++trueCount;
        } else {
            ++falseCount;
        }
    }
    
    // Both should have occurred (statistically very likely with 1000 samples)
    EXPECT_GT(trueCount, 0);
    EXPECT_GT(falseCount, 0);
}

/**
 * @brief Test nextBool with invalid probability throws exception
 * @details Verifies proper error handling for probabilities outside [0, 1]
 */
TEST(RandomGeneratorTest, NextBool_InvalidProbability_ThrowsException) {
    RandomGenerator generator(12345);
    EXPECT_THROW(generator.nextBool(-0.1), std::invalid_argument);
    EXPECT_THROW(generator.nextBool(1.1), std::invalid_argument);
}

/**
 * @brief Test nextDouble generates values within range
 * @details Verifies that generated doubles are within [min, max) bounds
 */
TEST(RandomGeneratorTest, NextDouble_WithinRange) {
    RandomGenerator generator(12345);
    
    for (int i = 0; i < 100; ++i) {
        const double value = generator.nextDouble(0.0, 1.0);
        EXPECT_GE(value, 0.0);
        EXPECT_LT(value, 1.0);
    }
}

/**
 * @brief Test nextDouble with custom range
 * @details Verifies that custom ranges work correctly
 */
TEST(RandomGeneratorTest, NextDouble_CustomRange) {
    RandomGenerator generator(12345);
    
    for (int i = 0; i < 100; ++i) {
        const double value = generator.nextDouble(-10.5, 20.3);
        EXPECT_GE(value, -10.5);
        EXPECT_LT(value, 20.3);
    }
}

/**
 * @brief Test nextDouble with invalid range throws exception
 * @details Verifies proper error handling when min >= max
 */
TEST(RandomGeneratorTest, NextDouble_InvalidRange_ThrowsException) {
    RandomGenerator generator(12345);
    EXPECT_THROW(generator.nextDouble(10.0, 5.0), std::invalid_argument);
    EXPECT_THROW(generator.nextDouble(5.0, 5.0), std::invalid_argument);
}

/**
 * @brief Test nextString generates correct length
 * @details Verifies that generated strings have the specified length
 */
TEST(RandomGeneratorTest, NextString_CorrectLength) {
    RandomGenerator generator(12345);
    
    const auto str1 = generator.nextString(10);
    EXPECT_EQ(str1.length(), 10);
    
    const auto str2 = generator.nextString(100);
    EXPECT_EQ(str2.length(), 100);
    
    const auto str3 = generator.nextString(0);
    EXPECT_EQ(str3.length(), 0);
}

/**
 * @brief Test nextString uses only characters from charset
 * @details Verifies that all characters in result are from the specified charset
 */
TEST(RandomGeneratorTest, NextString_ValidCharset) {
    RandomGenerator generator(12345);
    const std::string charset = "ABC123";
    
    const auto result = generator.nextString(1000, charset);
    
    for (const char c : result) {
        EXPECT_NE(charset.find(c), std::string::npos);
    }
}

/**
 * @brief Test nextString with custom charset
 * @details Verifies that custom character sets work correctly
 */
TEST(RandomGeneratorTest, NextString_CustomCharset) {
    RandomGenerator generator(12345);
    const std::string charset = "XYZ";
    
    const auto result = generator.nextString(50, charset);
    
    EXPECT_EQ(result.length(), 50);
    for (const char c : result) {
        EXPECT_TRUE(c == 'X' || c == 'Y' || c == 'Z');
    }
}

/**
 * @brief Test nextString with empty charset throws exception
 * @details Verifies proper error handling for empty charset
 */
TEST(RandomGeneratorTest, NextString_EmptyCharset_ThrowsException) {
    RandomGenerator generator(12345);
    EXPECT_THROW(generator.nextString(10, ""), std::invalid_argument);
}

/**
 * @brief Test nextString generates different strings with different seeds
 * @details Verifies that different seeds produce different random strings
 */
TEST(RandomGeneratorTest, NextString_DifferentSeeds_ProduceDifferentStrings) {
    RandomGenerator generator1(12345);
    RandomGenerator generator2(67890);
    
    const auto str1 = generator1.nextString(20);
    const auto str2 = generator2.nextString(20);
    
    EXPECT_NE(str1, str2);
}

/**
 * @brief Test nextGaussian generates values around mean
 * @details Verifies that Gaussian distribution centers around the specified mean
 */
TEST(RandomGeneratorTest, NextGaussian_AroundMean) {
    RandomGenerator generator(12345);
    const double mean = 100.0;
    const double stddev = 10.0;
    
    double sum = 0.0;
    const int sampleCount = 10000;
    
    for (int i = 0; i < sampleCount; ++i) {
        sum += generator.nextGaussian(mean, stddev);
    }
    
    const double actualMean = sum / sampleCount;
    
    // Sample mean should be close to specified mean (within 3*stddev/sqrt(n))
    const double tolerance = 3.0 * stddev / std::sqrt(sampleCount);
    EXPECT_NEAR(actualMean, mean, tolerance);
}

/**
 * @brief Test nextGaussian with standard deviation of 1.0
 * @details Verifies that standard normal distribution works correctly
 */
TEST(RandomGeneratorTest, NextGaussian_StandardNormal) {
    RandomGenerator generator(12345);
    
    // Should not throw and return reasonable values
    for (int i = 0; i < 100; ++i) {
        const double value = generator.nextGaussian(0.0, 1.0);
        // Most values should be within [-4, 4] for standard normal
        EXPECT_GE(value, -6.0);
        EXPECT_LE(value, 6.0);
    }
}

/**
 * @brief Test nextGaussian with invalid stddev throws exception
 * @details Verifies proper error handling for non-positive standard deviation
 */
TEST(RandomGeneratorTest, NextGaussian_InvalidStddev_ThrowsException) {
    RandomGenerator generator(12345);
    EXPECT_THROW(generator.nextGaussian(0.0, 0.0), std::invalid_argument);
    EXPECT_THROW(generator.nextGaussian(0.0, -1.0), std::invalid_argument);
}

/**
 * @brief Test setSeed produces reproducible results
 * @details Verifies that setting the same seed produces the same sequence
 */
TEST(RandomGeneratorTest, SetSeed_ReproducibleResults) {
    RandomGenerator generator1(42);
    const int val1 = generator1.nextInt(1, 1000);
    const double val2 = generator1.nextDouble(0.0, 1.0);
    const bool val3 = generator1.nextBool();
    
    RandomGenerator generator2(42);
    const int val1_copy = generator2.nextInt(1, 1000);
    const double val2_copy = generator2.nextDouble(0.0, 1.0);
    const bool val3_copy = generator2.nextBool();
    
    EXPECT_EQ(val1, val1_copy);
    EXPECT_EQ(val2, val2_copy);
    EXPECT_EQ(val3, val3_copy);
}

/**
 * @brief Test setSeed changes the sequence
 * @details Verifies that changing seed produces different sequence
 */
TEST(RandomGeneratorTest, SetSeed_ChangesSequence) {
    RandomGenerator generator(42);
    const int val1 = generator.nextInt(1, 1000);
    
    generator.setSeed(99);
    const int val2 = generator.nextInt(1, 1000);
    
    // Very unlikely to be the same with different seeds
    EXPECT_NE(val1, val2);
}

/**
 * @brief Test multiple calls generate variety
 * @details Verifies that consecutive calls produce varied results
 */
TEST(RandomGeneratorTest, MultipleCalls_GenerateVariety) {
    RandomGenerator generator(12345);
    std::set<int> uniqueValues;
    
    for (int i = 0; i < 100; ++i) {
        uniqueValues.insert(generator.nextInt(1, 1000));
    }
    
    // Should generate many unique values (statistically almost certain)
    EXPECT_GT(uniqueValues.size(), 50);
}

/**
 * @brief Test thread safety with concurrent access
 * @details Verifies that the generator can be safely used from multiple threads
 * @note This is a basic test; comprehensive thread safety testing requires more sophisticated tools
 */
TEST(RandomGeneratorTest, ThreadSafety_BasicTest) {
    RandomGenerator generator(12345);
    std::set<int> uniqueValues;
    std::mutex testMutex;
    
    // Launch multiple threads to generate random numbers
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&generator, &uniqueValues, &testMutex]() {
            for (int i = 0; i < 50; ++i) {
                const int value = generator.nextInt(1, 10000);
                std::lock_guard lock(testMutex);
                uniqueValues.insert(value);
            }
        });
    }
    
    for (auto &thread : threads) {
        thread.join();
    }
    
    // Should have generated many unique values without crashes
    EXPECT_GT(uniqueValues.size(), 100);
}
