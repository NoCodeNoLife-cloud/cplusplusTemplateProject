/**
 * @file SparseTableTest.cc
 * @brief Unit tests for the SparseTable class
 * @details Tests cover constructors, range queries (min, max, gcd),
 *          edge cases (empty table, single element, boundary sizes),
 *          element access, swap, and type compatibility.  All custom
 *          merge operations (MaxOp, GCDOp) are verified with random
 *          interval queries against brute-force computation.
 */

#include <gtest/gtest.h>

#include <algorithm>   // std::min_element, std::max_element
#include <cstddef>     // size_t
#include <numeric>     // std::gcd
#include <random>      // std::mt19937
#include <vector>

#include <cppforge/data_structure/tree/range/SparseTable.hpp>

// ==================== Custom Merge Operations ====================

namespace
{
    /// @brief Idempotent merge operation that returns the larger of two values.
    struct MaxOp
    {
        template <typename T>
        [[nodiscard]] constexpr auto operator()(const T& a, const T& b) const
            noexcept(noexcept(a < b)) -> const T&
        {
            return a < b ? b : a;
        }
    };

    /// @brief Idempotent merge operation that returns the GCD of two values.
    struct GCDOp
    {
        template <typename T>
        [[nodiscard]] constexpr auto operator()(const T& a, const T& b) const -> T
        {
            return std::gcd(a, b);
        }
    };
} // anonymous namespace

using namespace cppforge::data_structure::tree::range;

// ==================== Test Fixture ====================

class SparseTableTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ══════════════════════════════════════════════════════�?//  Constructor Tests
// ══════════════════════════════════════════════════════�?
/**
 * @brief Test default constructor creates an empty sparse table
 * @details Verifies that a default-constructed SparseTable is empty
 *          with zero size.
 */
TEST_F(SparseTableTest, DefaultConstructor_EmptyTree)
{
    const SparseTable<int> st;
    EXPECT_TRUE(st.empty());
    EXPECT_EQ(st.size(), 0);
}

/**
 * @brief Test size constructor creates a table with the correct size
 * @details Verifies that constructing with a positive size creates a
 *          non-empty sparse table where all elements are value-initialised.
 */
TEST_F(SparseTableTest, SizeConstructor_CorrectSize)
{
    const SparseTable<int> st(5);
    EXPECT_FALSE(st.empty());
    EXPECT_EQ(st.size(), 5);
    // All elements are value-initialised to 0.
    for (size_t i = 0; i < 5; ++i)
    {
        EXPECT_EQ(st.at(i), 0);
    }
}

/**
 * @brief Test size constructor with zero
 * @details Verifies that constructing a sparse table with n = 0
 *          produces an empty table.
 */
TEST_F(SparseTableTest, SizeConstructor_ZeroSize)
{
    const SparseTable<int> st(0);
    EXPECT_TRUE(st.empty());
    EXPECT_EQ(st.size(), 0);
}

/**
 * @brief Test size + initial-value constructor
 * @details Verifies that every element is initialised to the supplied value.
 */
TEST_F(SparseTableTest, SizeInitValueConstructor_AllInitialized)
{
    const SparseTable<int> st(5, 42);
    EXPECT_EQ(st.size(), 5);
    for (size_t i = 0; i < 5; ++i)
    {
        EXPECT_EQ(st.at(i), 42);
    }
    EXPECT_EQ(st.query(0, 5), 42);
}

/**
 * @brief Test iterator range constructor from a vector
 * @details Verifies that constructing from an iterator range stores
 *          the same elements and supports correct queries.
 */
TEST_F(SparseTableTest, IteratorConstructor_FromVector)
{
    const std::vector<int> data = {9, 3, 7, 1, 8, 2};
    const SparseTable<int> st(data.begin(), data.end());
    EXPECT_EQ(st.size(), 6);
    EXPECT_EQ(st.query(0, 6), 1);  // overall minimum
    EXPECT_EQ(st.query(1, 5), 1);  // min of {3,7,1,8}
    EXPECT_EQ(st.query(2, 4), 1);  // min of {7,1}
}

