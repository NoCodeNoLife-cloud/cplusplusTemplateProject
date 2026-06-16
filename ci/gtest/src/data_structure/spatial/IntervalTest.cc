/**
 * @file IntervalTest.cc
 * @brief Unit tests for the Interval<ValueT> class
 * @details Tests cover default construction, parameterized construction,
 *          accessors, overlap detection, point containment, interval
 *          containment, emptiness checks, and comparison operators for
 *          multiple value types (int, double, float).
 */

#include <gtest/gtest.h>

#include "data_structure/spatial/Interval.hpp"

using namespace common::data_structure::spatial;

// ══════════════════════════════════════════════════════════════════════════
//  Type aliases for common Interval variants used throughout the tests
// ══════════════════════════════════════════════════════════════════════════

/// Integer interval.
using IInt = Interval<int>;

/// Double-precision floating-point interval.
using IDouble = Interval<double>;

/// Single-precision floating-point interval.
using IFloat = Interval<float>;

// ══════════════════════════════════════════════════════════════════════════
//  Test fixture
// ══════════════════════════════════════════════════════════════════════════

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

// ══════════════════════════════════════════════════════════════════════════
//  1. Default construction
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief A default-constructed integer interval is empty.
 * @details Verifies that Interval<int>() reports empty() == true and that
 *          both endpoints are value-initialised to zero.
 */
TEST_F(IntervalTest, DefaultConstruct_Int_IsEmpty)
{
    const IInt iv;
    EXPECT_TRUE(iv.empty());
    EXPECT_EQ(iv.low(), 0);
    EXPECT_EQ(iv.high(), 0);
}

/**
 * @brief A default-constructed double interval is empty.
 * @details Verifies that Interval<double>() reports empty() == true and
 *          both endpoints are value-initialised to 0.0.
 */
TEST_F(IntervalTest, DefaultConstruct_Double_IsEmpty)
{
    const IDouble iv;
    EXPECT_TRUE(iv.empty());
    EXPECT_EQ(iv.low(), 0.0);
    EXPECT_EQ(iv.high(), 0.0);
}

// ══════════════════════════════════════════════════════════════════════════
//  2. Parameterized construction
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief A normal [low, high) interval with low < high is non-empty.
 * @details Verifies that constructing with distinct endpoints produces a
 *          non-empty interval with the correct endpoint values.
 */
TEST_F(IntervalTest, Construct_NormalInterval_NonEmpty)
{
    const IInt iv(0, 10);
    EXPECT_FALSE(iv.empty());
    EXPECT_EQ(iv.low(), 0);
    EXPECT_EQ(iv.high(), 10);
}

/**
 * @brief A zero-length interval [n, n) is empty.
 * @details Verifies that an interval with identical low and high endpoints
 *          reports empty() == true.
 */
TEST_F(IntervalTest, Construct_ZeroLength_IsEmpty)
{
    const IInt iv(5, 5);
    EXPECT_TRUE(iv.empty());
    EXPECT_EQ(iv.low(), 5);
    EXPECT_EQ(iv.high(), 5);
}

/**
 * @brief Construction with negative endpoints works correctly.
 * @details Verifies that intervals with negative values are properly
 *          constructed and report the correct endpoints.
 */
TEST_F(IntervalTest, Construct_NegativeValues_Works)
{
    const IInt iv(-5, -2);
    EXPECT_FALSE(iv.empty());
    EXPECT_EQ(iv.low(), -5);
    EXPECT_EQ(iv.high(), -2);
}

/**
 * @brief Construction with mixed negative and positive endpoints works.
 * @details Verifies that intervals spanning across zero are handled
 *          correctly.
 */
TEST_F(IntervalTest, Construct_CrossingZero_Works)
{
    const IInt iv(-3, 7);
    EXPECT_FALSE(iv.empty());
    EXPECT_EQ(iv.low(), -3);
    EXPECT_EQ(iv.high(), 7);
}

/**
 * @brief Construction with floating-point values works correctly.
 * @details Verifies that double-precision intervals are properly
 *          constructed and report the correct endpoints.
 */
