/**
 * @file CuckooFilterTest.cc
 * @brief Unit tests for the CuckooFilter class
 * @details Tests cover construction, insertion, containment queries, removal,
 *          cuckoo displacement, capacity limits, clear, introspection,
 *          comparison operators, range insertion, and template parameter
 *          variants.
 */

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "data_structure/filter/CuckooFilter.hpp"

using namespace cppforge::data_structure;

/**
 * @brief Test fixture for CuckooFilter tests
 */
class CuckooFilterTest : public testing::Test
{
protected:
    /// @brief Check whether a 64-bit integer is a power of two.
    static auto is_power_of_two(uint64_t n) -> bool
    {
        return n > 0 && (n & (n - 1)) == 0;
    }
};

// ══════════════════════════════════════════════════════════════════════════
//  Construction
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test constructing a filter with a non-zero expected capacity
 * @details Verifies that the filter has a positive number of buckets, zero
 *          elements initially, and a positive total capacity.
 */
TEST_F(CuckooFilterTest, Constructor_DefaultCapacity_ValidState)
{
    const CuckooFilter<std::string> filter(1000);

    EXPECT_GT(filter.size(), 0);
    EXPECT_EQ(filter.element_count(), 0);
    EXPECT_GT(filter.capacity(), 0);
}

/**
 * @brief Test constructing a filter with zero expected capacity
 * @details Verifies that even when zero capacity is requested, the filter
 *          allocates at least one bucket so that operations are well-defined.
 */
TEST_F(CuckooFilterTest, Constructor_ZeroCapacity_AtLeastOneBucket)
{
    const CuckooFilter<std::string> filter(0);

    EXPECT_GE(filter.size(), 1);
    EXPECT_EQ(filter.element_count(), 0);
}

/**
 * @brief Test that the number of buckets is always a power of two
 * @details The internal implementation rounds the bucket count up to the
 *          nearest power of two for efficient modulo via bitmask.
 */
TEST_F(CuckooFilterTest, Constructor_PowerOfTwoBuckets)
{
    const CuckooFilter<std::string> filter_a(100);
    const CuckooFilter<std::string> filter_b(1000);
    const CuckooFilter<std::string> filter_c(10000);

    EXPECT_TRUE(is_power_of_two(filter_a.size()));
    EXPECT_TRUE(is_power_of_two(filter_b.size()));
    EXPECT_TRUE(is_power_of_two(filter_c.size()));
}

// ══════════════════════════════════════════════════════════════════════════
//  Insert & Contains
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test inserting a single string key and verifying its presence
 * @details After inserting one key, contains() must report the key as present.
 *          A cuckoo filter guarantees no false negatives.
 */
TEST_F(CuckooFilterTest, Insert_SingleKey_ContainsReturnsTrue)
{
    CuckooFilter<std::string> filter(1000);

    filter.insert("hello");
    EXPECT_TRUE(filter.contains("hello"));
}

/**
 * @brief Test inserting multiple string keys and finding all of them
 * @details Verifies that all inserted keys are correctly reported as present.
 */
TEST_F(CuckooFilterTest, Insert_MultipleKeys_AllFound)
{
    CuckooFilter<std::string> filter(1000);

    filter.insert("apple");
    filter.insert("banana");
    filter.insert("cherry");

    EXPECT_TRUE(filter.contains("apple"));
    EXPECT_TRUE(filter.contains("banana"));
    EXPECT_TRUE(filter.contains("cherry"));
}

/**
 * @brief Test inserting integer keys
 * @details Verifies that the CuckooFilter works correctly with integer
 *          element types.
 */
TEST_F(CuckooFilterTest, Insert_IntegerKey_Works)
{
    CuckooFilter<int> filter(1000);

    filter.insert(42);
    filter.insert(-1);
    filter.insert(0);

    EXPECT_TRUE(filter.contains(42));
    EXPECT_TRUE(filter.contains(-1));
    EXPECT_TRUE(filter.contains(0));
}

/**
 * @brief Test contains returns false for a non-existent key
 * @details Verifies that contains() returns false for a key that was never
 *          inserted into the filter. Cuckoo filters have no false negatives.
 */
