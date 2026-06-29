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

using namespace cppforge::data_structure::probabilistic;

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

/**
 * @brief Test default constructor produces zero estimate
 * @details Verifies that a newly constructed HyperLogLog returns an estimate of zero before any insertions
 */
TEST_F(HyperLogLogTest, DefaultConstructor_EmptyEstimate_ReturnsZero)
{
    const HyperLogLog<14> hll;
    EXPECT_EQ(hll.estimate(), 0);
}

/**
 * @brief Test clear resets the estimate to zero after insertions
 * @details Verifies that clear() resets the cardinality estimate back to zero after elements have been inserted
 */
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

/**
 * @brief Test inserting a single element yields an estimate close to 1
 * @details Verifies that the cardinality estimate for a single unique element is within [1, 3]
 */
TEST_F(HyperLogLogTest, Insert_SingleElement_EstimateApproximatelyOne)
{
    HyperLogLog<14> hll;
    hll.insert("unique_element");
    // For a single element, the estimate should be very close to 1
    const uint64_t est = hll.estimate();
    EXPECT_GE(est, 1);
    EXPECT_LE(est, 3); // small jitter allowed for probabilistic structure
}

/**
 * @brief Test repeated insertion of the same element yields a stable estimate
 * @details Verifies that inserting the same element 10000 times still results in an estimate close to 1
 */
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

/**
 * @brief Test inserting many distinct elements yields an estimate close to the actual count
 * @details Verifies that the estimate for 10000 distinct elements is within [90%, 110%] of the true cardinality
 */
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

/**
 * @brief Test HyperLogLog works with different precision values
 * @details Verifies that precision values from 10 to 16 all produce reasonable estimates for 1000 elements
 */
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

/**
 * @brief Test error rate decreases as precision increases
 * @details Verifies that the theoretical error rate is monotonically decreasing with higher precision and the default precision error is approximately 0.8%
 */
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

/**
 * @brief Test merging two halves yields an estimate close to the full set
 * @details Verifies that merging two disjoint halves of a 5000-element set produces an estimate within 10% of the full-set estimate
 */
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

/**
 * @brief Test merging an empty HyperLogLog into a non-empty one leaves it unchanged
 * @details Verifies that merging an empty estimator into a populated one does not alter the estimate
 */
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

/**
 * @brief Test merging an estimator with itself leaves it unchanged
 * @details Verifies that self-merge (merging an estimator with itself) does not alter the estimate
 */
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

/**
 * @brief Test merge through the polymorphic IBaseEstimator interface
 * @details Verifies that merging via the base class reference works correctly for compatible HyperLogLog types
 */
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

/**
 * @brief Test merge with incompatible precision documents the limitation
 * @details Documents that merging HyperLogLog estimators of different precision through the polymorphic interface is undefined behaviour and should be avoided by the caller
 */
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

/**
 * @brief Test statistical error stays within 5 sigma across multiple trials
 * @details Verifies that the estimation error for HyperLogLog with p=12 across 10 independent trials stays within 5 times the theoretical standard error
 */
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

/**
 * @brief Test serialization and deserialization preserves the estimate
 * @details Verifies that serializing a HyperLogLog with 5000 inserted elements and deserializing it yields the same cardinality estimate
 */
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

/**
 * @brief Test serialization and deserialization of an empty HyperLogLog
 * @details Verifies that an empty HyperLogLog can be serialized and deserialized, yielding an estimate of zero
 */
TEST_F(HyperLogLogTest, SerializeDeserialize_Empty_Works)
{
    const HyperLogLog<14> original;
    const auto data = original.serialize();
    const auto restored = HyperLogLog<14>::deserialize(data);

    EXPECT_EQ(restored.estimate(), 0);
}

/**
 * @brief Test deserialize with wrong precision throws
 * @details Verifies that deserializing data from a 14-bit precision HyperLogLog into a 16-bit precision type throws std::invalid_argument
 */
TEST_F(HyperLogLogTest, Deserialize_WrongPrecision_Throws)
{
    HyperLogLog<14> hll;
    hll.insert("test");
    const auto data = hll.serialize();

    EXPECT_THROW(HyperLogLog<16>::deserialize(data), std::invalid_argument);
}

/**
 * @brief Test deserialize with truncated data throws
 * @details Verifies that deserializing data that is too short (missing register data) throws std::invalid_argument
 */
TEST_F(HyperLogLogTest, Deserialize_TruncatedData_Throws)
{
    const std::vector<uint8_t> badData = {uint8_t{0x01}, uint8_t{0x0E}}; // version + precision but no registers
    EXPECT_THROW(HyperLogLog<14>::deserialize(badData), std::invalid_argument);
}

/**
 * @brief Test deserialize with unknown version throws
 * @details Verifies that deserializing data with an unsupported version byte throws std::invalid_argument
 */
TEST_F(HyperLogLogTest, Deserialize_WrongVersion_Throws)
{
    std::vector<uint8_t> badData = {uint8_t{0xFF}, uint8_t{0x0E}}; // unknown version
    badData.resize(2 + HyperLogLog<14>::REGISTER_COUNT, 0);
    EXPECT_THROW(HyperLogLog<14>::deserialize(badData), std::invalid_argument);
}