TEST_F(IntervalTest, Construct_FloatingPoint_Works)
{
    const IDouble iv(1.5, 3.14);
    EXPECT_FALSE(iv.empty());
    EXPECT_EQ(iv.low(), 1.5);
    EXPECT_EQ(iv.high(), 3.14);
}

// ══════════════════════════════════════════════════════════════════════════
//  3. Accessors
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief low() and high() return the values passed to the constructor.
 * @details Verifies that both accessors return const references to the
 *          correct endpoint values for integer intervals.
 */
TEST_F(IntervalTest, Accessors_Int_ReturnCorrectValues)
{
    const IInt iv(3, 8);
    EXPECT_EQ(iv.low(), 3);
    EXPECT_EQ(iv.high(), 8);
}

/**
 * @brief Accessors work correctly for floating-point intervals.
 * @details Verifies that low() and high() return correct values for
 *          double-precision intervals.
 */
TEST_F(IntervalTest, Accessors_Double_ReturnCorrectValues)
{
    const IDouble iv(-1.5, 2.5);
    EXPECT_EQ(iv.low(), -1.5);
    EXPECT_EQ(iv.high(), 2.5);
}

/**
 * @brief Accessors are constexpr-compatible (compile-time verification).
 * @details Verifies that the accessors can be evaluated at compile time
 *          for a constexpr interval.
 */
TEST_F(IntervalTest, Accessors_Constexpr_Evaluable)
{
    constexpr IInt iv(10, 20);
    static_assert(iv.low() == 10);
    static_assert(iv.high() == 20);
}

// ══════════════════════════════════════════════════════════════════════════
//  4. overlaps()
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Completely disjoint intervals do not overlap.
 * @details [0, 5) and [10, 15) have no points in common.
 */
TEST_F(IntervalTest, Overlaps_Disjoint_ReturnsFalse)
{
    const IInt a(0, 5);
    const IInt b(10, 15);
    EXPECT_FALSE(a.overlaps(b));
    EXPECT_FALSE(b.overlaps(a));
}

/**
 * @brief Intervals that partially overlap report true.
 * @details [0, 5) and [3, 8) share the sub-interval [3, 5).
 */
TEST_F(IntervalTest, Overlaps_PartialOverlap_ReturnsTrue)
{
    const IInt a(0, 5);
    const IInt b(3, 8);
    EXPECT_TRUE(a.overlaps(b));
    EXPECT_TRUE(b.overlaps(a));
}

/**
 * @brief An interval fully contained within another still overlaps.
 * @details [0, 10) contains [2, 5); they share [2, 5).
 */
TEST_F(IntervalTest, Overlaps_OneContainsOther_ReturnsTrue)
{
    const IInt a(0, 10);
    const IInt b(2, 5);
    EXPECT_TRUE(a.overlaps(b));
    EXPECT_TRUE(b.overlaps(a));
}

/**
 * @brief Edge-touching intervals do NOT overlap (half-open semantics).
 * @details [0, 5) and [5, 10) touch at 5, but the half-open convention
 *          means 5 is excluded from [0, 5) and the formula
 *          !(high <= other.low || other.high <= low) evaluates to false.
 */
TEST_F(IntervalTest, Overlaps_EdgeTouching_ReturnsFalse)
{
    const IInt a(0, 5);
    const IInt b(5, 10);
    EXPECT_FALSE(a.overlaps(b));
    EXPECT_FALSE(b.overlaps(a));
}

/**
 * @brief Identical intervals overlap.
 * @details [0, 5) overlaps with itself.
 */
TEST_F(IntervalTest, Overlaps_Identical_ReturnsTrue)
{
    const IInt a(0, 5);
    const IInt b(0, 5);
    EXPECT_TRUE(a.overlaps(b));
    EXPECT_TRUE(b.overlaps(a));
}

/**
 * @brief Overlaps is commutative for all tested cases.
 * @details Verifies the symmetric property of the overlap relation.
 */