TEST_F(CuckooFilterTest, Contains_NonExistentKey_ReturnsFalse)
{
    CuckooFilter<std::string> filter(100);

    filter.insert("existing");
    EXPECT_FALSE(filter.contains("nonexistent"));
}

/**
 * @brief Test inserting the same key twice
 * @details Inserting a duplicate key adds another copy of the fingerprint.
 *          The filter should still report the key as present.
 */
TEST_F(CuckooFilterTest, Insert_DuplicateKey_StillContainsTrue)
{
    CuckooFilter<std::string> filter(1000);

    EXPECT_TRUE(filter.insert("key"));
    EXPECT_TRUE(filter.insert("key"));
    EXPECT_TRUE(filter.contains("key"));
}

/**
 * @brief Test inserting raw integer data of various widths
 * @details Verifies that CuckooFilter works with fundamental integer types
 *          such as uint32_t.
 */
TEST_F(CuckooFilterTest, Insert_RawData_ContainsWorks)
{
    CuckooFilter<uint32_t> filter(1000);

    const uint32_t value{42};
    filter.insert(value);
    EXPECT_TRUE(filter.contains(value));
}

// ══════════════════════════════════════════════════════════════════════════
//  Remove
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test removing an existing key returns true
 * @details Verifies that remove() returns true when the key was previously
 *          inserted.
 */
TEST_F(CuckooFilterTest, Remove_ExistingKey_ReturnsTrue)
{
    CuckooFilter<std::string> filter(1000);

    filter.insert("hello");
    EXPECT_TRUE(filter.remove("hello"));
}

/**
 * @brief Test that after removal, contains returns false
 * @details Verifies that removing a key causes contains() to report it as
 *          absent. This is a guaranteed property �?no false negatives.
 */
TEST_F(CuckooFilterTest, Remove_ExistingKey_AfterRemove_ContainsReturnsFalse)
{
    CuckooFilter<std::string> filter(1000);

    filter.insert("hello");
    filter.remove("hello");
    EXPECT_FALSE(filter.contains("hello"));
}

/**
 * @brief Test removing a key that was never inserted
 * @details Verifies that remove() returns false for a non-existent key.
 */
TEST_F(CuckooFilterTest, Remove_NonExistentKey_ReturnsFalse)
{
    CuckooFilter<std::string> filter(1000);

    filter.insert("existing");
    EXPECT_FALSE(filter.remove("nonexistent"));
}

/**
 * @brief Test insert �?remove �?reinsert workflow
 * @details Verifies that a key can be removed and then reinserted, and that
 *          contains() correctly reports its presence after reinsertion.
 */
TEST_F(CuckooFilterTest, Remove_InsertRemoveReinsert_Works)
{
    CuckooFilter<std::string> filter(1000);

    filter.insert("key");
    filter.remove("key");
    EXPECT_FALSE(filter.contains("key"));

    filter.insert("key");
    EXPECT_TRUE(filter.contains("key"));
}

/**
 * @brief Test removing all inserted elements resets element count to zero
 * @details Verifies that after inserting and removing several keys the
 *          element_count returns to zero.
 */
TEST_F(CuckooFilterTest, Remove_AllElements_ElementCountZero)
{
    CuckooFilter<std::string> filter(10000);

    filter.insert("a");
    filter.insert("b");
    filter.insert("c");

    EXPECT_TRUE(filter.remove("a"));
    EXPECT_TRUE(filter.remove("b"));
    EXPECT_TRUE(filter.remove("c"));

    EXPECT_EQ(filter.element_count(), 0);
}

// ══════════════════════════════════════════════════════════════════════════
//  Cuckoo Displacement & Capacity
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test inserting up to the filter's approximate capacity
 * @details Verifies that inserting up to ~80% of total capacity succeeds.
 *          Cuckoo filters typically achieve 95%+ load before failures begin.
 */
TEST_F(CuckooFilterTest, Insert_FillToCapacity_AllSucceed)
{
    CuckooFilter<int> filter(1000);
    const uint64_t target = static_cast<uint64_t>(
        static_cast<double>(filter.capacity()) * 0.80);

    for (uint64_t i = 0; i < target; ++i)
    {
        EXPECT_TRUE(filter.insert(static_cast<int>(i * 2 + 1)));
    }

    EXPECT_EQ(filter.element_count(), target);
}

