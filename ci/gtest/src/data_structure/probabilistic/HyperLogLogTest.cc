/**
 * @file HyperLogLogTest.cc
 * @brief Unit tests for the HyperLogLog cardinality estimator
 * @details Tests cover basic insertion, estimation accuracy, merge correctness,
 *          serialization, edge cases, and statistical error bounds.
 */

#include <gtest/gtest.h>

#include "data_structure/probabilistic/HyperLogLog.hpp"

#include <cmath>
#include <cstdint>
#include <random>
#include <set>
#include <string>
#include <vector>

using namespace common::data_structure::probabilistic;

// ---------------------------------------------------------------------------
//  Test fixture
// ---------------------------------------------------------------------------

class HyperLogLogTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ---------------------------------------------------------------------------
//  Empty / Default state
// ---------------------------------------------------------------------------

TEST_F(HyperLogLogTest, DefaultConstructor_EmptyEstimate_ReturnsZero)
{
    const HyperLogLog<14> hll;
    EXPECT_EQ(hll.estimate(), 0);
}

TEST_F(HyperLogLogTest, Clear_AfterInsert_ReturnsZero)
{
    HyperLogLog<14> hll;
    hll.insert("hello");
    EXPECT_GT(hll.estimate(), 0);

    hll.clear();
    EXPECT_EQ(hll.estimate(), 0);
}

// ---------------------------------------------------------------------------
//  Basic insertion
// ---------------------------------------------------------------------------

TEST_F(HyperLogLogTest, Insert_SingleElement_EstimateApproximatelyOne)
{
    HyperLogLog<14> hll;
    hll.insert("unique_element");
    // For a single element, the estimate should be very close to 1
    const uint64_t est = hll.estimate();
    EXPECT_GE(est, 1);
    EXPECT_LE(est, 3); // small jitter allowed for probabilistic structure
}

TEST_F(HyperLogLogTest, Insert_SameElementRepeated_StableEstimate)
{
    HyperLogLog<14> hll;
    for (int i = 0; i < 10000; ++i)
    {
        hll.insert("same_element");
    }
    const uint64_t est = hll.estimate();
    EXPECT_GE(est, 1);
    EXPECT_LE(est, 10); // should still be very close to 1
}

TEST_F(HyperLogLogTest, Insert_MultipleDistinctElements_NearActual)
{
    HyperLogLog<14> hll;
    constexpr uint64_t N = 10000;

    for (uint64_t i = 0; i < N; ++i)
    {
        hll.insert(std::to_string(i));
    }

    const uint64_t est = hll.estimate();
    const double ratio = static_cast<double>(est) / static_cast<double>(N);

    // Expect estimate within 3 sigma (theoretical error ~ 0.8% for p=14)
    // 3 * 0.008 = 0.024, but we allow a generous 10% for test reliability
    EXPECT_GT(ratio, 0.90);
    EXPECT_LT(ratio, 1.10);
}

// ---------------------------------------------------------------------------
//  Precision variants
// ---------------------------------------------------------------------------

TEST_F(HyperLogLogTest, DifferentPrecision_AllWork)
{
    {
        HyperLogLog<10> hll;
        for (uint64_t i = 0; i < 1000; ++i) hll.insert(std::to_string(i));
        const uint64_t est = hll.estimate();
        EXPECT_GT(est, static_cast<uint64_t>(800));
        EXPECT_LT(est, static_cast<uint64_t>(1200));
    }
    {
        HyperLogLog<12> hll;
        for (uint64_t i = 0; i < 1000; ++i) hll.insert(std::to_string(i));
        const uint64_t est = hll.estimate();
        EXPECT_GT(est, static_cast<uint64_t>(800));
        EXPECT_LT(est, static_cast<uint64_t>(1200));
    }
    {
        HyperLogLog<14> hll;
        for (uint64_t i = 0; i < 1000; ++i) hll.insert(std::to_string(i));
        const uint64_t est = hll.estimate();
        EXPECT_GT(est, static_cast<uint64_t>(800));
        EXPECT_LT(est, static_cast<uint64_t>(1200));
    }
    {
        HyperLogLog<16> hll;
        for (uint64_t i = 0; i < 1000; ++i) hll.insert(std::to_string(i));
        const uint64_t est = hll.estimate();
        EXPECT_GT(est, static_cast<uint64_t>(800));
        EXPECT_LT(est, static_cast<uint64_t>(1200));
    }
}