TEST_F(IntervalTest, Overlaps_Commutative_Symmetric)
{
    const IInt a(-3, 7);
    const IInt b(1, 12);
    EXPECT_TRUE(a.overlaps(b));
    EXPECT_TRUE(b.overlaps(a));
}

// ══════════════════════════════════════════════════════════════════════════
//  5. contains(point)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief A point at the low boundary is contained (inclusive).
 * @details [0, 5) contains 0 because low <= point.
 */
TEST_F(IntervalTest, ContainsPoint_AtLowBoundary_ReturnsTrue)
{
    const IInt iv(0, 5);
    EXPECT_TRUE(iv.contains(0));
}

/**
 * @brief A point at the high boundary is NOT contained (exclusive).
 * @details [0, 5) does not contain 5 because point < high is false.
 */
TEST_F(IntervalTest, ContainsPoint_AtHighBoundary_ReturnsFalse)
{
    const IInt iv(0, 5);
    EXPECT_FALSE(iv.contains(5));
}

/**
 * @brief A point strictly inside the interval is contained.
 * @details [0, 5) contains 3.
 */
TEST_F(IntervalTest, ContainsPoint_StrictlyInside_ReturnsTrue)
{
    const IInt iv(0, 5);
    EXPECT_TRUE(iv.contains(3));
}

/**
 * @brief A point strictly below the low boundary is not contained.
 * @details [0, 5) does not contain -1.
 */
TEST_F(IntervalTest, ContainsPoint_BelowLow_ReturnsFalse)
{
    const IInt iv(0, 5);
    EXPECT_FALSE(iv.contains(-1));
}

/**
 * @brief A point strictly above the high boundary is not contained.
 * @details [0, 5) does not contain 10.
 */
TEST_F(IntervalTest, ContainsPoint_AboveHigh_ReturnsFalse)
{
    const IInt iv(0, 5);
    EXPECT_FALSE(iv.contains(10));
}

/**
 * @brief An empty interval contains no point, even at its endpoints.
 * @details [5, 5).contains(5) → low <= 5 (true) && 5 < high (false) → false.
 */
TEST_F(IntervalTest, ContainsPoint_EmptyInterval_ReturnsFalse)
{
    const IInt iv(5, 5);
    EXPECT_FALSE(iv.contains(5));
    EXPECT_FALSE(iv.contains(4));
    EXPECT_FALSE(iv.contains(6));
}

// ══════════════════════════════════════════════════════════════════════════
//  6. contains(interval)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief A strictly smaller interval is fully contained.
 * @details [0, 10) contains [2, 7).
 */
TEST_F(IntervalTest, ContainsInterval_FullyContained_ReturnsTrue)
{
    const IInt outer(0, 10);
    const IInt inner(2, 7);
    EXPECT_TRUE(outer.contains(inner));
}

/**
 * @brief An interval extending below the outer low is not contained.
 * @details [0, 10) does not contain [-2, 5) because -2 < 0.
 */
TEST_F(IntervalTest, ContainsInterval_ExtendsBelowLow_ReturnsFalse)
{
    const IInt outer(0, 10);
    const IInt inner(-2, 5);
    EXPECT_FALSE(outer.contains(inner));
}

/**
 * @brief An interval extending beyond the outer high is not contained.
 * @details [0, 10) does not contain [5, 12) because 12 > 10.
 */
TEST_F(IntervalTest, ContainsInterval_ExtendsBeyondHigh_ReturnsFalse)
{
    const IInt outer(0, 10);
    const IInt inner(5, 12);
    EXPECT_FALSE(outer.contains(inner));
}

/**
 * @brief An identical interval is contained.
 * @details [0, 10) contains itself.
 */
TEST_F(IntervalTest, ContainsInterval_Identical_ReturnsTrue)
{
    const IInt iv(0, 10);
    EXPECT_TRUE(iv.contains(iv));
}

/**
 * @brief An empty interval is contained within any non-empty interval.
 * @details [5, 5) is empty; [0, 10).contains([5, 5)) is true because
 *          0 <= 5 && 5 <= 10.
 */