/**
 * @brief Test that insertion returns false when the filter is overfilled
 * @details Constructs a very small filter and inserts many unique keys.
 *          Once the filter is saturated, insert() must return false.
 */
TEST_F(CuckooFilterTest, Insert_ExceedCapacity_ReturnsFalse)
{
    // 1 bucket �?4 slots; both bucket indices map to the same bucket
    CuckooFilter<int> filter(1);

    // Insert many keys with distinct values; some will definitely fail
    bool any_failure = false;
    for (int i = 0; i < 100; ++i)
    {
        if (!filter.insert(i))
        {
            any_failure = true;
            break;
        }
    }

    EXPECT_TRUE(any_failure);
}

/**
 * @brief Test that element_count remains consistent when insert fails.
 * @details After a failed insert(), the element count must remain unchanged,
 *          and the filter state must be internally consistent.
 */
TEST_F(CuckooFilterTest, Insert_Fail_ElementCountConsistent)
{
    CuckooFilter<int> filter(1);  // Very small filter

    // Fill the filter
    uint64_t success_count = 0;
    int i = 0;
    for (; i < 10000; ++i)
    {
        if (!filter.insert(i))
        {
            break;
        }
        ++success_count;
    }

    // Filter should be full now and further inserts should fail
    const auto count_before = filter.element_count();

    // Try more inserts �?they should all fail
    for (int j = 0; j < 100; ++j)
    {
        EXPECT_FALSE(filter.insert(i + j));
    }

    // Element count must remain unchanged
    EXPECT_EQ(filter.element_count(), count_before);
}

/**
 * @brief Test high-load insertion with cuckoo displacement
 * @details Inserts a large number of keys to trigger the cuckoo displacement
 *          loop. Verifies that no crash or undefined behaviour occurs and
 *          that the load factor is high.
 */
TEST_F(CuckooFilterTest, CuckooDisplacement_HighLoad_NoCrash)
{
    CuckooFilter<uint64_t> filter(10000);
    const uint64_t target = static_cast<uint64_t>(
        static_cast<double>(filter.capacity()) * 0.95);

    uint64_t inserted = 0;
    for (uint64_t i = 0; i < target * 2; ++i)
    {
        if (filter.insert(i))
        {
            ++inserted;
        }
    }

    // Should have successfully inserted a high percentage
    EXPECT_GT(inserted, filter.capacity() / 2);
    EXPECT_GT(filter.load_factor(), 0.50);
}

// ══════════════════════════════════════════════════════════════════════════
//  Clear
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test clear resets the filter state
 * @details Verifies that clear() removes all inserted keys, resets element
 *          count to zero, and previously inserted keys are no longer reported
 *          as present.
 */
TEST_F(CuckooFilterTest, Clear_ResetsState)
{
    CuckooFilter<std::string> filter(1000);

    filter.insert("hello");
    EXPECT_TRUE(filter.contains("hello"));

    filter.clear();
    EXPECT_FALSE(filter.contains("hello"));
    EXPECT_EQ(filter.element_count(), 0);

    // After clear, new insertions still work
    filter.insert("world");
    EXPECT_TRUE(filter.contains("world"));
}

// ══════════════════════════════════════════════════════════════════════════
//  Introspection
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test element_count tracks the number of insertions
 * @details Verifies that element_count() increments with each successful
 *          insertion and reflects the correct count.
 */
TEST_F(CuckooFilterTest, ElementCount_TracksInsertions)
{
    CuckooFilter<std::string> filter(1000);

    EXPECT_EQ(filter.element_count(), 0);

    filter.insert("a");
    EXPECT_EQ(filter.element_count(), 1);

    filter.insert("b");
    EXPECT_EQ(filter.element_count(), 2);

    filter.insert("c");
    EXPECT_EQ(filter.element_count(), 3);
}

/**
 * @brief Test element_count decrements on remove
 * @details Verifies that a successful remove() decreases the element count
 *          by one.
 */
