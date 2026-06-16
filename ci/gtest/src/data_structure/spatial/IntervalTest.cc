/**
 * @file IntervalTest.cc
 * @brief Unit tests for the Interval<ValueT> value type
 * @details Tests cover constructors, accessors, overlap detection,
 *          point containment, interval containment, empty-state queries,
 *          equality/ordering comparisons, constexpr evaluation, and
 *          type compatibility (int, double, unsigned).  All tests
 *          exercise the half-open [low, high) semantics.
 */

#include <gtest/gtest.h>

#include <limits>

#include "data_structure/spatial/Interval.hpp"

using namespace common::data_structure::spatial;

// ==================== Test Fixture ====================

class IntervalTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// ═══════════════════════════════════════════════════════
//  Constructor Tests
// ═══════════════════════════════════════════════════════

/**
 * @brief Test default constructor creates an empty interval
 * @details Verifies that a default-constructed Interval is empty,
 *          and that both endpoints are value-initialised to zero.
 */
TEST_F(IntervalTest, DefaultConstructor_EmptyInterval)
{
    const Interval<int> iv;
    EXPECT_TRUE(iv.empty());
    EXPECT_EQ(iv.low(), 0);
    EXPECT_EQ(iv.high(), 0);
}

/**
 * @brief Test value constructor with a normal range
 * @details Interval(3, 7) should have low=3, high=7 and be non-empty.
 */
TEST_F(IntervalTest, ValueConstructor_Normal)
{
    const Interval<int> iv(3, 7);
    EXPECT_EQ(iv.low(), 3);
    EXPECT_EQ(iv.high(), 7);
    EXPECT_FALSE(iv.empty());
}

/**
 * @brief Test value constructor where low == high (degenerate)
 * @details Interval(5, 5) is a zero-length interval and must be empty.
 */
TEST_F(IntervalTest, ValueConstructor_SinglePoint)
{
    const Interval<int> iv(5, 5);
    EXPECT_TRUE(iv.empty());
    EXPECT_EQ(iv.low(), 5);
    EXPECT_EQ(iv.high(), 5);
}

/**
 * @brief Test value constructor with negative values
 * @details Verifies that negative endpoints are stored correctly.
 */
TEST_F(IntervalTest, ValueConstructor_NegativeValues)
{
    const Interval<int> iv(-5, -1);
    EXPECT_EQ(iv.low(), -5);
    EXPECT_EQ(iv.high(), -1);
    EXPECT_FALSE(iv.empty());
}

// ═══════════════════════════════════════════════════════
//  Accessor Tests
// ═══════════════════════════════════════════════════════

/**
 * @brief Test low() and high() accessors
 * @details Verifies that the accessors return const references to the
 *          stored endpoints and are usable in const contexts.
 */
TEST_F(IntervalTest, LowHigh_Accessors)
{
    const Interval<int> iv(2, 8);
    EXPECT_EQ(iv.low(), 2);
    EXPECT_EQ(iv.high(), 8);

    // Verify they return const references (can bind to const ref).
    const auto& l = iv.low();
    const auto& h = iv.high();
    EXPECT_EQ(l, 2);
    EXPECT_EQ(h, 8);
}

// ═══════════════════════════════════════════════════════
//  Overlaps Tests
// ═══════════════════════════════════════════════════════

/**
 * @brief Test overlaps when one interval fully contains the query
 * @details [3, 7) completely contains [4, 6), so they overlap.
 */
TEST_F(IntervalTest, Overlaps_CompletelyOverlap)
{
    const Interval<int> a(3, 7);
    const Interval<int> b(4, 6);
    EXPECT_TRUE(a.overlaps(b));
    EXPECT_TRUE(b.overlaps(a));
}

/**
 * @brief Test overlaps when intervals partially intersect
 * @details [3, 7) and [5, 9) share [5, 7), so they overlap.
 */
TEST_F(IntervalTest, Overlaps_PartialOverlap)
{
    const Interval<int> a(3, 7);
    const Interval<int> b(5, 9);
    EXPECT_TRUE(a.overlaps(b));
    EXPECT_TRUE(b.overlaps(a));
}

/**
 * @brief Test that touching edges do NOT overlap (half-open semantics)
 * @details [3, 5) and [5, 8) meet at the single point 5, but 5 is
 *          excluded from [3, 5) (high bound) and included in [5, 8)
 *          (low bound).  Under half-open semantics the shared point
 *          does not belong to both intervals, so they do NOT overlap.
 */