/**
 * @brief Test iterator constructor with an empty range
 * @details Verifies that an empty range produces an empty sparse table.
 */
TEST_F(SparseTableTest, IteratorConstructor_EmptyRange)
{
    const std::vector<int> data;
    const SparseTable<int> st(data.begin(), data.end());
    EXPECT_TRUE(st.empty());
    EXPECT_EQ(st.size(), 0);
}

/**
 * @brief Test initializer list constructor
 * @details Verifies that initializer list construction stores the
 *          correct values and supports query.
 */
TEST_F(SparseTableTest, InitializerListConstructor)
{
    const SparseTable<int> st = {9, 3, 7, 1, 8, 2};
    EXPECT_EQ(st.size(), 6);
    EXPECT_EQ(st.query(0, 6), 1);
    EXPECT_EQ(st.query(2, 5), 1);  // min of {7,1,8}
}

/**
 * @brief Test initializer list constructor with an empty list
 * @details Verifies that an empty initializer list produces an empty
 *          sparse table.
 */
TEST_F(SparseTableTest, InitializerListConstructor_Empty)
{
    const SparseTable<int> st = {};
    EXPECT_TRUE(st.empty());
    EXPECT_EQ(st.size(), 0);
}

// ══════════════════════════════════════════════════════�?//  Query Tests
// ══════════════════════════════════════════════════════�?
/**
 * @brief Test single-element range query
 * @details query(i, i+1) returns the value of the element at position i.
 */
TEST_F(SparseTableTest, Query_SingleElement)
{
    const SparseTable<int> st = {9, 3, 7, 1, 8, 2};
    EXPECT_EQ(st.query(0, 1), 9);
    EXPECT_EQ(st.query(1, 2), 3);
    EXPECT_EQ(st.query(2, 3), 7);
    EXPECT_EQ(st.query(3, 4), 1);
    EXPECT_EQ(st.query(4, 5), 8);
    EXPECT_EQ(st.query(5, 6), 2);
}

/**
 * @brief Test full-range query
 * @details query(0, n) returns the minimum over every element.
 */
TEST_F(SparseTableTest, Query_FullRange)
{
    const SparseTable<int> st = {9, 3, 7, 1, 8, 2};
    EXPECT_EQ(st.query(0, 6), 1);
}

/**
 * @brief Test prefix and suffix queries against manual calculation
 * @details Verifies that query results for various prefixes and suffixes
 *          match the expected minima computed by hand.
 */
TEST_F(SparseTableTest, Query_PrefixSuffix)
{
    // data:  [5, 2, 8, 4, 1, 9, 3]
    const SparseTable<int> st = {5, 2, 8, 4, 1, 9, 3};

    // Prefix minima
    EXPECT_EQ(st.query(0, 1), 5);
    EXPECT_EQ(st.query(0, 2), 2);
    EXPECT_EQ(st.query(0, 3), 2);
    EXPECT_EQ(st.query(0, 4), 2);
    EXPECT_EQ(st.query(0, 5), 1);
    EXPECT_EQ(st.query(0, 6), 1);
    EXPECT_EQ(st.query(0, 7), 1);

    // Suffix minima
    EXPECT_EQ(st.query(6, 7), 3);
    EXPECT_EQ(st.query(5, 7), 3);
    EXPECT_EQ(st.query(4, 7), 1);
    EXPECT_EQ(st.query(3, 7), 1);
    EXPECT_EQ(st.query(2, 7), 1);
    EXPECT_EQ(st.query(1, 7), 1);
    EXPECT_EQ(st.query(0, 7), 1);

    // Interior ranges
    EXPECT_EQ(st.query(1, 4), 2);  // {2,8,4}
    EXPECT_EQ(st.query(2, 5), 1);  // {8,4,1}
}

