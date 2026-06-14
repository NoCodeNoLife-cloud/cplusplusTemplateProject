/**
 * @file BoundingBox.hpp
 * @brief N-dimensional axis-aligned bounding box (AABB)
 * @details A generic N-dimensional axis-aligned bounding box template
 *          parameterised on coordinate type and dimensionality.  Serves as
 *          the fundamental spatial primitive for R-Tree and related spatial
 *          indexing structures.
 *
 * All geometric predicates and metrics are constexpr-compatible when the
 * coordinate type is a literal type.
 *
 * @par Point Type Requirements
 * The @c contains(point), @c minDistance(point), and @c minDistSquared(point)
 * methods accept any point-like type supporting @c operator[] for
 * coordinate access, e.g. @c std::array&lt;double,N&gt;.
 *
 * @par Thread Safety
 * This class is const-only thread-safe (all members are value types).
 *
 * @tparam CoordT  Coordinate value type (default @c double).
 * @tparam Dims    Number of spatial dimensions (default 2).
 */

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <type_traits>

namespace common::data_structure::spatial
{
    // ── Helper: subscript-detection concept (iterator constructor) ──

    /// @brief Checks whether a type supports @c operator[] with a
    ///        @c size_t index.
    /// @tparam T The type to test.
    template <typename T>
    concept Subscriptable = requires(T t, size_t i)
    {
        t[i];
    };

    // ═══════════════════════════════════════════════════════════════════════
    //  BoundingBox
    // ═══════════════════════════════════════════════════════════════════════

    /// @brief N-dimensional axis-aligned bounding box.
    ///
    /// Represents an axis-aligned hyper-rectangle defined by a minimum
    /// corner and a maximum corner.  Supports intersection tests, inclusion
    /// tests, area/margin computation, enlargement, and distance queries.
    ///
    /// @par Usage Example
    /// @code
    ///   // 2-D bounding box with double coordinates
    ///   BoundingBox<> box({0.0, 0.0}, {1.0, 1.0});
    ///
    ///   // 3-D integer bounding box
    ///   BoundingBox<int, 3> ibox({0, 0, 0}, {10, 10, 10});
    ///
    ///   // Enclosing box for a set of points
    ///   std::vector<std::array<double, 2>> pts = {{1,2}, {3,4}, {5,6}};
    ///   BoundingBox<> rangeBox(pts.begin(), pts.end());
    /// @endcode
    ///
    /// @tparam CoordT  Coordinate type (must be arithmetic).
    /// @tparam Dims    Number of dimensions (must be &ge; 1).
    template <typename CoordT = double, size_t Dims = 2>
    class BoundingBox
    {
        static_assert(Dims > 0, "BoundingBox requires at least 1 dimension");

    public:
        /// Coordinate value type.
        using coordinate_type = CoordT;

        /// Compile-time dimensionality.
        static constexpr size_t kDimensions = Dims;

        // ── Constructors ─────────────────────────────────────────────────

        /// @brief Default constructor — zero-initialises both corners.
        BoundingBox() = default;

        /// @brief Constructs a box from explicit minimum and maximum corners.
        ///
        /// The caller is responsible for ensuring @c min[d] &le; @c max[d]
        /// for all dimensions @c d.
        ///
        /// @param min Minimum corner coordinates.
        /// @param max Maximum corner coordinates.
        constexpr BoundingBox(const std::array<CoordT, Dims>& min,
                              const std::array<CoordT, Dims>& max) noexcept
            : min_(min)
            , max_(max)
        {}

        /// @brief Constructs a degenerate box (single point).
        ///
        /// Both corners are set to the same point, yielding a zero-volume
        /// bounding box.
        ///
        /// @param point The point that the box collapses to.
        explicit constexpr BoundingBox(
            const std::array<CoordT, Dims>& point) noexcept
            : min_(point)
            , max_(point)
        {}

        /// @brief Constructs the minimal enclosing box for a range of points.
        ///
        /// Iterates over @p [first, last) and computes the axis-aligned
        /// bounding box that encloses every point.  If the range is empty
        /// (@c first == @c last) the box is zero-initialised.
        ///
        /// @tparam Iter  Forward iterator whose value type supports
        ///               @c operator[] for coordinate access.
        /// @param  first Beginning of the point range.
        /// @param  last  End of the point range.
        template <typename Iter>
            requires Subscriptable<std::iter_value_t<Iter>>
        constexpr BoundingBox(Iter first, Iter last) noexcept
        {
            if (first == last)
            {
                min_ = {};
                max_ = {};
                return;
            }

            for (size_t d = 0; d < Dims; ++d)
            {
                min_[d] = (*first)[d];
                max_[d] = (*first)[d];
            }
            ++first;

            while (first != last)
            {
                for (size_t d = 0; d < Dims; ++d)
                {
                    if ((*first)[d] < min_[d])
                    {
                        min_[d] = (*first)[d];
                    }
                    if ((*first)[d] > max_[d])
                    {
                        max_[d] = (*first)[d];
                    }
                }
                ++first;
            }
        }