TEST_F(IntervalTest, Overlaps_TouchingEdges)
{
    const Interval<int> a(3, 5);
    const Interval<int> b(5, 8);
    EXPECT_FALSE(a.overlaps(b));
    EXPECT_FALSE(b.overlaps(a));
}

/**
 * @brief Test disjoint intervals with a gap
 * @details [3, 5) and [7, 9) are separated by (5, 7), no overlap.
 */
TEST_F(IntervalTest, Overlaps_NoOverlap)
{
    const Interval<int> a(3, 5);
    const Interval<int> b(7, 9);
    EXPECT_FALSE(a.overlaps(b));
    EXPECT_FALSE(b.overlaps(a));
}

/**
 * @brief Test that an interval overlaps with itself
 * @details An interval always overlaps with an identical interval.
 */
TEST_F(IntervalTest, Overlaps_Identical)
{
    const Interval<int> a(3, 7);
    const Interval<int> b(3, 7);
    EXPECT_TRUE(a.overlaps(b));
    EXPECT_TRUE(b.overlaps(a));
}

/**
 * @brief Test overlaps when one interval contains the other
 * @details [2, 10) fully contains [4, 6), so they overlap.
 */
TEST_F(IntervalTest, Overlaps_OneContainsOther)
{
    const Interval<int> outer(2, 10);
    const Interval<int> inner(4, 6);
    EXPECT_TRUE(outer.overlaps(inner));
    EXPECT_TRUE(inner.overlaps(outer));
}

/**
 * @brief Test overlaps with empty intervals
 * @details An empty interval [5, 5) has zero width.  The current
 *          overlaps implementation does not short-circuit on empty
 *          intervals, so it may report results that depend solely
 *          on endpoint comparisons.  This test documents the
 *          observable behaviour.
 */
TEST_F(IntervalTest, Overlaps_EmptyInterval)
{
    const Interval<int> empty(5, 5);
    const Interval<int> other(3, 7);

    // [3, 7) vs [5, 5):  !(7 <= 5 || 5 <= 3) = !(false || false) = true
    EXPECT_TRUE(other.overlaps(empty));
    EXPECT_TRUE(empty.overlaps(other));

    // Two empty intervals at the same point:
    //   !(5 <= 5 || 5 <= 5) = !(true || true) = false
    const Interval<int> empty2(5, 5);
    EXPECT_FALSE(empty.overlaps(empty2));
}

// ═══════════════════════════════════════════════════════
//  Contains Point Tests
// ═══════════════════════════════════════════════════════

/**
 * @brief Test contains(point) for a point strictly inside
 * @details [3, 7) contains 5 because 3 <= 5 < 7.
 */
TEST_F(IntervalTest, ContainsPoint_Inside)
{
    const Interval<int> iv(3, 7);
    EXPECT_TRUE(iv.contains(5));
}

/**
 * @brief Test contains(point) at the lower bound (inclusive)
 * @details [3, 7) contains 3 because the lower bound is inclusive.
 */
TEST_F(IntervalTest, ContainsPoint_LowEdge)
{
    const Interval<int> iv(3, 7);
    EXPECT_TRUE(iv.contains(3));
}

/**
 * @brief Test contains(point) at the upper bound (exclusive)
 * @details [3, 7) does NOT contain 7 because the upper bound is exclusive.
 */
TEST_F(IntervalTest, ContainsPoint_HighEdge)
{
    const Interval<int> iv(3, 7);
    EXPECT_FALSE(iv.contains(7));
}

/**
 * @brief Test contains(point) for a point below the lower bound
 * @details [3, 7) does not contain 2.
 */
TEST_F(IntervalTest, ContainsPoint_BelowLow)
{
    const Interval<int> iv(3, 7);
    EXPECT_FALSE(iv.contains(2));
}

/**
 * @brief Test contains(point) for a point above the upper bound
 * @details [3, 7) does not contain 8.
 */
TEST_F(IntervalTest, ContainsPoint_AboveHigh)
{
    const Interval<int> iv(3, 7);
    EXPECT_FALSE(iv.contains(8));
}

/**
 * @brief Test contains(point) on an empty interval
 * @details An empty interval [5, 5) contains no points, including 5.
 */
TEST_F(IntervalTest, ContainsPoint_EmptyInterval)
{
    const Interval<int> empty(5, 5);
    EXPECT_FALSE(empty.contains(5));
    EXPECT_FALSE(empty.contains(0));
    EXPECT_FALSE(empty.contains(10));
}

// ═══════════════════════════════════════════════════════
//  Contains Interval Tests
// ═══════════════════════════════════════════════════════