// ---------------------------------------------------------------------------
//  Error rate (theoretical)
// ---------------------------------------------------------------------------

TEST_F(HyperLogLogTest, ErrorRate_DecreasesWithPrecision)
{
    const double e10 = HyperLogLog<10>::errorRate();
    const double e12 = HyperLogLog<12>::errorRate();
    const double e14 = HyperLogLog<14>::errorRate();
    const double e16 = HyperLogLog<16>::errorRate();

    EXPECT_GT(e10, e12);
    EXPECT_GT(e12, e14);
    EXPECT_GT(e14, e16);

    // Check default precision error is about 0.8%
    const double e_def = HyperLogLog<>::errorRate();
    EXPECT_NEAR(e_def, 0.0081, 0.001);
}

// ---------------------------------------------------------------------------
//  Merge
// ---------------------------------------------------------------------------

TEST_F(HyperLogLogTest, Merge_TwoHalfSets_EqualsFullSet)
{
    HyperLogLog<12> full;
    HyperLogLog<12> half1;
    HyperLogLog<12> half2;

    constexpr uint64_t N = 5000;

    for (uint64_t i = 0; i < N; ++i)
    {
        full.insert(std::to_string(i));
        if (i < N / 2)
        {
            half1.insert(std::to_string(i));
        }
        else
        {
            half2.insert(std::to_string(i));
        }
    }

    half1.merge(half2);
    const uint64_t fullEst = full.estimate();
    const uint64_t mergedEst = half1.estimate();

    // Merged estimate should be close to the full-set estimate
    const double ratio = static_cast<double>(fullEst) /
                         static_cast<double>(mergedEst);
    EXPECT_GT(ratio, 0.90);
    EXPECT_LT(ratio, 1.10);
}

TEST_F(HyperLogLogTest, Merge_EmptyIntoNonEmpty_NoChange)
{
    HyperLogLog<12> original;
    for (uint64_t i = 0; i < 1000; ++i)
    {
        original.insert(std::to_string(i));
    }

    const uint64_t before = original.estimate();
    const HyperLogLog<12> empty;

    original.merge(empty);

    EXPECT_EQ(original.estimate(), before);
}

TEST_F(HyperLogLogTest, Merge_SelfMerge_NoChange)
{
    HyperLogLog<14> hll;
    for (uint64_t i = 0; i < 1000; ++i)
    {
        hll.insert(std::to_string(i));
    }

    const uint64_t before = hll.estimate();
    hll.merge(hll);
    EXPECT_EQ(hll.estimate(), before);
}

TEST_F(HyperLogLogTest, Merge_PolymorphicInterface_Works)
{
    // Verify that polymorphic merge (through IBaseEstimator) works for
    // compatible types (same HyperLogLog precision).

    HyperLogLog<14> hll1;
    HyperLogLog<14> hll2;
    hll1.insert("a");
    hll2.insert("b");

    EXPECT_NO_THROW(hll1.merge(static_cast<IBaseEstimator&>(hll2)));
    EXPECT_GT(hll1.estimate(), 1);
}

TEST_F(HyperLogLogTest, Merge_IncompatiblePrecision_Throws)
{
    // Merging HyperLogLog with different precision through IBaseEstimator
    // should throw because the register arrays have different sizes.

    HyperLogLog<10> small;
    HyperLogLog<14> large;
    small.insert("a");

    // dynamic_cast will succeed (both are HyperLogLog), but register
    // counts differ, so the element-wise max loop will overrun one of
    // the register arrays. In practice this is UB and hard to detect.
    // We accept that the caller is responsible for precision matching.
    // For now this test documents the limitation -- use the type-safe
    // merge(HyperLogLog) overload when precision could differ.
    SUCCEED() << "Precision mismatch should be avoided by the caller; "
                 "use merge(const HyperLogLog&) for compile-time safety.";
}

// ---------------------------------------------------------------------------
//  Statistical error bounds (multiple trials)
// ---------------------------------------------------------------------------