TEST_F(CuckooFilterTest, ElementCount_Remove_Decrements)
{
    CuckooFilter<std::string> filter(1000);

    filter.insert("a");
    filter.insert("b");
    filter.insert("c");
    EXPECT_EQ(filter.element_count(), 3);

    filter.remove("b");
    EXPECT_EQ(filter.element_count(), 2);

    filter.remove("a");
    EXPECT_EQ(filter.element_count(), 1);
}

/**
 * @brief Test load_factor is in the (0,1) range after insertions
 * @details Verifies that after inserting some elements, the load factor is
 *          strictly between zero and one.
 */
TEST_F(CuckooFilterTest, LoadFactor_AfterInsertions_Reasonable)
{
    CuckooFilter<std::string> filter(1000);

    for (int i = 0; i < 100; ++i)
    {
        filter.insert(std::to_string(i));
    }

    const double lf = filter.load_factor();
    EXPECT_GT(lf, 0.0);
    EXPECT_LT(lf, 1.0);
}

/**
 * @brief Test capacity formula: capacity = size() * entries_per_bucket()
 * @details Verifies that the reported total capacity matches the product of
 *          the number of buckets and the compile-time entries-per-bucket
 *          constant.
 */
TEST_F(CuckooFilterTest, Capacity_EntryPerBucket_Formula)
{
    const CuckooFilter<std::string> filter(1000);

    EXPECT_EQ(filter.capacity(),
              filter.size() * filter.entries_per_bucket());
}

/**
 * @brief Test effective_fpp returns a value in (0,1)
 * @details Verifies that the effective false-positive probability is a
 *          plausible value between zero and one.
 */
TEST_F(CuckooFilterTest, EffectiveFpp_InRange)
{
    const CuckooFilter<std::string> filter(1000);

    const double fpp = filter.effective_fpp();
    EXPECT_GT(fpp, 0.0);
    EXPECT_LT(fpp, 1.0);
}

// ══════════════════════════════════════════════════════════════════════════
//  Comparison
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test equality operator for filters with identical content
 * @details Verifies that two filters constructed with the same parameters
 *          and identical inserted keys compare as equal.
 */