/**
 * @brief Test contains(interval) when the argument is a strict subset
 * @details [2, 10) fully contains [4, 6).
 */
TEST_F(IntervalTest, ContainsInterval_Subset)
{
    const Interval<int> outer(2, 10);
    const Interval<int> inner(4, 6);
    EXPECT_TRUE(outer.contains(inner));
    EXPECT_FALSE(inner.contains(outer));
}

/**
 * @brief Test contains(interval) when the argument is identical
 * @details An interval always contains itself.
 */
TEST_F(IntervalTest, ContainsInterval_Exact)
{
    const Interval<int> a(3, 7);
    const Interval<int> b(3, 7);
    EXPECT_TRUE(a.contains(b));
    EXPECT_TRUE(b.contains(a));
}

/**
 * @brief Test contains(interval) when the argument overlaps but is not contained
 * @details [3, 7) does not fully contain [5, 9) because 9 > 7.
 */
TEST_F(IntervalTest, ContainsInterval_OverlapButNotContain)
{
    const Interval<int> a(3, 7);
    const Interval<int> b(5, 9);
    EXPECT_FALSE(a.contains(b));
    EXPECT_FALSE(b.contains(a));
}

/**
 * @brief Test contains(interval) for disjoint intervals
 * @details [3, 5) does not contain [7, 9) (no overlap at all).
 */
TEST_F(IntervalTest, ContainsInterval_Disjoint)
{
    const Interval<int> a(3, 5);
    const Interval<int> b(7, 9);
    EXPECT_FALSE(a.contains(b));
    EXPECT_FALSE(b.contains(a));
}

/**
 * @brief Test contains(interval) with empty sub-intervals
 * @details An empty interval is a subset of any interval whose span
 *          covers the degenerate point.
 */
TEST_F(IntervalTest, ContainsInterval_EmptySubset)
{
    const Interval<int> outer(2, 10);
    const Interval<int> empty(5, 5);
    EXPECT_TRUE(outer.contains(empty));

    // An empty interval contains itself
    EXPECT_TRUE(empty.contains(empty));

    // An empty interval does NOT contain a non-empty interval
    EXPECT_FALSE(empty.contains(outer));
}

// ═══════════════════════════════════════════════════════
//  Empty Tests
// ═══════════════════════════════════════════════════════

/**
 * @brief Test that a default-constructed interval is empty
 * @details Default constructor value-initialises both endpoints to zero,
 *          so low == high == 0 and empty() == true.
 */
TEST_F(IntervalTest, Empty_DefaultConstructed)
{
    const Interval<int> iv;
    EXPECT_TRUE(iv.empty());
}

/**
 * @brief Test that a single-point interval (low == high) is empty
 * @details Interval(5, 5) is degenerate with zero width.
 */
TEST_F(IntervalTest, Empty_SinglePointInterval)
{
    const Interval<int> iv(5, 5);
    EXPECT_TRUE(iv.empty());
}

/**
 * @brief Test that a normal interval is not empty
 * @details Interval(3, 7) has positive width.
 */
TEST_F(IntervalTest, Empty_NormalInterval)
{
    const Interval<int> iv(3, 7);
    EXPECT_FALSE(iv.empty());
}

// ═══════════════════════════════════════════════════════
//  Comparison Operator Tests
// ═══════════════════════════════════════════════════════

/**
 * @brief Test equality for identical intervals
 * @details Two intervals constructed with the same endpoints are equal.
 */
TEST_F(IntervalTest, Equality_Identical)
{
    const Interval<int> a(3, 7);
    const Interval<int> b(3, 7);
    EXPECT_TRUE(a == b);
}

/**
 * @brief Test equality for different intervals
 * @details Intervals with different endpoints are not equal.
 */
TEST_F(IntervalTest, Equality_Different)
{
    const Interval<int> a(3, 7);
    const Interval<int> b(4, 8);
    EXPECT_FALSE(a == b);
}

/**
 * @brief Test equality for two default-constructed (empty) intervals
 * @details All empty intervals with default values are equal.
 */
TEST_F(IntervalTest, Equality_Empty)
{
    const Interval<int> a;
    const Interval<int> b;
    EXPECT_TRUE(a == b);
}

/**
 * @brief Test less-than ordering by low bound
 * @details [3, 5) < [4, 6) because 3 < 4 (low differs).
 */