        // ── Accessors ────────────────────────────────────────────────────

        /// @brief Returns a const reference to the minimum corner.
        /// @return The minimum corner array.
        [[nodiscard]] constexpr auto min() const noexcept
            -> const std::array<CoordT, Dims>&
        {
            return min_;
        }

        /// @brief Returns a const reference to the maximum corner.
        /// @return The maximum corner array.
        [[nodiscard]] constexpr auto max() const noexcept
            -> const std::array<CoordT, Dims>&
        {
            return max_;
        }

        /// @brief Returns the minimum coordinate for a single dimension.
        /// @param dim Dimension index (0-based).
        /// @return The minimum coordinate along dimension @p dim.
        [[nodiscard]] constexpr auto min(size_t dim) const noexcept -> CoordT
        {
            return min_[dim];
        }

        /// @brief Returns the maximum coordinate for a single dimension.
        /// @param dim Dimension index (0-based).
        /// @return The maximum coordinate along dimension @p dim.
        [[nodiscard]] constexpr auto max(size_t dim) const noexcept -> CoordT
        {
            return max_[dim];
        }

        // ── Geometric Predicates ────────────────────────────────────────

        /// @brief Tests whether a point lies inside (or on) this box.
        ///
        /// Returns @c true iff for every dimension @c d:
        /// @c min_[d] &le; point[d] &le; max_[d].
        ///
        /// @tparam PointT  Point type supporting @c operator[].
        /// @param  point   The query point.
        /// @return @c true if the point is inside the box.
        template <typename PointT>
        [[nodiscard]] constexpr auto contains(const PointT& point) const noexcept
            -> bool
        {
            for (size_t d = 0; d < Dims; ++d)
            {
                if (point[d] < min_[d] || point[d] > max_[d])
                {
                    return false;
                }
            }
            return true;
        }

        /// @brief Tests whether this box intersects another box.
        ///
        /// Returns @c true iff the boxes overlap in every dimension:
        /// @c min_[d] &le; other.max_[d] && @c max_[d] &ge; other.min_[d].
        ///
        /// @param  other The other bounding box.
        /// @return @c true if the boxes intersect.
        [[nodiscard]] constexpr auto intersects(
            const BoundingBox& other) const noexcept -> bool
        {
            for (size_t d = 0; d < Dims; ++d)
            {
                if (min_[d] > other.max_[d] || max_[d] < other.min_[d])
                {
                    return false;
                }
            }
            return true;
        }

        /// @brief Tests whether this box fully contains another box.
        ///
        /// Returns @c true iff for every dimension @c d:
        /// @c min_[d] &le; other.min_[d] && @c other.max_[d] &le; max_[d].
        ///
        /// @param  other The other bounding box.
        /// @return @c true if @p other is entirely inside this box.
        [[nodiscard]] constexpr auto contains(
            const BoundingBox& other) const noexcept -> bool
        {
            for (size_t d = 0; d < Dims; ++d)
            {
                if (other.min_[d] < min_[d] || other.max_[d] > max_[d])
                {
                    return false;
                }
            }
            return true;
        }

        // ── Geometric Metrics ────────────────────────────────────────────

        /// @brief Computes the volume (area) of this box.
        ///
        /// The area is the product of the extents across all dimensions:
        /// @f$ \prod_{d=0}^{Dims-1} (max_d - min_d) @f$.
        ///
        /// @return The box volume / area.
        [[nodiscard]] constexpr auto area() const noexcept -> CoordT
        {
            auto result = max_[0] - min_[0];
            for (size_t d = 1; d < Dims; ++d)
            {
                result *= (max_[d] - min_[d]);
            }
            return result;
        }

        /// @brief Computes the sum of side lengths (margin).
        ///
        /// The margin is the sum of extents across all dimensions:
        /// @f$ \sum_{d=0}^{Dims-1} (max_d - min_d) @f$.
        /// Used by R*-Tree's ChooseSplitAxis heuristic.
        ///
        /// @return The sum of side lengths.
        [[nodiscard]] constexpr auto margin() const noexcept -> CoordT
        {
            auto result = max_[0] - min_[0];
            for (size_t d = 1; d < Dims; ++d)
            {
                result += (max_[d] - min_[d]);
            }
            return result;
        }

        // ── Mutation ─────────────────────────────────────────────────────