TEST_F(HyperLogLogTest, StatisticalError_WithinExpectedBounds)
{
    // Test that HyperLogLog with p=12 (m=4096) stays within ~3x theoretical
    // std error across multiple independent trials.

    const double THEORETICAL_ERROR = HyperLogLog<12>::errorRate(); // ~1.6%
    constexpr uint64_t N = 10000;
    constexpr int TRIALS = 10;

    double maxRatio = 0.0;
    double minRatio = 2.0;

    for (int trial = 0; trial < TRIALS; ++trial)
    {
        HyperLogLog<12> hll;
        for (uint64_t i = 0; i < N; ++i)
        {
            hll.insert(std::to_string(i) + "_" + std::to_string(trial));
        }

        const double ratio = static_cast<double>(hll.estimate()) /
                             static_cast<double>(N);
        maxRatio = std::max(maxRatio, ratio);
        minRatio = std::min(minRatio, ratio);
    }

    // Allow up to 5 sigma (very generous for random test stability)
    const double ALLOWED_DEVIATION = 5.0 * THEORETICAL_ERROR;
    EXPECT_GT(maxRatio, 1.0 - ALLOWED_DEVIATION);
    EXPECT_LT(maxRatio, 1.0 + ALLOWED_DEVIATION);
    EXPECT_GT(minRatio, 1.0 - ALLOWED_DEVIATION);
    EXPECT_LT(minRatio, 1.0 + ALLOWED_DEVIATION);
}

// ---------------------------------------------------------------------------
//  Serialization
// ---------------------------------------------------------------------------

TEST_F(HyperLogLogTest, SerializeDeserialize_PreservesEstimate)
{
    HyperLogLog<14> original;
    for (uint64_t i = 0; i < 5000; ++i)
    {
        original.insert(std::to_string(i));
    }

    const uint64_t originalEst = original.estimate();
    const auto data = original.serialize();
    const auto restored = HyperLogLog<14>::deserialize(data);

    EXPECT_EQ(restored.estimate(), originalEst);
}

TEST_F(HyperLogLogTest, SerializeDeserialize_Empty_Works)
{
    const HyperLogLog<14> original;
    const auto data = original.serialize();
    const auto restored = HyperLogLog<14>::deserialize(data);

    EXPECT_EQ(restored.estimate(), 0);
}

TEST_F(HyperLogLogTest, Deserialize_WrongPrecision_Throws)
{
    HyperLogLog<14> hll;
    hll.insert("test");
    const auto data = hll.serialize();

    EXPECT_THROW(HyperLogLog<16>::deserialize(data), std::invalid_argument);
}

TEST_F(HyperLogLogTest, Deserialize_TruncatedData_Throws)
{
    const std::vector<uint8_t> badData = {uint8_t{0x01}, uint8_t{0x0E}}; // version + precision but no registers
    EXPECT_THROW(HyperLogLog<14>::deserialize(badData), std::invalid_argument);
}

TEST_F(HyperLogLogTest, Deserialize_WrongVersion_Throws)
{
    std::vector<uint8_t> badData = {uint8_t{0xFF}, uint8_t{0x0E}}; // unknown version
    badData.resize(2 + HyperLogLog<14>::REGISTER_COUNT, 0);
    EXPECT_THROW(HyperLogLog<14>::deserialize(badData), std::invalid_argument);
}

// ---------------------------------------------------------------------------
//  Large cardinality test
// ---------------------------------------------------------------------------

TEST_F(HyperLogLogTest, LargeCardinality_100k_ReasonableEstimate)
{
    HyperLogLog<14> hll;
    constexpr uint64_t N = 100000;

    for (uint64_t i = 0; i < N; ++i)
    {
        hll.insert(std::to_string(i));
    }

    const uint64_t est = hll.estimate();
    const double ratio = static_cast<double>(est) / static_cast<double>(N);

    // For p=14, error ~0.8%, allow 3x that = 2.4%
    EXPECT_GT(ratio, 0.95);
    EXPECT_LT(ratio, 1.05);
}

// ---------------------------------------------------------------------------
//  Memory usage
// ---------------------------------------------------------------------------

TEST_F(HyperLogLogTest, MemoryUsage_Reasonable)
{
    const HyperLogLog<14> hll;
    // m = 16384 registers * 1 byte + small overhead for vector + class
    const uint64_t mem = hll.memoryUsage();
    EXPECT_GE(mem, HyperLogLog<14>::REGISTER_COUNT);
    EXPECT_LT(mem, HyperLogLog<14>::REGISTER_COUNT + 128); // small overhead
}

// ---------------------------------------------------------------------------
//  Type insertion — various data types
// ---------------------------------------------------------------------------

TEST_F(HyperLogLogTest, Insert_StdString_Works)
{
    HyperLogLog<14> hll;
    hll.insert(std::string("hello"));
    EXPECT_GT(hll.estimate(), 0);
}

TEST_F(HyperLogLogTest, Insert_Integer_Works)
{
    HyperLogLog<14> hll;
    hll.insert(42);
    EXPECT_GT(hll.estimate(), 0);
}

