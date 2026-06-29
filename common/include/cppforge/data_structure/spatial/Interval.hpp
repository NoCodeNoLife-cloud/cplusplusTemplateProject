/**
 * @file Interval.hpp
 * @brief 1-D half-open interval [low, high) value type
 * @details A generic one-dimensional half-open interval template
 *          parameterised on the value type.  Serves as the fundamental
 *          interval primitive for spatial indexing structures such as
 *          interval tree and related data structures.
 *
 * All predicates and queries are constexpr-compatible when the value
 * type is a literal type.
 *
 * @par Thread Safety
 * This class is const-only thread-safe (all members are value types).
 *
 * @par Semantics
 * The interval [low, high) contains a point @c p iff
 * @c low &le; p && p &lt; high.  An interval with @c low == @c high
 * is considered empty.
 *
 * @tparam ValueT  Value type (must satisfy std::regular and
 *                 std::totally_ordered).
 */

#pragma once

#include <cassert>
#include <compare>
#include <concepts>

namespace cppforge::data_structure::spatial
{
    // ══════════════════════════════════════════════════════════════════════�?    //  Interval
    // ══════════════════════════════════════════════════════════════════════�?
    /// @brief A 1-D half-open interval [low, high).
    ///
    /// Represents a contiguous range along the real line (or any totally
    /// ordered domain).  The half-open convention [low, high) means the
    /// point @c low is included while the point @c high is excluded.
    ///
    /// @par Usage Example
    /// @code
    ///   // Integer interval
    ///   Interval<int> iv(0, 10);
    ///
    ///   // Floating-point interval
    ///   Interval<double> fiv(1.5, 3.14);
    ///
    ///   // Empty interval (low == high)
    ///   Interval<int> empty(5, 5);
    ///   assert(empty.empty());
    ///
    ///   // Overlap and containment tests
    ///   Interval<int> a(0, 5), b(3, 8);
    ///   assert(a.overlaps(b));
    ///   assert(a.contains(2));
    ///   assert(a.contains(Interval<int>(1, 3)));
    /// @endcode
    ///
    /// @tparam ValueT  Value type (must satisfy @c std::regular and
    ///                 @c std::totally_ordered).
    template <typename ValueT>
        requires std::regular<ValueT> && std::totally_ordered<ValueT>
    class Interval
    {
    public:
        /// Value type alias.
        using value_type = ValueT;

        // ── Constructors ─────────────────────────────────────────────────

        /// @brief Default constructor -- creates a degenerate (empty)
        ///        interval with both endpoints value-initialised.
        Interval() = default;

        /// @brief Constructs a half-open interval [low, high).
        ///
        /// @pre @p low &le; @p high
        ///
        /// @param low  Lower bound (inclusive).
        /// @param high Upper bound (exclusive).
        constexpr Interval(const ValueT& low, const ValueT& high) noexcept
            : low_(low)
            , high_(high)
        {
            assert(low_ <= high_);
        }

        // ── Accessors ────────────────────────────────────────────────────

        /// @brief Returns a const reference to the lower bound (inclusive).
        /// @return The lower bound @c low.
        [[nodiscard]] constexpr auto low() const noexcept -> const ValueT&
        {
            return low_;
        }

        /// @brief Returns a const reference to the upper bound (exclusive).
        /// @return The upper bound @c high.
        [[nodiscard]] constexpr auto high() const noexcept -> const ValueT&
        {
            return high_;
        }

        // ── Predicates ───────────────────────────────────────────────────

        /// @brief Tests whether this interval overlaps with @p other.
        ///
        /// Two intervals overlap if they share at least one point:
        /// @code
        ///   !(high_ <= other.low_ || other.high_ <= low_)
        /// @endcode
        ///
        /// @param  other The other interval.
        /// @return @c true if the intervals overlap (including edge-touching
        ///         where one interval's @c high equals the other's @c low).
        [[nodiscard]] constexpr auto overlaps(
            const Interval& other) const noexcept -> bool
        {
            return !(high_ <= other.low_ || other.high_ <= low_);
        }

        /// @brief Tests whether a point lies inside this interval.
        ///
        /// Returns @c true iff @c low_ &le; @p point && @p point &lt; high_.
        ///
        /// @param  point The query point.
        /// @return @c true if the point is inside the interval.
        [[nodiscard]] constexpr auto contains(
            const ValueT& point) const noexcept -> bool
        {
            return low_ <= point && point < high_;
        }

        /// @brief Tests whether this interval fully contains @p other.
        ///
        /// Returns @c true iff @c low_ &le; other.low_ &&
        /// @c other.high_ &le; high_.
        ///
        /// @param  other The other interval.
        /// @return @c true if @p other is entirely inside this interval.
        [[nodiscard]] constexpr auto contains(
            const Interval& other) const noexcept -> bool
        {
            return low_ <= other.low_ && other.high_ <= high_;
        }

        /// @brief Checks whether this interval is empty.
        ///
        /// An interval is empty when @c low_ == @c high_.
        ///
        /// @return @c true if the interval has zero length (low == high).
        [[nodiscard]] constexpr auto empty() const noexcept -> bool
        {
            return low_ == high_;
        }

        // ── Comparison ───────────────────────────────────────────────────

        /// @brief Three-way comparison (lexicographic on low, then high).
        ///
        /// Intervals are ordered primarily by their lower bound and
        /// secondarily by their upper bound.
        ///
        /// @param other The other interval.
        /// @return @c std::partial_ordering reflecting the comparison result.
        [[nodiscard]] constexpr auto operator<=>(
            const Interval& other) const noexcept -> std::partial_ordering
        {
            if (auto cmp = low_ <=> other.low_; cmp != 0)
            {
                return cmp;
            }
            return high_ <=> other.high_;
        }

        /// @brief Equality comparison (member-wise).
        /// @param other The other interval.
        /// @return @c true if both endpoints are equal.
        [[nodiscard]] constexpr auto operator==(
            const Interval& other) const noexcept -> bool = default;

    private:
        ValueT low_{};   ///< Lower bound (inclusive)
        ValueT high_{};  ///< Upper bound (exclusive)
    };

} // namespace cppforge::data_structure::spatial