        /// @brief Enlarges this box to also enclose @p other.
        ///
        /// For each dimension, @c min_[d] = min(min_[d], other.min_[d])
        /// and @c max_[d] = max(max_[d], other.max_[d]).
        ///
        /// @param other The box to incorporate.
        /// @return Reference to this box (after enlargement).
        constexpr auto enlarge(const BoundingBox& other) noexcept
            -> BoundingBox&
        {
            for (size_t d = 0; d < Dims; ++d)
            {
                if (other.min_[d] < min_[d])
                {
                    min_[d] = other.min_[d];
                }
                if (other.max_[d] > max_[d])
                {
                    max_[d] = other.max_[d];
                }
            }
            return *this;
        }

        /// @brief Returns the minimal box that encloses both this and
        ///        @p other (non-mutating).
        ///
        /// @param other The other bounding box.
        /// @return The combined bounding box.
        [[nodiscard]] constexpr auto combined(
            const BoundingBox& other) const noexcept -> BoundingBox
        {
            auto result = *this;
            result.enlarge(other);
            return result;
        }

        /// @brief Returns the area of the combined box (this + other).
        ///
        /// Equivalent to @c this->combined(other).area() but expressed as a
        /// single expression for convenience in spatial index heuristics.
        ///
        /// @param other The other bounding box.
        /// @return The area of the combined bounding box.
        [[nodiscard]] constexpr auto combinedArea(
            const BoundingBox& other) const noexcept -> CoordT
        {
            return combined(other).area();
        }

        /// @brief Returns the increase in area when @p other is added.
        ///
        /// Equivalent to @c combinedArea(other) - area().  Used by R-Tree
        /// insertion heuristics to choose the least-enlargement node.
        ///
        /// @param other The other bounding box.
        /// @return The area enlargement.
        [[nodiscard]] constexpr auto enlargement(
            const BoundingBox& other) const noexcept -> CoordT
        {
            return combinedArea(other) - area();
        }

        // ── Distance ─────────────────────────────────────────────────────

        /// @brief Computes the squared minimum Euclidean distance from a
        ///        point to this box.
        ///
        /// If the point is inside the box the result is 0.  Otherwise, for
        /// each dimension the contribution is:
        ///   - @c (min_[d] - point[d])^2 if @c point[d] &lt; min_[d],
        ///   - @c (point[d] - max_[d])^2 if @c point[d] &gt; max_[d],
        ///   - 0 otherwise.
        ///
        /// @tparam PointT  Point type supporting @c operator[].
        /// @param  point   The query point.
        /// @return Squared Euclidean distance (always non-negative).
        template <typename PointT>
        [[nodiscard]] constexpr auto minDistSquared(
            const PointT& point) const noexcept -> double
        {
            double result = 0.0;
            for (size_t d = 0; d < Dims; ++d)
            {
                if (point[d] < min_[d])
                {
                    const auto diff = static_cast<double>(min_[d] - point[d]);
                    result += diff * diff;
                }
                else if (point[d] > max_[d])
                {
                    const auto diff = static_cast<double>(point[d] - max_[d]);
                    result += diff * diff;
                }
            }
            return result;
        }

        /// @brief Computes the minimum Euclidean distance from a point to
        ///        this box.
        ///
        /// @copydetails minDistSquared
        ///
        /// @tparam PointT  Point type supporting @c operator[].
        /// @param  point   The query point.
        /// @return Euclidean distance (always non-negative).
        template <typename PointT>
        [[nodiscard]] auto minDistance(const PointT& point) const noexcept
            -> double
        {
            return std::sqrt(minDistSquared(point));
        }

        /// @brief Returns the geometric centre of this box.
        ///
        /// For each dimension: @c (min_[d] + max_[d]) / CoordT{2}.
        ///
        /// @return Array containing the centre coordinates.
        [[nodiscard]] constexpr auto center() const noexcept
            -> std::array<CoordT, Dims>
        {
            std::array<CoordT, Dims> c{};
            for (size_t d = 0; d < Dims; ++d)
            {
                c[d] = (min_[d] + max_[d]) / CoordT{2};
            }
            return c;
        }

        // ── Comparison ───────────────────────────────────────────────────

        /// @brief Equality comparison (compares min and max arrays).
        /// @param other The other bounding box.
        /// @return @c true if both corners are element-wise equal.
        [[nodiscard]] constexpr auto operator==(
            const BoundingBox& other) const noexcept -> bool = default;

    private:
        std::array<CoordT, Dims> min_{};   ///< Minimum corner coordinates
        std::array<CoordT, Dims> max_{};   ///< Maximum corner coordinates
    };

} // namespace common::data_structure::spatial