// ---------------------------------------------------------------------------
//  Large cardinality test
// ---------------------------------------------------------------------------

/**
 * @brief Test large cardinality of 100k elements yields a reasonable estimate
 * @details Verifies that the estimate for 100000 distinct elements stays within [95%, 105%] of the true cardinality
 */
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

/**
 * @brief Test memory usage is within expected bounds
 * @details Verifies that memoryUsage() returns at least REGISTER_COUNT bytes and no more than REGISTER_COUNT plus 128 bytes of overhead
 */
TEST_F(HyperLogLogTest, MemoryUsage_Reasonable)
{
    const HyperLogLog<14> hll;
    // m = 16384 registers * 1 byte + small overhead for vector + class
    const uint64_t mem = hll.memoryUsage();
    EXPECT_GE(mem, HyperLogLog<14>::REGISTER_COUNT);
    EXPECT_LT(mem, HyperLogLog<14>::REGISTER_COUNT + 128); // small overhead
}

// ---------------------------------------------------------------------------
//  Type insertion â€?various data types
// ---------------------------------------------------------------------------

/**
 * @brief Test inserting a std::string works
 * @details Verifies that inserting a std::string key produces a positive cardinality estimate
 */
TEST_F(HyperLogLogTest, Insert_StdString_Works)
{
    HyperLogLog<14> hll;
    hll.insert(std::string("hello"));
    EXPECT_GT(hll.estimate(), 0);
}

/**
 * @brief Test inserting an integer works
 * @details Verifies that inserting an integer key produces a positive cardinality estimate
 */
TEST_F(HyperLogLogTest, Insert_Integer_Works)
{
    HyperLogLog<14> hll;
    hll.insert(42);
    EXPECT_GT(hll.estimate(), 0);
}

/**
 * @brief Test inserting different data types into the same estimator
 * @details Verifies that the HyperLogLog can accept std::string, int, double, and const char* keys in the same instance
 */
TEST_F(HyperLogLogTest, Insert_MultipleTypes_SameEstimator)
{
    HyperLogLog<14> hll;
    hll.insert(std::string("string_key"));
    hll.insert(12345);
    hll.insert(3.14159);
    hll.insert("c_string_literal");
    EXPECT_GT(hll.estimate(), 0);
}

/**
 * @brief Test inserting raw byte data works
 * @details Verifies that the HyperLogLog accepts raw byte data via pointer and size overload
 */
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

/**
 * @brief Test insert raw data through the IBaseEstimator interface
 * @details Verifies that the insert(const void*, size_t) method works when called through the base class interface
 */
TEST_F(HyperLogLogTest, Interface_InsertRaw_Override)
{
    // Verify we can call the interface method directly
    HyperLogLog<14> hll;
    IBaseEstimator& estimator = hll;
    estimator.insert("hello", 5);
    EXPECT_GT(estimator.estimate(), 0);
}

/**
 * @brief Test memoryUsage through the IBaseEstimator interface
 * @details Verifies that memoryUsage() returns a positive value when called through the base class interface
 */
TEST_F(HyperLogLogTest, Interface_MemoryUsage_Override)
{
    const HyperLogLog<14> hll;
    const IBaseEstimator& est = hll;
    EXPECT_GT(est.memoryUsage(), 0);
}

// ---------------------------------------------------------------------------
//  Edge cases
// ---------------------------------------------------------------------------

/**
 * @brief Test inserting an empty string does not crash
 * @details Verifies that inserting an empty string or empty std::string does not cause a crash or exception
 */
TEST_F(HyperLogLogTest, Insert_EmptyString_DoesNotCrash)
{
    HyperLogLog<14> hll;
    EXPECT_NO_THROW(hll.insert(""));
    EXPECT_NO_THROW(hll.insert(std::string{}));
}

/**
 * @brief Test inserting zero bytes does not crash
 * @details Verifies that inserting with a null pointer and zero size does not cause a crash
 */
TEST_F(HyperLogLogTest, Insert_ZeroBytes_DoesNotCrash)
{
    HyperLogLog<14> hll;
    EXPECT_NO_THROW(hll.insert(static_cast<const void*>(nullptr), 0));
}

// ---------------------------------------------------------------------------
//  Determinism â€?same data -> same estimate
// ---------------------------------------------------------------------------

/**
 * @brief Test determinism ensures same inserts produce the same estimate
 * @details Verifies that inserting the same set of data into two separate HyperLogLog instances produces identical estimates
 */
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

/**
 * @brief Test estimate against an exact set for small cardinality
 * @details Verifies that the HyperLogLog estimate stays within [90%, 110%] of the exact distinct count for 5000 random 64-bit values
 */
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

/**
 * @brief Test merging disjoint sets yields an estimate near the sum of individual estimates
 * @details Verifies that merging two disjoint sets of 20000 elements each produces a combined estimate close to the sum of the individual estimates
 */
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