TEST_F(IntervalTest, LessThan_ByLow)
{
    const Interval<int> a(3, 5);
    const Interval<int> b(4, 6);
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

/**
 * @brief Test less-than ordering by high bound when low is equal
 * @details [3, 5) < [3, 7) because low is equal (3 == 3) and 5 < 7.
 */
TEST_F(IntervalTest, LessThan_SameLowByHigh)
{
    const Interval<int> a(3, 5);
    const Interval<int> b(3, 7);
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

/**
 * @brief Test greater-than derived from three-way comparison
 * @details [4, 6) > [3, 5) because 4 > 3.
 */
TEST_F(IntervalTest, GreaterThan_ByLow)
{
    const Interval<int> a(4, 6);
    const Interval<int> b(3, 5);
    EXPECT_TRUE(a > b);
    EXPECT_FALSE(b > a);
}

// ═══════════════════════════════════════════════════════
//  Constexpr Tests
// ═══════════════════════════════════════════════════════

/**
 * @brief Compile-time verification of all Interval methods
 * @details Uses static_assert to ensure every method is constexpr
 *          and produces correct results at compile time.
 */
TEST_F(IntervalTest, Constexpr_CompileTime)
{
    // Default construction
    static_assert(Interval<int>().empty());

    // Value construction and accessors
    constexpr auto kIv = Interval<int>(3, 7);
    static_assert(kIv.low() == 3);
    static_assert(kIv.high() == 7);
    static_assert(!kIv.empty());

    // Single-point (empty) interval
    constexpr auto kEmpty = Interval<int>(5, 5);
    static_assert(kEmpty.empty());

    // Overlaps
    constexpr auto kA = Interval<int>(3, 7);
    constexpr auto kB1 = Interval<int>(4, 6);   // completely inside
    constexpr auto kB2 = Interval<int>(5, 9);   // partially overlapping
    constexpr auto kB3 = Interval<int>(1, 3);   // [1,3) vs [3,7) touch at 3
    constexpr auto kB4 = Interval<int>(7, 9);   // [3,7) vs [7,9) touch at 7
    static_assert(kA.overlaps(kB1));
    static_assert(kA.overlaps(kB2));
    static_assert(kA.overlaps(kA));
    static_assert(!kA.overlaps(kB3));  // [3,7) vs [1,3) — touching edges
    static_assert(!kA.overlaps(kB4));  // [3,7) vs [7,9) — touching edges

    // Contains point
    static_assert(kA.contains(3));    // low edge (inclusive)
    static_assert(kA.contains(5));    // interior
    static_assert(!kA.contains(7));   // high edge (exclusive)
    static_assert(!kA.contains(2));   // below low
    static_assert(!kA.contains(8));   // above high

    // Contains interval
    constexpr auto kOuter = Interval<int>(2, 10);
    constexpr auto kInner = Interval<int>(4, 6);
    static_assert(kOuter.contains(kInner));
    static_assert(kOuter.contains(kOuter));
    static_assert(!kInner.contains(kOuter));
    static_assert(!kA.contains(kB2));  // overlap but not contained

    // Equality
    static_assert(kA == Interval<int>(3, 7));
    static_assert(!(kA == Interval<int>(3, 8)));

    // Ordering
    static_assert(Interval<int>(3, 5) < Interval<int>(4, 6));
    static_assert(Interval<int>(3, 5) < Interval<int>(3, 7));
    static_assert(Interval<int>(4, 6) > Interval<int>(3, 5));
}

// ═══════════════════════════════════════════════════════
//  Type Compatibility Tests
// ═══════════════════════════════════════════════════════

/**
 * @brief Test Interval<double> basic operations
 * @details Verifies that the Interval template works correctly with
 *          double-precision floating-point values.
 */
TEST_F(IntervalTest, DoubleType_BasicOperations)
{
    const Interval<double> iv(1.5, 3.14);
    EXPECT_DOUBLE_EQ(iv.low(), 1.5);
    EXPECT_DOUBLE_EQ(iv.high(), 3.14);
    EXPECT_FALSE(iv.empty());

    // Contains point (double)
    EXPECT_TRUE(iv.contains(1.5));    // low edge (inclusive)
    EXPECT_TRUE(iv.contains(2.0));    // interior
    EXPECT_FALSE(iv.contains(3.14));  // high edge (exclusive)
    EXPECT_FALSE(iv.contains(1.0));   // below low
    EXPECT_FALSE(iv.contains(4.0));   // above high

    // Overlaps
    const Interval<double> a(1.0, 5.0);
    const Interval<double> b(3.0, 7.0);
    EXPECT_TRUE(a.overlaps(b));
    EXPECT_TRUE(b.overlaps(a));

    const Interval<double> c(5.0, 5.0);
    EXPECT_TRUE(c.empty());

    const Interval<double> d(5.0, 8.0);
    // [1,5) and [5,8) touch at 5 — no overlap (half-open)
    EXPECT_FALSE(a.overlaps(d));
    EXPECT_FALSE(d.overlaps(a));

    // Contains interval
    const Interval<double> outer(0.0, 10.0);
    const Interval<double> inner(2.5, 7.5);
    EXPECT_TRUE(outer.contains(inner));
    EXPECT_FALSE(inner.contains(outer));

    // Comparison
    EXPECT_TRUE(Interval<double>(1.0, 3.0) == Interval<double>(1.0, 3.0));
    EXPECT_FALSE(Interval<double>(1.0, 3.0) == Interval<double>(1.0, 4.0));
    EXPECT_TRUE(Interval<double>(1.0, 3.0) < Interval<double>(2.0, 3.0));
}

/**
 * @brief Test Interval<unsigned int> basic operations
 * @details Verifies that the Interval template works correctly with
 *          unsigned integer types.
 */
TEST_F(IntervalTest, UnsignedType_BasicOperations)
{
    const Interval<unsigned int> iv(10u, 20u);
    EXPECT_EQ(iv.low(), 10u);
    EXPECT_EQ(iv.high(), 20u);
    EXPECT_FALSE(iv.empty());

    // Contains point
    EXPECT_TRUE(iv.contains(10u));   // low edge
    EXPECT_TRUE(iv.contains(15u));   // interior
    EXPECT_FALSE(iv.contains(20u));  // high edge
    EXPECT_FALSE(iv.contains(5u));   // below low
    EXPECT_FALSE(iv.contains(25u));  // above high

    // Overlaps
    EXPECT_TRUE(iv.overlaps(Interval<unsigned int>(15u, 25u)));
    EXPECT_FALSE(iv.overlaps(Interval<unsigned int>(20u, 30u)));  // edge touch
    EXPECT_FALSE(iv.overlaps(Interval<unsigned int>(25u, 30u)));  // disjoint

    // Contains interval
    EXPECT_TRUE(iv.contains(Interval<unsigned int>(12u, 18u)));
    EXPECT_TRUE(iv.contains(iv));
    EXPECT_FALSE(iv.contains(Interval<unsigned int>(12u, 22u)));

    // Comparison
    EXPECT_TRUE(Interval<unsigned int>(5u, 10u) < iv);
    EXPECT_TRUE(Interval<unsigned int>(10u, 15u) < Interval<unsigned int>(10u, 20u));
}

// ═══════════════════════════════════════════════════════
//  Edge Case Tests
// ═══════════════════════════════════════════════════════

/**
 * @brief Test Interval with large integer values at type limits
 * @details Verifies correctness when endpoints are near
 *          std::numeric_limits<int>::max()/min().
 */
TEST_F(IntervalTest, LargeValues_TypeLimits)
{
    constexpr auto kMin = std::numeric_limits<int>::min();
    constexpr auto kMax = std::numeric_limits<int>::max();

    const Interval<int> full(kMin, kMax);
    EXPECT_EQ(full.low(), kMin);
    EXPECT_EQ(full.high(), kMax);
    EXPECT_FALSE(full.empty());

    // Single-point at max
    const Interval<int> at_max(kMax, kMax);
    EXPECT_TRUE(at_max.empty());

    // Point containment at boundaries
    EXPECT_TRUE(full.contains(kMin));
    EXPECT_TRUE(full.contains(0));
    EXPECT_FALSE(full.contains(kMax));  // exclusive upper bound

    // Overlaps with another large interval
    const Interval<int> left(kMin, 0);
    const Interval<int> right(0, kMax);
    // [min, 0) and [0, max) touch at 0 — no overlap
    EXPECT_FALSE(left.overlaps(right));
    EXPECT_FALSE(right.overlaps(left));

    // Two intervals that span the full range
    EXPECT_TRUE(full.overlaps(left));
    EXPECT_TRUE(full.overlaps(right));
}

/**
 * @brief Test that default-constructed intervals of different types
 *        all satisfy the empty invariant
 */
TEST_F(IntervalTest, DefaultConstructed_AllTypesEmpty)
{
    const Interval<int> i;
    const Interval<double> d;
    const Interval<unsigned> u;
    EXPECT_TRUE(i.empty());
    EXPECT_TRUE(d.empty());
    EXPECT_TRUE(u.empty());
    EXPECT_EQ(i.low(), 0);
    EXPECT_DOUBLE_EQ(d.low(), 0.0);
    EXPECT_EQ(u.low(), 0u);
}