/**
 * @brief Test that adjacent intervals compose consistently
 * @details For any split point mid, the min of the left and right
 *          sub-ranges must be >= the min of the combined range.
 *          Equivalently, min(left_min, right_min) == combined_min.
 */
TEST_F(SparseTableTest, Query_AdjacentIntervalsCompose)
{
    const SparseTable<int> st = {5, 2, 8, 4, 1, 9, 3, 6};

    for (size_t l = 0; l < 7; ++l)
    {
        for (size_t r = l + 2; r <= 8; ++r)
        {
            for (size_t mid = l + 1; mid < r; ++mid)
            {
                auto left_min  = st.query(l, mid);
                auto right_min = st.query(mid, r);
                auto combined  = st.query(l, r);
                EXPECT_LE(combined, left_min);
                EXPECT_LE(combined, right_min);
                EXPECT_EQ(combined, std::min(left_min, right_min));
            }
        }
    }
}

/**
 * @brief Test random interval queries for correctness
 * @details Generates a random array of size 100 and performs 1000
 *          random range queries, comparing each SparseTable result
 *          against a brute-force scan.  Uses fixed seed for
 *          reproducibility.
 */
TEST_F(SparseTableTest, Query_RandomIntervals)
{
    // NOLINTBEGIN(cert-msc32-c,cert-msc51-cpp)
    std::mt19937 rng(42);
    // NOLINTEND(cert-msc32-c,cert-msc51-cpp)

    constexpr size_t kSize   = 100;
    constexpr size_t kTrials = 1000;

    std::vector<int> data(kSize);
    for (size_t i = 0; i < kSize; ++i)
    {
        data[i] = static_cast<int>(rng() % 10000);  // NOLINT
    }

    const SparseTable<int> st(data.begin(), data.end());
    ASSERT_EQ(st.size(), kSize);

    for (size_t trial = 0; trial < kTrials; ++trial)
    {
        size_t l = static_cast<size_t>(rng()) % kSize;
        size_t r = l + 1 + (static_cast<size_t>(rng()) % (kSize - l));
        ASSERT_LE(r, kSize);

        auto expected = *std::min_element(data.begin() + l,
                                          data.begin() + r);
        EXPECT_EQ(st.query(l, r), expected)
            << "Mismatch for query(" << l << ", " << r << ")";
    }
}

/**
 * @brief Test query with l == r-1 (length one, fast path)
 * @details The implementation has a fast path for single-element queries;
 *          this test exercises that path for every index.
 */
TEST_F(SparseTableTest, Query_LengthOne)
{
    const SparseTable<int> st = {10, 20, 30, 40, 50};
    for (size_t i = 0; i < 5; ++i)
    {
        EXPECT_EQ(st.query(i, i + 1), static_cast<int>((i + 1) * 10));
    }
}

/**
 * @brief Test query with invalid range where l >= r throws
 * @details Verifies that query throws std::out_of_range when the
 *          left bound is not strictly less than the right bound.
 */