TEST_F(CuckooFilterTest, Equality_SameContent_ReturnsTrue)
{
    CuckooFilter<std::string> a(1000);
    CuckooFilter<std::string> b(1000);

    a.insert("alpha");
    a.insert("beta");
    b.insert("alpha");
    b.insert("beta");

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

/**
 * @brief Test inequality operator for filters with different content
 * @details Verifies that two filters with different inserted keys compare
 *          as unequal.
 */
TEST_F(CuckooFilterTest, Inequality_DifferentContent_ReturnsTrue)
{
    CuckooFilter<std::string> a(1000);
    const CuckooFilter<std::string> b(1000);

    a.insert("unique");
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
}

// ══════════════════════════════════════════════════════════════════════════
//  Range Insert
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test inserting a range of keys via iterators
 * @details Verifies that insert(begin, end) adds all keys in the range and
 *          contains() reports each one as present.
 */
TEST_F(CuckooFilterTest, Insert_RangeOfKeys_AllContained)
{
    CuckooFilter<std::string> filter(100);
    const std::vector<std::string> keys = {"alpha", "beta", "gamma"};

    filter.insert(keys.begin(), keys.end());

    EXPECT_TRUE(filter.contains("alpha"));
    EXPECT_TRUE(filter.contains("beta"));
    EXPECT_TRUE(filter.contains("gamma"));
}

// ══════════════════════════════════════════════════════════════════════════
//  Template Variants
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test CuckooFilter with 16-bit fingerprints
 * @details Verifies that the FingerprintSize<16> variant compiles and
 *          functions correctly.
 */
TEST_F(CuckooFilterTest, Fingerprint16_Works)
{
    CuckooFilter<int, 4, FingerprintSize<16>> filter(100);

    filter.insert(42);
    EXPECT_TRUE(filter.contains(42));
    EXPECT_FALSE(filter.contains(999));
    EXPECT_EQ(filter.fingerprint_bits(), 16);
}

/**
 * @brief Test CuckooFilter with 32-bit fingerprints
 * @details Verifies that the FingerprintSize<32> variant compiles and
 *          functions correctly.
 */
TEST_F(CuckooFilterTest, Fingerprint32_Works)
{
    CuckooFilter<int, 4, FingerprintSize<32>> filter(100);

    filter.insert(42);
    EXPECT_TRUE(filter.contains(42));
    EXPECT_FALSE(filter.contains(999));
    EXPECT_EQ(filter.fingerprint_bits(), 32);
}

/**
 * @brief Test CuckooFilter with 2 entries per bucket
 * @details Verifies that reducing EntriesPerBucket (e.g. to 2) compiles
 *          and works correctly.
 */
TEST_F(CuckooFilterTest, EntriesPerBucket2_Works)
{
    CuckooFilter<int, 2> filter(100);

    filter.insert(1);
    filter.insert(2);
    EXPECT_TRUE(filter.contains(1));
    EXPECT_TRUE(filter.contains(2));
    EXPECT_EQ(filter.entries_per_bucket(), 2);
}

/// @brief A custom hash functor that divides by 32 (high collision rate).
struct HashDiv32
{
    auto operator()(int key) const noexcept -> uint64_t
    {
        return static_cast<uint64_t>(static_cast<uint32_t>(key)) / 32;
    }
};

/**
 * @brief Test CuckooFilter with a custom hash functor
 * @details Verifies that providing an alternative hash implementation
 *          compiles and the filter operates correctly.
 */
TEST_F(CuckooFilterTest, CustomHash_Works)
{
    CuckooFilter<int, 4, FingerprintSize<8>, HashDiv32> filter(100);

    filter.insert(100);
    filter.insert(200);
    EXPECT_TRUE(filter.contains(100));
    EXPECT_TRUE(filter.contains(200));
}

/**
 * @brief Test CuckooFilter with a custom hash functor �?remove operation.
 * @details Verifies that remove() works correctly with a custom hash.
 */
TEST_F(CuckooFilterTest, CustomHash_Remove_Works)
{
    CuckooFilter<int, 4, FingerprintSize<8>, HashDiv32> filter(100);

    EXPECT_TRUE(filter.insert(100));
    EXPECT_TRUE(filter.contains(100));

    EXPECT_TRUE(filter.remove(100));
    EXPECT_FALSE(filter.contains(100));
    EXPECT_EQ(filter.element_count(), 0);
}

// ══════════════════════════════════════════════════════════════════════════
//  Edge Cases
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test remove then reinsert the same key
 * @details Verifies that a key can be removed and then inserted again,
 *          and that the filter correctly reports its presence.
 */
TEST_F(CuckooFilterTest, RemoveThenInsert_SameKey_Works)
{
    CuckooFilter<std::string> filter(1000);

    EXPECT_TRUE(filter.insert("key"));
    EXPECT_TRUE(filter.remove("key"));
    EXPECT_FALSE(filter.contains("key"));

    EXPECT_TRUE(filter.insert("key"));
    EXPECT_TRUE(filter.contains("key"));
}

/**
 * @brief Test inserting an empty string
 * @details Verifies that an empty string can be used as a key without
 *          causing issues.
 */
TEST_F(CuckooFilterTest, Insert_EmptyString_Works)
{
    CuckooFilter<std::string> filter(1000);

    EXPECT_TRUE(filter.insert(""));
    EXPECT_TRUE(filter.contains(""));
    EXPECT_EQ(filter.element_count(), 1);

    // Non-empty keys still work alongside empty keys
    filter.insert("nonempty");
    EXPECT_TRUE(filter.contains("nonempty"));
}

/**
 * @brief Test large bulk insertion preserves correct element count
 * @details Inserts a large number of distinct keys and verifies that
 *          element_count() matches the number of successful insertions.
 */
TEST_F(CuckooFilterTest, LargeBulkInsert_CapacityCorrect)
{
    CuckooFilter<uint64_t> filter(100000);
    constexpr uint64_t NUM_KEYS = 50000;

    uint64_t succeeded = 0;
    for (uint64_t i = 0; i < NUM_KEYS; ++i)
    {
        if (filter.insert(i))
        {
            ++succeeded;
        }
    }

    EXPECT_EQ(filter.element_count(), succeeded);
    EXPECT_GT(succeeded, NUM_KEYS * 9 / 10);  // at least 90% should succeed
}