TEST_F(IntervalTest, ContainsInterval_EmptyWithinNonEmpty_ReturnsTrue)
{
    const IInt outer(0, 10);
    const IInt empty(5, 5);
    EXPECT_TRUE(outer.contains(empty));
}

/**
 * @brief An empty interval at the outer boundary is contained.
 * @details [0, 10) contains [0, 0) (empty at low boundary) and
 *          [10, 10) (empty at high boundary).
 */
TEST_F(IntervalTest, ContainsInterval_EmptyAtBoundary_ReturnsTrue)
{
    const IInt outer(0, 10);
    const IInt emptyLow(0, 0);
    const IInt emptyHigh(10, 10);
    EXPECT_TRUE(outer.contains(emptyLow));
    EXPECT_TRUE(outer.contains(emptyHigh));
}

/**
 * @brief A larger interval is not contained in a smaller one.
 * @details [2, 5) does not contain [0, 10).
 */
TEST_F(IntervalTest, ContainsInterval_SmallerDoesNotContainLarger)
{
    const IInt small(2, 5);
    const IInt large(0, 10);
    EXPECT_FALSE(small.contains(large));
}

// ══════════════════════════════════════════════════════════════════════════
//  7. empty()
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief A default-constructed interval is empty.
 * @details Verifies that an Interval constructed with the default
 *          constructor reports empty() == true.
 */
TEST_F(IntervalTest, Empty_DefaultConstructed_ReturnsTrue)
{
    const IInt iv;
    EXPECT_TRUE(iv.empty());
}

/**
 * @brief A zero-length interval [n, n) is empty.
 * @details Verifies that constructing an interval with low == high results
 *          in an empty interval.
 */
TEST_F(IntervalTest, Empty_ZeroLength_ReturnsTrue)
{
    const IInt iv(7, 7);
    EXPECT_TRUE(iv.empty());
}

/**
 * @brief A non-zero-length interval [n, m) with n < m is non-empty.
 * @details Verifies that a properly constructed interval with distinct
 *          endpoints is not empty.
 */
TEST_F(IntervalTest, Empty_NonZeroLength_ReturnsFalse)
{
    const IInt iv(3, 8);
    EXPECT_FALSE(iv.empty());
}

/**
 * @brief An interval with negative values and distinct endpoints is
 *        non-empty.
 * @details Verifies that empty() works correctly for negative-valued
 *          intervals.
 */