TEST_F(SparseTableTest, Query_InvalidRangeLGEQR_Throws)
{
    const SparseTable<int> st = {1, 2, 3};
    EXPECT_THROW(static_cast<void>(st.query(2, 2)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(st.query(2, 1)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(st.query(0, 0)), std::out_of_range);
}

/**
 * @brief Test query with right bound exceeding size throws
 * @details Verifies that query throws std::out_of_range when r > n.
 */
TEST_F(SparseTableTest, Query_OutOfBoundsR_Throws)
{
    const SparseTable<int> st = {1, 2, 3};
    EXPECT_THROW(static_cast<void>(st.query(0, 4)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(st.query(2, 5)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(st.query(0, 100)), std::out_of_range);
}

/**
 * @brief Test query on an empty table throws
 * @details Verifies that query throws std::out_of_range when the
 *          sparse table has no elements.
 */
TEST_F(SparseTableTest, Query_EmptyTable_Throws)
{
    const SparseTable<int> st;
    EXPECT_THROW(static_cast<void>(st.query(0, 1)), std::out_of_range);
}

// ══════════════════════════════════════════════════════�?//  Custom MergeOp Tests
// ══════════════════════════════════════════════════════�?
/**
 * @brief Test SparseTable with MaxOp for range-maximum queries
 * @details Uses the custom MaxOp defined in the anonymous namespace to
 *          verify that the sparse table supports maximum queries.
 */
TEST_F(SparseTableTest, MaxOp_BasicCorrectness)
{
    const SparseTable<int, MaxOp> st = {3, 9, 2, 7, 5, 8};

    EXPECT_EQ(st.query(0, 1), 3);
    EXPECT_EQ(st.query(1, 2), 9);
    EXPECT_EQ(st.query(0, 2), 9);
    EXPECT_EQ(st.query(0, 6), 9);
    EXPECT_EQ(st.query(2, 5), 7);   // {2, 7, 5}
    EXPECT_EQ(st.query(4, 6), 8);   // {5, 8}
}

/**
 * @brief Test random interval queries with MaxOp
 * @details Generates a random array and performs random queries
 *          comparing SparseTable with MaxOp against brute-force
 *          max_element.
 */
TEST_F(SparseTableTest, MaxOp_RandomIntervals)
{
    // NOLINTBEGIN(cert-msc32-c,cert-msc51-cpp)
    std::mt19937 rng(42);
    // NOLINTEND(cert-msc32-c,cert-msc51-cpp)

    constexpr size_t kSize   = 100;
    constexpr size_t kTrials = 1000;

    std::vector<int> data(kSize);
    for (size_t i = 0; i < kSize; ++i)
    {
        data[i] = static_cast<int>(rng() % 10000);  // NOLINT
    }

    const SparseTable<int, MaxOp> st(data.begin(), data.end());

    for (size_t trial = 0; trial < kTrials; ++trial)
    {
        size_t l = static_cast<size_t>(rng()) % kSize;
        size_t r = l + 1 + (static_cast<size_t>(rng()) % (kSize - l));

        auto expected = *std::max_element(data.begin() + l,
                                          data.begin() + r);
        EXPECT_EQ(st.query(l, r), expected)
            << "Mismatch for MaxOp query(" << l << ", " << r << ")";
    }
}

/**
 * @brief Test SparseTable with GCDOp for range-GCD queries
 * @details Uses std::gcd as the merge operation.  Verifies both
 *          hand-picked known values and random interval queries.
 */
TEST_F(SparseTableTest, GCDOp_BasicCorrectness)
{
    // data: {6, 15, 10, 25, 30}
    const SparseTable<int, GCDOp> st = {6, 15, 10, 25, 30};

    // Whole array
    EXPECT_EQ(st.query(0, 5), 1);   // gcd of all = 1

    // Prefixes
    EXPECT_EQ(st.query(0, 1), 6);   // gcd(6)
    EXPECT_EQ(st.query(0, 2), 3);   // gcd(6, 15) = 3
    EXPECT_EQ(st.query(0, 3), 1);   // gcd(6, 15, 10) = 1

    // Suffixes
    EXPECT_EQ(st.query(4, 5), 30);  // gcd(30)
    EXPECT_EQ(st.query(3, 5), 5);   // gcd(25, 30) = 5
    EXPECT_EQ(st.query(2, 5), 5);   // gcd(10, 25, 30) = 5

    // Interiors
    EXPECT_EQ(st.query(1, 4), 5);   // gcd(15, 10, 25) = 5
    EXPECT_EQ(st.query(1, 3), 5);   // gcd(15, 10) = 5
}

/**
 * @brief Test random interval queries with GCDOp
 * @details Generates a random array and performs random queries
 *          comparing SparseTable with GCDOp against brute-force
 *          GCD computation.
 */
TEST_F(SparseTableTest, GCDOp_RandomIntervals)
{
    // NOLINTBEGIN(cert-msc32-c,cert-msc51-cpp)
    std::mt19937 rng(42);
    // NOLINTEND(cert-msc32-c,cert-msc51-cpp)

    constexpr size_t kSize   = 50;
    constexpr size_t kTrials = 500;

    std::vector<int> data(kSize);
    for (size_t i = 0; i < kSize; ++i)
    {
        // Use values in [1, 1000] to avoid zero-gcd issues.
        data[i] = 1 + (static_cast<int>(rng() % 1000));  // NOLINT
    }

    const SparseTable<int, GCDOp> st(data.begin(), data.end());

    for (size_t trial = 0; trial < kTrials; ++trial)
    {
        size_t l = static_cast<size_t>(rng()) % kSize;
        size_t r = l + 1 + (static_cast<size_t>(rng()) % (kSize - l));

        int expected = 0;
        for (size_t i = l; i < r; ++i)
        {
            expected = std::gcd(expected, data[i]);
        }
        EXPECT_EQ(st.query(l, r), expected)
            << "Mismatch for GCDOp query(" << l << ", " << r << ")";
    }
}

// ══════════════════════════════════════════════════════�?//  At / Empty / Size Tests
// ══════════════════════════════════════════════════════�?
/**
 * @brief Test at() element access
 * @details Verifies that at() returns correct values at various positions.
 */
TEST_F(SparseTableTest, At_AccessElements)
{
    const SparseTable<int> st = {10, 20, 30, 40, 50};
    EXPECT_EQ(st.at(0), 10);
    EXPECT_EQ(st.at(1), 20);
    EXPECT_EQ(st.at(2), 30);
    EXPECT_EQ(st.at(3), 40);
    EXPECT_EQ(st.at(4), 50);
}

/**
 * @brief Test at() with out-of-bounds index throws
 * @details Verifies that at() throws std::out_of_range for invalid positions.
 */
TEST_F(SparseTableTest, At_OutOfBounds_Throws)
{
    const SparseTable<int> st = {1, 2, 3};
    EXPECT_THROW(static_cast<void>(st.at(3)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(st.at(100)), std::out_of_range);
}

/**
 * @brief Test at() on an empty table throws
 * @details Verifies that at() throws std::out_of_range when table is empty.
 */
TEST_F(SparseTableTest, At_EmptyTable_Throws)
{
    const SparseTable<int> st;
    EXPECT_THROW(static_cast<void>(st.at(0)), std::out_of_range);
}

/**
 * @brief Test empty() returns true for a default-constructed table
 * @details Verifies that empty() correctly identifies an empty table.
 */
TEST_F(SparseTableTest, Empty_InitiallyEmpty)
{
    const SparseTable<int> st;
    EXPECT_TRUE(st.empty());
}

/**
 * @brief Test empty() returns false for a non-empty table
 * @details Verifies that empty() correctly identifies a table with elements.
 */
TEST_F(SparseTableTest, Empty_NonEmpty)
{
    const SparseTable<int> st(5);
    EXPECT_FALSE(st.empty());
}

/**
 * @brief Test size() returns correct element count
 * @details Verifies that size() returns the number of elements for various sizes.
 */
TEST_F(SparseTableTest, Size_CorrectCount)
{
    const SparseTable<int> st;
    EXPECT_EQ(st.size(), 0);

    const SparseTable<int> st2(10);
    EXPECT_EQ(st2.size(), 10);

    const SparseTable<int> st3(100);
    EXPECT_EQ(st3.size(), 100);
}

// ══════════════════════════════════════════════════════�?//  Swap Tests
// ══════════════════════════════════════════════════════�?
/**
 * @brief Test swap exchanges contents between two non-empty tables
 * @details Verifies that after swap, each table holds the other's data.
 */
TEST_F(SparseTableTest, Swap_ExchangesContents)
{
    SparseTable<int> st1 = {1, 2, 3};
    SparseTable<int> st2 = {10, 20};

    st1.swap(st2);

    EXPECT_EQ(st1.size(), 2);
    EXPECT_EQ(st1.at(0), 10);
    EXPECT_EQ(st1.at(1), 20);

    EXPECT_EQ(st2.size(), 3);
    EXPECT_EQ(st2.at(0), 1);
    EXPECT_EQ(st2.at(1), 2);
    EXPECT_EQ(st2.at(2), 3);
}

/**
 * @brief Test swap between an empty and a non-empty table
 * @details Verifies that swapping a non-empty table with an empty one
 *          correctly transfers ownership.
 */
TEST_F(SparseTableTest, Swap_WithEmpty)
{
    SparseTable<int> st1 = {5, 10};
    SparseTable<int> st2;

    st1.swap(st2);

    EXPECT_TRUE(st1.empty());
    EXPECT_EQ(st1.size(), 0);

    EXPECT_EQ(st2.size(), 2);
    EXPECT_EQ(st2.at(0), 5);
    EXPECT_EQ(st2.at(1), 10);
}

/**
 * @brief Test swap is symmetric
 * @details Verifies that swapping twice restores the original contents.
 */
TEST_F(SparseTableTest, Swap_Symmetric)
{
    SparseTable<int> st1 = {1, 2};
    SparseTable<int> st2 = {3, 4};

    st1.swap(st2);
    st1.swap(st2);

    EXPECT_EQ(st1.size(), 2);
    EXPECT_EQ(st1.at(0), 1);
    EXPECT_EQ(st1.at(1), 2);

    EXPECT_EQ(st2.size(), 2);
    EXPECT_EQ(st2.at(0), 3);
    EXPECT_EQ(st2.at(1), 4);
}

// ══════════════════════════════════════════════════════�?//  Type Compatibility Tests
// ══════════════════════════════════════════════════════�?
/**
 * @brief Test SparseTable with double type
 * @details Verifies that the sparse table works correctly with
 *          double-precision floating-point values.
 */
TEST_F(SparseTableTest, DoubleType_Correctness)
{
    const SparseTable<double> st = {3.5, 1.2, 7.8, 0.9, 5.4};

    EXPECT_DOUBLE_EQ(st.query(0, 5), 0.9);
    EXPECT_DOUBLE_EQ(st.query(0, 3), 1.2);
    EXPECT_DOUBLE_EQ(st.query(1, 4), 0.9);
    EXPECT_DOUBLE_EQ(st.query(2, 4), 0.9);
    EXPECT_DOUBLE_EQ(st.query(4, 5), 5.4);
}

/**
 * @brief Test SparseTable with long long type
 * @details Verifies that the sparse table works correctly with large
 *          64-bit integer values.
 */
TEST_F(SparseTableTest, LongLongType_Correctness)
{
    const SparseTable<long long> st = {
        10000000000LL,
        5000000000LL,
        30000000000LL,
        2000000000LL,
    };

    EXPECT_EQ(st.query(0, 4), 2000000000LL);
    EXPECT_EQ(st.query(0, 2), 5000000000LL);
    EXPECT_EQ(st.query(1, 3), 5000000000LL);
    EXPECT_EQ(st.query(2, 4), 2000000000LL);
}

/**
 * @brief Test SparseTable with unsigned int type
 * @details Verifies that the sparse table works correctly with
 *          unsigned integer types.
 */
TEST_F(SparseTableTest, UnsignedType_Correctness)
{
    const SparseTable<unsigned int> st = {42u, 17u, 88u, 5u, 33u};

    EXPECT_EQ(st.query(0, 5), 5u);
    EXPECT_EQ(st.query(0, 3), 17u);
    EXPECT_EQ(st.query(3, 5), 5u);
    EXPECT_EQ(st.query(2, 4), 5u);
}

// ══════════════════════════════════════════════════════�?//  Boundary Size Tests
// ══════════════════════════════════════════════════════�?
/**
 * @brief Test SparseTable with a single element (n = 1)
 * @details The smallest non-empty sparse table; exercises the single-row
 *          code path throughout.
 */
TEST_F(SparseTableTest, N_SingleElement)
{
    const SparseTable<int> st = {42};

    EXPECT_EQ(st.size(), 1);
    EXPECT_FALSE(st.empty());
    EXPECT_EQ(st.at(0), 42);
    EXPECT_EQ(st.query(0, 1), 42);
}

/**
 * @brief Test SparseTable with two elements (n = 2)
 * @details The smallest table where the merge step actually combines
 *          two entries.
 */
TEST_F(SparseTableTest, N_TwoElements)
{
    const SparseTable<int, MaxOp> st = {10, 20};

    EXPECT_EQ(st.size(), 2);
    EXPECT_EQ(st.at(0), 10);
    EXPECT_EQ(st.at(1), 20);
    EXPECT_EQ(st.query(0, 1), 10);
    EXPECT_EQ(st.query(1, 2), 20);
    EXPECT_EQ(st.query(0, 2), 20);
}

/**
 * @brief Test SparseTable with n being a non-power-of-two (n = 17)
 * @details Verifies correctness when the array size is not a power of two,
 *          which creates an uneven last level in the sparse table.
 */
TEST_F(SparseTableTest, N_NonPowerOfTwo)
{
    // NOLINTBEGIN(cert-msc32-c,cert-msc51-cpp)
    std::mt19937 rng(42);
    // NOLINTEND(cert-msc32-c,cert-msc51-cpp)

    constexpr size_t kSize   = 17;
    constexpr size_t kTrials = 500;

    std::vector<int> data(kSize);
    for (size_t i = 0; i < kSize; ++i)
    {
        data[i] = static_cast<int>(rng() % 10000);  // NOLINT
    }

    const SparseTable<int> st(data.begin(), data.end());

    for (size_t trial = 0; trial < kTrials; ++trial)
    {
        size_t l = static_cast<size_t>(rng()) % kSize;
        size_t r = l + 1 + (static_cast<size_t>(rng()) % (kSize - l));

        auto expected = *std::min_element(data.begin() + l,
                                          data.begin() + r);
        EXPECT_EQ(st.query(l, r), expected)
            << "Mismatch for query(" << l << ", " << r << ") "
            << "with n = " << kSize;
    }
}

/**
 * @brief Test SparseTable with n being a power of two (n = 16)
 * @details A power-of-two size exercises the case where every level is
 *          completely filled.
 */
TEST_F(SparseTableTest, N_PowerOfTwo)
{
    // NOLINTBEGIN(cert-msc32-c,cert-msc51-cpp)
    std::mt19937 rng(42);
    // NOLINTEND(cert-msc32-c,cert-msc51-cpp)

    constexpr size_t kSize   = 16;
    constexpr size_t kTrials = 500;

    std::vector<int> data(kSize);
    for (size_t i = 0; i < kSize; ++i)
    {
        data[i] = static_cast<int>(rng() % 10000);  // NOLINT
    }

    const SparseTable<int> st(data.begin(), data.end());

    for (size_t trial = 0; trial < kTrials; ++trial)
    {
        size_t l = static_cast<size_t>(rng()) % kSize;
        size_t r = l + 1 + (static_cast<size_t>(rng()) % (kSize - l));

        auto expected = *std::min_element(data.begin() + l,
                                          data.begin() + r);
        EXPECT_EQ(st.query(l, r), expected)
            << "Mismatch for query(" << l << ", " << r << ") "
            << "with n = " << kSize;
    }
}