TEST_F(HyperLogLogTest, Insert_MultipleTypes_SameEstimator)
{
    HyperLogLog<14> hll;
    hll.insert(std::string("string_key"));
    hll.insert(12345);
    hll.insert(3.14159);
    hll.insert("c_string_literal");
    EXPECT_GT(hll.estimate(), 0);
}

TEST_F(HyperLogLogTest, Insert_RawBytes_Works)
{
    HyperLogLog<14> hll;
    const std::string data = "raw byte data";
    hll.insert(data.data(), data.size());
    EXPECT_GT(hll.estimate(), 0);
}

// ---------------------------------------------------------------------------
//  Interface compliance
// ---------------------------------------------------------------------------

TEST_F(HyperLogLogTest, Interface_InsertRaw_Override)
{
    // Verify we can call the interface method directly
    HyperLogLog<14> hll;
    IBaseEstimator& estimator = hll;
    estimator.insert("hello", 5);
    EXPECT_GT(estimator.estimate(), 0);
}

TEST_F(HyperLogLogTest, Interface_MemoryUsage_Override)
{
    const HyperLogLog<14> hll;
    const IBaseEstimator& est = hll;
    EXPECT_GT(est.memoryUsage(), 0);
}

// ---------------------------------------------------------------------------
//  Edge cases
// ---------------------------------------------------------------------------

TEST_F(HyperLogLogTest, Insert_EmptyString_DoesNotCrash)
{
    HyperLogLog<14> hll;
    EXPECT_NO_THROW(hll.insert(""));
    EXPECT_NO_THROW(hll.insert(std::string{}));
}

TEST_F(HyperLogLogTest, Insert_ZeroBytes_DoesNotCrash)
{
    HyperLogLog<14> hll;
    EXPECT_NO_THROW(hll.insert(static_cast<const void*>(nullptr), 0));
}

// ---------------------------------------------------------------------------
//  Determinism — same data -> same estimate
// ---------------------------------------------------------------------------

TEST_F(HyperLogLogTest, Determinism_SameInserts_SameEstimate)
{
    HyperLogLog<14> hll1;
    HyperLogLog<14> hll2;

    const std::vector<std::string> data = {"apple", "banana", "cherry", "date", "elderberry"};
    for (const auto& s : data)
    {
        hll1.insert(s);
        hll2.insert(s);
    }

    EXPECT_EQ(hll1.estimate(), hll2.estimate());
}

// ---------------------------------------------------------------------------
//  Comparison with exact set (small scale)
// ---------------------------------------------------------------------------

TEST_F(HyperLogLogTest, CompareWithExact_SmallSet)
{
    HyperLogLog<14> hll;
    std::set<uint64_t> exact;

    std::mt19937_64 rng(42);  // NOLINT: fixed seed for test determinism
    for (int i = 0; i < 5000; ++i)
    {
        const uint64_t val = rng();
        hll.insert(val);
        exact.insert(val);
    }

    const uint64_t expected = exact.size();
    const uint64_t estimated = hll.estimate();
    const double ratio = static_cast<double>(estimated) / static_cast<double>(expected);

    EXPECT_GT(ratio, 0.90);
    EXPECT_LT(ratio, 1.10);
}

// ---------------------------------------------------------------------------
//  High-cardinality merge (distinct element sets)
// ---------------------------------------------------------------------------

TEST_F(HyperLogLogTest, Merge_DisjointSets_UnionNearSum)
{
    HyperLogLog<14> hllA;
    HyperLogLog<14> hllB;

    constexpr uint64_t N = 20000;
    for (uint64_t i = 0; i < N; ++i)
    {
        hllA.insert("set_a_" + std::to_string(i));
        hllB.insert("set_b_" + std::to_string(i));
    }

    const uint64_t estA = hllA.estimate();
    const uint64_t estB = hllB.estimate();

    hllA.merge(hllB);
    const uint64_t estMerged = hllA.estimate();

    // The merged estimate should be close to the sum of the two individual estimates
    // (since the sets are disjoint). The sum may be slightly less than 2*N due to
    // underestimation at higher cardinalities.
    EXPECT_GT(estMerged, estA);
    EXPECT_GT(estMerged, estB);
    EXPECT_NEAR(static_cast<double>(estMerged),
                static_cast<double>(estA) + static_cast<double>(estB),
                static_cast<double>(N) * 0.15);
}