TEST_F(IntervalTest, Empty_NegativeValues_ReturnsFalse)
{
    const IInt iv(-10, -5);
    EXPECT_FALSE(iv.empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  8. Comparison operators
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Identical intervals compare equal.
 * @details Two intervals with the same low and high are equal.
 */
TEST_F(IntervalTest, Compare_Identical_AreEqual)
{
    const IInt a(2, 7);
    const IInt b(2, 7);
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(b == a);
}

/**
 * @brief Intervals with different low values are not equal.
 * @details [2, 7) != [3, 7) because low differs.
 */
TEST_F(IntervalTest, Compare_DifferentLow_NotEqual)
{
    const IInt a(2, 7);
    const IInt b(3, 7);
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

/**
 * @brief Intervals with different high values are not equal.
 * @details [2, 7) != [2, 8) because high differs.
 */
TEST_F(IntervalTest, Compare_DifferentHigh_NotEqual)
{
    const IInt a(2, 7);
    const IInt b(2, 8);
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

/**
 * @brief operator<=> orders intervals lexicographically by low, then high.
 * @details [1, 5) < [2, 3) because low 1 < low 2.
 */
TEST_F(IntervalTest, Compare_Spaceship_LowDominates)
{
    const IInt a(1, 5);
    const IInt b(2, 3);
    EXPECT_EQ(a <=> b, std::partial_ordering::less);
    EXPECT_EQ(b <=> a, std::partial_ordering::greater);
}

/**
 * @brief When low is equal, high determines the ordering.
 * @details [1, 3) < [1, 7) because low is equal and high 3 < high 7.
 */
TEST_F(IntervalTest, Compare_Spaceship_HighTiebreaker)
{
    const IInt a(1, 3);
    const IInt b(1, 7);
    EXPECT_EQ(a <=> b, std::partial_ordering::less);
    EXPECT_EQ(b <=> a, std::partial_ordering::greater);
}

/**
 * @brief Identical intervals are equivalent under <=>.
 * @details [2, 7) <=> [2, 7) is equivalent.
 */
TEST_F(IntervalTest, Compare_Spaceship_Identical_Equivalent)
{
    const IInt a(2, 7);
    const IInt b(2, 7);
    EXPECT_EQ(a <=> b, std::partial_ordering::equivalent);
    EXPECT_EQ(b <=> a, std::partial_ordering::equivalent);
}

// ══════════════════════════════════════════════════════════════════════════
//  9. Template variants — float
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Interval<float> default construction.
 * @details Verifies that a default-constructed Interval<float> is empty
 *          with endpoints initialised to 0.0f.
 */
TEST_F(IntervalTest, Float_DefaultConstruct_IsEmpty)
{
    const IFloat iv;
    EXPECT_TRUE(iv.empty());
    EXPECT_EQ(iv.low(), 0.0f);
    EXPECT_EQ(iv.high(), 0.0f);
}

/**
 * @brief Interval<float> parameterized construction.
 * @details Verifies that a normal Interval<float> is properly constructed
 *          and non-empty.
 */
TEST_F(IntervalTest, Float_Construct_Works)
{
    const IFloat iv(1.5f, 3.0f);
    EXPECT_FALSE(iv.empty());
    EXPECT_EQ(iv.low(), 1.5f);
    EXPECT_EQ(iv.high(), 3.0f);
}

/**
 * @brief Interval<float> overlap detection.
 * @details Verifies that the overlap predicate works correctly for
 *          single-precision floating-point intervals.
 */
TEST_F(IntervalTest, Float_Overlaps_Works)
{
    const IFloat a(0.0f, 5.0f);
    const IFloat b(3.0f, 8.0f);
    EXPECT_TRUE(a.overlaps(b));

    const IFloat c(10.0f, 15.0f);
    EXPECT_FALSE(a.overlaps(c));
}

/**
 * @brief Interval<float> point containment.
 * @details Verifies that the point containment predicate works correctly
 *          for single-precision floating-point intervals, including
 *          the half-open boundary semantics.
 */
TEST_F(IntervalTest, Float_ContainsPoint_Works)
{
    const IFloat iv(0.0f, 5.0f);
    EXPECT_TRUE(iv.contains(0.0f));       // low boundary inclusive
    EXPECT_FALSE(iv.contains(5.0f));      // high boundary exclusive
    EXPECT_TRUE(iv.contains(2.5f));       // strictly inside
    EXPECT_FALSE(iv.contains(-1.0f));     // below low
}

/**
 * @brief Interval<float> interval containment.
 * @details Verifies that the interval containment predicate works
 *          correctly for single-precision floating-point intervals.
 */
TEST_F(IntervalTest, Float_ContainsInterval_Works)
{
    const IFloat outer(0.0f, 10.0f);
    const IFloat inner(2.0f, 7.0f);
    EXPECT_TRUE(outer.contains(inner));

    const IFloat below(-1.0f, 5.0f);
    EXPECT_FALSE(outer.contains(below));

    const IFloat above(5.0f, 12.0f);
    EXPECT_FALSE(outer.contains(above));
}

/**
 * @brief Interval<float> comparison.
 * @details Verifies that comparison operators work correctly for
 *          single-precision floating-point intervals.
 */
TEST_F(IntervalTest, Float_Compare_Works)
{
    const IFloat a(1.0f, 5.0f);
    const IFloat b(1.0f, 5.0f);
    const IFloat c(2.0f, 3.0f);

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_EQ(a <=> c, std::partial_ordering::less);
    EXPECT_EQ(c <=> a, std::partial_ordering::greater);
}
