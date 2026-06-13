/**
 * @file KDTree.hpp
 * @brief KD-Tree — K-dimensional spatial partitioning tree
 * @details A space-partitioning data structure for organising points in
 *          K-dimensional space.  Supports optimal median-based bulk
 *          construction, incremental insertion, nearest-neighbour (NN),
 *          K-nearest-neighbours (KNN), and radius-based range search.
 *
 * The tree alternates splitting dimensions at each level.  The bulk-build
 * selects the median along the current axis using @c std::nth_element,
 * yielding a balanced tree.  Nearest-neighbour search prunes branches whose
 * splitting plane is farther than the current best distance.
 *
 * @par Thread Safety
 * All public methods are thread-safe.  Concurrent access is serialised via
 * an internal @c std::mutex.
 *
 * @par Point Type Requirements
 * Any point type @c PointT can be used with a custom
 * @c KDTreeAccessor&lt;PointT, K&gt; specialisation.  The accessor must expose:
 * @code
 *   static double get(const PointT& point, size_t dim);
 * @endcode
 * The library ships with specialisations for:
 *   - @c std::array&lt;double, K&gt; (uses @c operator[])
 *   - @c geometry::Point2D (K=2)
 *   - @c geometry::Point3D (K=3)
 *
 * @par Usage Example
 * @code
 *   // 2D points via std::array
 *   KDTree<std::array<double, 2>, 2> tree;
 *   tree.build({{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}});
 *   auto nn = tree.nearestNeighbor({3.1, 3.9});
 *
 *   // 3D points via geometry::Point3D
 *   KDTree<geometry::Point3D, 3> tree3d;
 *   tree3d.insert({1.0, 2.0, 3.0});
 *   auto knn = tree3d.kNearestNeighbors({1.1, 2.1, 3.1}, 3);
 * @endcode
 *
 * @tparam PointT  The point value type.
 * @tparam K       Dimensionality (must be &gt; 0).
 * @tparam Accessor Coordinate accessor trait (defaults to KDTreeAccessor).
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>

#include "data_structure/geometry/Point2D.hpp"
#include "data_structure/geometry/Point3D.hpp"

namespace common::data_structure::spatial
{
    // ══════════════════════════════════════════════════════════════════════
    //  KDTreeAccessor — coordinate extraction trait
    // ══════════════════════════════════════════════════════════════════════

    /// @brief Primary template: requires @c operator[] on the point type.
    ///
    /// Users may specialise this trait for their own point types.  Every
    /// specialisation must provide:
    /// @code
    ///   static double get(const PointT& p, size_t dim);
    /// @endcode
    ///
    /// @tparam PointT The point type.
    /// @tparam K      Dimensionality (ignored by the default accessor).
    template <typename PointT, size_t K>
    struct KDTreeAccessor
    {
        /// @brief Extracts the @p dim -th coordinate from @p p.
        static auto get(const PointT& p, size_t dim) -> double
        {
            return static_cast<double>(p[dim]);
        }
    };

    /// @brief Accessor specialisation for geometry::Point2D.
    template <>
    struct KDTreeAccessor<geometry::Point2D, 2>
    {
        static auto get(const geometry::Point2D& p, size_t dim) -> double
        {
            switch (dim)
            {
                case 0: return p.getX();
                case 1: return p.getY();
                default:
                    throw std::out_of_range(
                        "Point2D accessor: dimension must be 0 or 1");
            }
        }
    };

    /// @brief Accessor specialisation for geometry::Point3D.
    template <>
    struct KDTreeAccessor<geometry::Point3D, 3>
    {
        static auto get(const geometry::Point3D& p, size_t dim) -> double
        {
            switch (dim)
            {
                case 0: return p.getX();
                case 1: return p.getY();
                case 2: return p.getZ();
                default:
                    throw std::out_of_range(
                        "Point3D accessor: dimension must be 0, 1, or 2");
            }
        }
    };

    // ══════════════════════════════════════════════════════════════════════
    //  KDTree — K-dimensional tree
    // ══════════════════════════════════════════════════════════════════════

    /// @brief A K-dimensional tree for spatial searches.
    ///
    /// Supports bulk construction (optimal median split), single-point
    /// insertion, nearest-neighbour, KNN, and radius-based range search.
    ///
    /// @tparam PointT  Point value type.
    /// @tparam K       Number of dimensions.
    /// @tparam Accessor Coordinate accessor (default KDTreeAccessor).
    template <typename PointT, size_t K,
              typename Accessor = KDTreeAccessor<PointT, K>>
    class KDTree
    {
        static_assert(K > 0, "KDTree requires at least 1 dimension");

    public:
        using point_type      = PointT;
        using accessor_type   = Accessor;

        /// @brief Compile-time dimensionality.
        static constexpr size_t Dimensions = K;

        // ── Construction / destruction ──────────────────────────────────

        KDTree() = default;
        ~KDTree() = default;

        // Disable copy (internal std::mutex and std::unique_ptr are
        // non-copyable).
        KDTree(const KDTree&) = delete;
        auto operator=(const KDTree&) -> KDTree& = delete;

        // ── Move ────────────────────────────────────────────────────────
        // std::mutex is not movable, so we implement move manually.

        KDTree(KDTree&& other) noexcept
            : root_(std::move(other.root_))
            , size_(other.size_)
        {
            other.size_ = 0;
        }

        auto operator=(KDTree&& other) noexcept -> KDTree&
        {
            if (this != &other)
            {
                std::lock(mutex_, other.mutex_);
                std::lock_guard lk_this(mutex_, std::adopt_lock);
                std::lock_guard lk_other(other.mutex_, std::adopt_lock);

                root_ = std::move(other.root_);
                size_ = other.size_;
                other.size_ = 0;
            }
            return *this;
        }

        // ── Bulk build ──────────────────────────────────────────────────

        /// @brief Builds the tree from a range of points (optimal median
        ///        split).  Any existing points are discarded.
        ///
        /// @tparam Iter  Forward iterator whose value_type is PointT.
        /// @param  first Beginning of the input range.
        /// @param  last  End of the input range.
        template <typename Iter>
        void build(Iter first, Iter last)
        {
            std::vector<PointT> points(first, last);
            std::lock_guard lock(mutex_);
            root_ = buildTree(points.data(), points.size(), 0);
            size_ = points.size();
        }

        /// @brief Builds the tree from an initializer list.
        void build(std::initializer_list<PointT> points)
        {
            build(points.begin(), points.end());
        }

        // ── Insert ──────────────────────────────────────────────────────

        /// @brief Inserts a single point into the tree.
        ///
        /// The point is inserted via recursive descent; the tree may become
        /// unbalanced over many insertions.  For balanced performance
        /// consider @c build() instead.
        ///
        /// If the point already exists (by @c operator==) it will **not**
        /// be inserted again.
        ///
        /// @param point The point to insert.
        /// @return @c true if the point was inserted, @c false if a
        ///         duplicate was found.
        auto insert(const PointT& point) -> bool
        {
            std::lock_guard lock(mutex_);
            if (insertNode(root_, point, 0))
            {
                ++size_;
                return true;
            }
            return false;
        }

        // ── Nearest neighbour ───────────────────────────────────────────

        /// @brief Finds the point closest to @p query.
        /// @param query The query point.
        /// @return The closest point, or @c std::nullopt if the tree is
        ///         empty.
        [[nodiscard]] auto nearestNeighbor(const PointT& query) const
            -> std::optional<PointT>
        {
            std::lock_guard lock(mutex_);
            if (!root_)
            {
                return std::nullopt;
            }

            const PointT* best    = nullptr;
            double        bestSq  = std::numeric_limits<double>::max();

            nearestSearch(root_.get(), query, 0, best, bestSq);

            if (best)
            {
                return *best;
            }
            return std::nullopt;
        }

        // ── K-nearest neighbours ────────────────────────────────────────

        /// @brief Finds the @p k closest points to @p query.
        ///
        /// The result is sorted by increasing distance from the query.
        ///
        /// @param query The query point.
        /// @param k     Number of neighbours to return.
        /// @return At most @p k points, sorted by distance (closest first).
        [[nodiscard]] auto kNearestNeighbors(const PointT& query,
                                             size_t k) const
            -> std::vector<PointT>
        {
            if (k == 0)
            {
                return {};
            }

            std::lock_guard lock(mutex_);
            if (!root_)
            {
                return {};
            }

            // Max-heap: keep the k closest; top() is the farthest among
            // the current candidates.
            using Pair = std::pair<double, const PointT*>;
            auto cmp  = [](const Pair& a, const Pair& b)
            {
                return a.first < b.first; // max-heap
            };
            std::priority_queue<Pair, std::vector<Pair>, decltype(cmp)> heap(cmp);

            knnSearch(root_.get(), query, 0, k, heap);

            // Extract results in reverse order (farthest first), then
            // reverse for closest-first output.
            std::vector<PointT> result;
            result.reserve(heap.size());
            while (!heap.empty())
            {
                result.push_back(*heap.top().second);
                heap.pop();
            }
            std::reverse(result.begin(), result.end());
            return result;
        }

        // ── Range search ────────────────────────────────────────────────

        /// @brief Finds all points within @p radius of @p query.
        /// @param query  The query point.
        /// @param radius Search radius (Euclidean distance).
        /// @return Points whose distance from @p query is &le; @p radius.
        [[nodiscard]] auto rangeSearch(const PointT& query,
                                       double radius) const
            -> std::vector<PointT>
        {
            std::lock_guard lock(mutex_);
            std::vector<PointT> result;
            if (!root_)
            {
                return result;
            }

            const double radiusSq = radius * radius;
            rangeSearch(root_.get(), query, radiusSq, 0, result);
            return result;
        }

        // ── Administration ──────────────────────────────────────────────

        /// @brief Removes all points from the tree.
        void clear()
        {
            std::lock_guard lock(mutex_);
            root_.reset();
            size_ = 0;
        }

        /// @brief Returns the number of points in the tree.
        [[nodiscard]] auto size() const -> size_t
        {
            std::lock_guard lock(mutex_);
            return size_;
        }

        /// @brief Checks whether the tree contains any points.
        [[nodiscard]] auto empty() const -> bool
        {
            std::lock_guard lock(mutex_);
            return size_ == 0;
        }

    private:
        // ── Internal types ──────────────────────────────────────────────

        /// @brief A KD-Tree node.
        struct Node
        {
            PointT                 point;
            std::unique_ptr<Node>  left;
            std::unique_ptr<Node>  right;

            explicit Node(PointT pt)
                : point(std::move(pt))
            {
            }
        };

        // ── Internal helpers ────────────────────────────────────────────

        /// @brief Coordinate access via the accessor trait.
        static auto coord(const PointT& p, size_t dim) -> double
        {
            return Accessor::get(p, dim);
        }

        /// @brief Squared Euclidean distance between two points.
        static auto squaredDistance(const PointT& a,
                                    const PointT& b) -> double
        {
            double sum = 0.0;
            for (size_t d = 0; d < K; ++d)
            {
                const double diff = coord(a, d) - coord(b, d);
                sum += diff * diff;
            }
            return sum;
        }

        // ── Build helper ────────────────────────────────────────────────

        /// @brief Recursively builds a balanced subtree from @p points.
        ///
        /// Uses @c std::nth_element to find the median along the current
        /// splitting axis.  The input range is reordered in-place.
        ///
        /// @param points Mutable span of points.
        /// @param count  Number of points in the span.
        /// @param depth  Current depth in the tree (determines splitting
        ///               axis).
        /// @return The root node of the constructed subtree.
        static auto buildTree(PointT* points, size_t count,
                              size_t depth) -> std::unique_ptr<Node>
        {
            if (count == 0)
            {
                return nullptr;
            }

            const size_t axis = depth % K;
            const size_t mid  = count / 2;

            // Partial sort: place the median element at position mid.
            auto cmp = [axis](const PointT& a, const PointT& b)
            {
                return coord(a, axis) < coord(b, axis);
            };
            std::nth_element(points, points + mid, points + count, cmp);

            auto node         = std::make_unique<Node>(points[mid]);
            node->left        = buildTree(points, mid, depth + 1);
            node->right       = buildTree(points + mid + 1,
                                          count - mid - 1, depth + 1);
            return node;
        }

        // ── Insert helper ───────────────────────────────────────────────

        /// @brief Recursively inserts a point into the subtree.
        ///
        /// @param node  Current subtree root (may be null).
        /// @param point The point to insert.
        /// @param depth Current depth.
        /// @return @c true if inserted, @c false if duplicate.
        static auto insertNode(std::unique_ptr<Node>& node,
                               const PointT& point,
                               size_t depth) -> bool
        {
            if (!node)
            {
                node = std::make_unique<Node>(point);
                return true;
            }

            // Check for exact duplicate
            if constexpr (K <= 3)
            {
                // Fast-path: use operator== for small dimensions
                if (node->point == point)
                {
                    return false;
                }
            }
            else
            {
                // Generic: compare all coordinates
                bool same = true;
                for (size_t d = 0; d < K; ++d)
                {
                    if (coord(node->point, d) != coord(point, d))
                    {
                        same = false;
                        break;
                    }
                }
                if (same)
                {
                    return false;
                }
            }

            const size_t axis = depth % K;
            if (coord(point, axis) < coord(node->point, axis))
            {
                return insertNode(node->left, point, depth + 1);
            }
            return insertNode(node->right, point, depth + 1);
        }

        // ── Nearest-neighbour search ────────────────────────────────────

        /// @brief Recursive nearest-neighbour search with pruning.
        ///
        /// @param node      Current node (non-null).
        /// @param query     The query point.
        /// @param depth     Current depth.
        /// @param best[in,out] Best point found so far.
        /// @param bestDistSq[in,out] Squared distance to @p best.
        static void nearestSearch(const Node* node,
                                  const PointT& query,
                                  size_t depth,
                                  const PointT*& best,
                                  double& bestDistSq) noexcept
        {
            const size_t axis = depth % K;
            const double coord_q = coord(query, axis);
            const double coord_n = coord(node->point, axis);

            // Determine near / far child
            const Node* nearChild = nullptr;
            const Node* farChild  = nullptr;
            if (coord_q < coord_n)
            {
                nearChild = node->left.get();
                farChild  = node->right.get();
            }
            else
            {
                nearChild = node->right.get();
                farChild  = node->left.get();
            }

            // Search the near side first
            if (nearChild)
            {
                nearestSearch(nearChild, query, depth + 1,
                              best, bestDistSq);
            }

            // Check current node
            const double distSq = squaredDistance(query, node->point);
            if (distSq < bestDistSq)
            {
                bestDistSq = distSq;
                best       = &node->point;
            }

            // Prune the far side if the splitting plane is farther than
            // the current best distance.
            const double axisDelta = coord_n - coord_q;
            if (farChild && (axisDelta * axisDelta) < bestDistSq)
            {
                nearestSearch(farChild, query, depth + 1,
                              best, bestDistSq);
            }
        }

        // ── KNN search ──────────────────────────────────────────────────

        /// @brief Recursive KNN search using a max-heap.
        ///
        /// The heap maintains the @p k closest points found so far.
        /// Branches whose splitting plane is farther than the heap's
        /// top (farthest among the candidates) are pruned.
        ///
        /// @param node   Current node.
        /// @param query  Query point.
        /// @param depth  Current depth.
        /// @param k      Number of neighbours requested.
        /// @param heap   Max-heap of (squared distance, point*).
        using DistPair = std::pair<double, const PointT*>;

        template <typename Cmp>
        static void knnSearch(const Node* node,
                              const PointT& query,
                              size_t depth,
                              size_t k,
                              std::priority_queue<DistPair,
                                                  std::vector<DistPair>,
                                                  Cmp>& heap)
        {
            const size_t axis = depth % K;
            const double coord_q = coord(query, axis);
            const double coord_n = coord(node->point, axis);

            const Node* nearChild = nullptr;
            const Node* farChild  = nullptr;
            if (coord_q < coord_n)
            {
                nearChild = node->left.get();
                farChild  = node->right.get();
            }
            else
            {
                nearChild = node->right.get();
                farChild  = node->left.get();
            }

            // Search the near side first
            if (nearChild)
            {
                knnSearch(nearChild, query, depth + 1, k, heap);
            }

            // Consider current node
            const double distSq = squaredDistance(query, node->point);

            if (heap.size() < k)
            {
                heap.emplace(distSq, &node->point);
            }
            else if (distSq < heap.top().first)
            {
                heap.pop();
                heap.emplace(distSq, &node->point);
            }

            // Prune the far side
            const double axisDelta = coord_n - coord_q;
            if (farChild && (heap.size() < k ||
                             (axisDelta * axisDelta) < heap.top().first))
            {
                knnSearch(farChild, query, depth + 1, k, heap);
            }
        }

        // ── Range search ────────────────────────────────────────────────

        /// @brief Recursive radius-based range search.
        ///
        /// @param node     Current node.
        /// @param query    Query point.
        /// @param radiusSq Squared search radius.
        /// @param depth    Current depth.
        /// @param result   Output vector.
        static void rangeSearch(const Node* node,
                                const PointT& query,
                                double radiusSq,
                                size_t depth,
                                std::vector<PointT>& result)
        {
            const size_t axis = depth % K;
            const double coord_q = coord(query, axis);
            const double coord_n = coord(node->point, axis);

            // Check current point
            if (squaredDistance(query, node->point) <= radiusSq)
            {
                result.push_back(node->point);
            }

            // Which side of the splitting plane?
            const Node* nearChild = nullptr;
            const Node* farChild  = nullptr;
            if (coord_q < coord_n)
            {
                nearChild = node->left.get();
                farChild  = node->right.get();
            }
            else
            {
                nearChild = node->right.get();
                farChild  = node->left.get();
            }

            if (nearChild)
            {
                rangeSearch(nearChild, query, radiusSq, depth + 1, result);
            }

            // Check far side only if the splitting plane is within radius
            const double axisDelta = coord_n - coord_q;
            if (farChild && (axisDelta * axisDelta) <= radiusSq)
            {
                rangeSearch(farChild, query, radiusSq, depth + 1, result);
            }
        }

        // ── Member data ─────────────────────────────────────────────────

        mutable std::mutex       mutex_;
        std::unique_ptr<Node>    root_;
        size_t                   size_ = 0;
    };

} // namespace common::data_structure::spatial
