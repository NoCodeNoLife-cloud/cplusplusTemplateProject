/**
 * @file RTree.hpp
 * @brief R*-Tree �?spatial index for axis-aligned bounding boxes
 * @details A bounded-capacity R*-Tree (Beckmann et al., 1990) for
 *          N-dimensional spatial data.  Supports exact-match insertion,
 *          deletion, rectangular range search, and K-nearest-neighbour
 *          queries.  The tree self-balances via forced reinsertion and
 *          quadratic-cost node splitting.
 *
 * Reference: Beckmann, N., Kriegel, H.-P., Schneider, R., Seeger, B.,
 *            "The R*-tree: an efficient and robust access method for
 *            points and rectangles" (SIGMOD 1990).
 *
 * @par Thread Safety
 * All public methods are thread-safe.  Concurrent access is serialised via
 * an internal @c std::mutex.
 *
 * @par Template Parameters
 * @tparam DataT      Value type associated with each stored entry.
 * @tparam Dims       Number of spatial dimensions (must be > 0).
 * @tparam MaxEntries Maximum number of entries per node (must be >= 3).
 *
 * @par Usage Example
 * @code
 *   // 2-D spatial index for string data
 *   RTree<std::string, 2, 16> tree;
 *
 *   // Insert items with their bounding boxes
 *   tree.insert({{0.0, 0.0}, {1.0, 1.0}}, "item-A");
 *   tree.insert({{2.0, 2.0}, {3.0, 3.0}}, "item-B");
 *
 *   // Range query
 *   auto results = tree.search({{0.0, 0.0}, {5.0, 5.0}});
 *
 *   // K-nearest-neighbour
 *   auto knn = tree.nearestNeighbor(std::array{1.5, 1.5}, 5);
 * @endcode
 */

#pragma once

#include <cppforge/data_structure/spatial/BoundingBox.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cmath>
#include <limits>
#include <memory>
#include <mutex>
#include <queue>
#include <utility>
#include <vector>

namespace cppforge::data_structure::spatial
{

// ════════════════════════════════════════════════════════════════════════�?//  RTree
// ════════════════════════════════════════════════════════════════════════�?
/// @brief R*-Tree spatial index.
///
/// Stores entries keyed by N-dimensional axis-aligned bounding boxes.
/// The tree uses a minimum-entries threshold (40 % of MaxEntries) and
/// forced reinsertion (30 % of MaxEntries) to maintain good structure.
///
/// @tparam DataT      Payload type.
/// @tparam Dims       Dimensionality.
/// @tparam MaxEntries Maximum entries before overflow triggers split.
template <typename DataT, size_t Dims = 2, size_t MaxEntries = 10>
class RTree final
{
    static_assert(Dims > 0, "RTree requires at least 1 dimension");
    static_assert(MaxEntries >= 3, "RTree requires MaxEntries >= 3");

public:
    /// Payload type alias.
    using data_type = DataT;

    /// Bounding box type used internally (always double-precision).
    using box_type = BoundingBox<double, Dims>;

    /// Compile-time dimensionality.
    static constexpr size_t kDimensions = Dims;

    /// Minimum entries per node (40 % of MaxEntries, at least 1).
    static constexpr size_t kMinEntries = std::max<size_t>(MaxEntries * 40 / 100, 1);

    /// Number of entries reinserted on forced reinsertion (30 %).
    static constexpr size_t kReinsertCount = std::max<size_t>(MaxEntries * 30 / 100, 1);

    // ── Construction ────────────────────────────────────────────────────

    RTree() = default;
    ~RTree() = default;

    // Disable copy (mutex is non-copyable).
    RTree(const RTree&) = delete;
    auto operator=(const RTree&) -> RTree& = delete;

    /// @brief Move constructor.
    RTree(RTree&& other) noexcept
        : root_(std::move(other.root_))
        , dataCount_(other.dataCount_)
    {
        other.dataCount_ = 0;
    }

    /// @brief Move-assignment operator.
    auto operator=(RTree&& other) noexcept -> RTree&
    {
        if (this != &other)
        {
            std::lock(mtx_, other.mtx_);
            std::lock_guard lk_this(mtx_, std::adopt_lock);
            std::lock_guard lk_other(other.mtx_, std::adopt_lock);

            root_ = std::move(other.root_);
            dataCount_ = other.dataCount_;
            other.dataCount_ = 0;
        }
        return *this;
    }

    // ── Modifiers ───────────────────────────────────────────────────────

    /// @brief Inserts an entry with copy semantics.
    /// @param bbox The bounding box of the entry.
    /// @param data The payload to associate.
    void insert(const box_type& bbox, const DataT& data)
    {
        std::lock_guard lock(mtx_);
        insertImpl(bbox, DataT(data), 0);
    }

    /// @brief Inserts an entry with move semantics.
    /// @param bbox The bounding box of the entry.
    /// @param data The payload to move.
    void insert(const box_type& bbox, DataT&& data)
    {
        std::lock_guard lock(mtx_);
        insertImpl(bbox, std::move(data), 0);
    }

    /// @brief Removes a single occurrence of (bbox, data).
    ///
    /// Performs exact matching on both the bounding box and the payload.
    /// Only the first matching entry is removed.
    ///
    /// @param bbox Bounding box to match.
    /// @param data Payload to match.
    /// @return @c true if an entry was found and removed.
    [[nodiscard]] auto remove(const box_type& bbox, const DataT& data) -> bool
    {
        std::lock_guard lock(mtx_);
        if (!root_)
        {
            return false;
        }

        std::vector<LeafEntry> orphanedItems;
        bool found = removeRecursive(root_.get(), nullptr, 0, bbox, data, orphanedItems);

        if (!found)
        {
            return false;
        }

        // Reinsert all orphaned entries.
        for (auto& entry : orphanedItems)
        {
            insertImpl(entry.first, std::move(entry.second), 0, false);
        }

        // If root has a single child, collapse it.
        if (!root_->isLeaf_ && root_->children_.size() == 1)
        {
            root_ = std::move(root_->children_[0].second);
        }

        --dataCount_;
        return true;
    }

    /// @brief Removes all entries from the tree.
    void clear()
    {
        std::lock_guard lock(mtx_);
        root_.reset();
        dataCount_ = 0;
    }

    // ── Queries ─────────────────────────────────────────────────────────

    /// @brief Finds all entries whose bounding boxes intersect @p query.
    /// @param query The query bounding box.
    /// @return Vector of matching payloads.
    [[nodiscard]] auto search(const box_type& query) const -> std::vector<DataT>
    {
        std::lock_guard lock(mtx_);
        std::vector<DataT> result;
        if (root_)
        {
            searchNode(root_.get(), query, result);
        }
        return result;
    }

    /// @brief Finds the @p k closest entries to @p query.
    ///
    /// Uses Euclidean distance between the query point and each entry's
    /// bounding box (zero if inside).  Results are sorted by increasing
    /// distance.
    ///
    /// @tparam PointT  Point type supporting @c operator[].
    /// @param  query   The query point.
    /// @param  k       Number of neighbours to return.
    /// @return Up to @p k (data, distance) pairs, closest first.
    template <typename PointT>
    [[nodiscard]] auto nearestNeighbor(const PointT& query, size_t k) const
        -> std::vector<std::pair<DataT, double>>
    {
        std::lock_guard lock(mtx_);
        std::vector<std::pair<DataT, double>> result;
        if (!root_ || k == 0)
        {
            return result;
        }

        // Max-heap: keep the k closest candidates.
        struct Candidate
        {
            double distSq_;
            DataT data_;
        };
        auto candCmp = [](const Candidate& a, const Candidate& b) { return a.distSq_ < b.distSq_; };
        std::priority_queue<Candidate, std::vector<Candidate>, decltype(candCmp)> candidates(candCmp);

        // Min-heap: best-first traversal of nodes.
        struct QueueEntry
        {
            double minDistSq_;
            const Node* node_;
        };
        auto qCmp = [](const QueueEntry& a, const QueueEntry& b) { return a.minDistSq_ > b.minDistSq_; };
        std::priority_queue<QueueEntry, std::vector<QueueEntry>, decltype(qCmp)> queue(qCmp);
        queue.push({0.0, root_.get()});

        while (!queue.empty())
        {
            auto [minDistSq, curNode] = queue.top();
            queue.pop();

            // Prune: every remaining node is at least this far away.
            if (candidates.size() >= k && minDistSq >= candidates.top().distSq_)
            {
                break;
            }

            if (curNode->isLeaf_)
            {
                for (const auto& item : curNode->items_)
                {
                    const double dSq = item.first.minDistSquared(query);
                    if (candidates.size() < k)
                    {
                        candidates.push({dSq, item.second});
                    }
                    else if (dSq < candidates.top().distSq_)
                    {
                        candidates.pop();
                        candidates.push({dSq, item.second});
                    }
                }
            }
            else
            {
                for (const auto& child : curNode->children_)
                {
                    const double dSq = child.first.minDistSquared(query);
                    queue.push({dSq, child.second.get()});
                }
            }
        }

        // Extract and sort by distance (closest first).
        result.reserve(candidates.size());
        while (!candidates.empty())
        {
            result.emplace_back(candidates.top().data_, std::sqrt(candidates.top().distSq_));
            candidates.pop();
        }
        std::reverse(result.begin(), result.end());
        return result;
    }

    // ── Status ──────────────────────────────────────────────────────────

    /// @brief Returns the number of entries stored.
    [[nodiscard]] auto size() const -> size_t
    {
        std::lock_guard lock(mtx_);
        return dataCount_;
    }

    /// @brief Checks whether the tree is empty.
    [[nodiscard]] auto empty() const -> bool
    {
        std::lock_guard lock(mtx_);
        return dataCount_ == 0;
    }

    /// @brief Returns the height (number of levels) of the tree.
    /// @return 0 if empty, otherwise the number of levels.
    [[nodiscard]] auto height() const -> size_t
    {
        std::lock_guard lock(mtx_);
        if (!root_) return 0;

        size_t h = 1;
        const Node* n = root_.get();
        while (!n->isLeaf_ && !n->children_.empty())
        {
            ++h;
            n = n->children_[0].second.get();
        }
        return h;
    }

private:
    // ── Internal types ──────────────────────────────────────────────────

    /// @brief A node in the R-Tree.
    struct Node
    {
        bool isLeaf_;
        box_type mbr_;
        std::vector<std::pair<box_type, std::unique_ptr<Node>>> children_;
        std::vector<std::pair<box_type, DataT>> items_;

        explicit Node(bool isLeaf)
            : isLeaf_(isLeaf)
        {
        }
    };

    /// Type alias for internal node entries.
    using InternalEntry = std::pair<box_type, std::unique_ptr<Node>>;

    /// Type alias for leaf entries.
    using LeafEntry = std::pair<box_type, DataT>;

    // ── Mutable state ──────────────────────────────────────────────────

    mutable std::mutex mtx_;
    std::unique_ptr<Node> root_;
    size_t dataCount_{0};

    // ══════════════════════════════════════════════════════════════════════�?    //  Algorithm helpers
    // ══════════════════════════════════════════════════════════════════════�?
    // ── computeMBR ──────────────────────────────────────────────────────

    /// @brief Computes the MBR enclosing a list of leaf entries.
    static auto computeMBR(const std::vector<LeafEntry>& entries) -> box_type
    {
        if (entries.empty())
        {
            return box_type{};
        }
        box_type mbr = entries[0].first;
        for (size_t i = 1; i < entries.size(); ++i)
        {
            mbr.enlarge(entries[i].first);
        }
        return mbr;
    }

    /// @brief Computes the MBR enclosing a list of internal entries.
    static auto computeMBR(const std::vector<InternalEntry>& entries) -> box_type
    {
        if (entries.empty())
        {
            return box_type{};
        }
        box_type mbr = entries[0].first;
        for (size_t i = 1; i < entries.size(); ++i)
        {
            mbr.enlarge(entries[i].first);
        }
        return mbr;
    }

    // ── intersectionArea ────────────────────────────────────────────────

    /// @brief Returns the area (volume) of the intersection of two boxes.
    static auto intersectionArea(const box_type& a, const box_type& b) -> double
    {
        double area = 1.0;
        for (size_t d = 0; d < Dims; ++d)
        {
            const double lo = std::max(a.min(d), b.min(d));
            const double hi = std::min(a.max(d), b.max(d));
            if (lo >= hi)
            {
                return 0.0;
            }
            area *= (hi - lo);
        }
        return area;
    }

    // ── chooseSubtree ───────────────────────────────────────────────────

    /// @brief R*-Tree ChooseSubtree: descends to the best leaf for @p bbox.
    ///
    /// At level 1 (children are leaves) minimises overlap enlargement.
    /// At deeper levels minimises area enlargement.
    static auto chooseSubtree(Node* node, const box_type& bbox, size_t level) -> Node*
    {
        if (node->isLeaf_)
        {
            return node;
        }

        const size_t childCount = node->children_.size();

        // ── Level 1 (children are leaves) �?minimise overlap enlargement ─

        if (level == 0)
        {
            size_t bestIdx = 0;
            double bestOverlap = std::numeric_limits<double>::max();
            double bestEnl = std::numeric_limits<double>::max();
            double bestArea = std::numeric_limits<double>::max();

            for (size_t i = 0; i < childCount; ++i)
            {
                const auto& child = node->children_[i];
                const box_type enlarged = child.first.combined(bbox);

                // Compute overlap of the enlarged MBR against all siblings.
                double overlapSum = 0.0;
                for (size_t j = 0; j < childCount; ++j)
                {
                    if (j == i) continue;
                    overlapSum += intersectionArea(enlarged, node->children_[j].first);
                }

                // Original overlap.
                double origOverlap = 0.0;
                for (size_t j = 0; j < childCount; ++j)
                {
                    if (j == i) continue;
                    origOverlap += intersectionArea(child.first, node->children_[j].first);
                }

                const double overlapEnl = overlapSum - origOverlap;
                const double enl = child.first.enlargement(bbox);
                const double area = child.first.area();

                if (overlapEnl < bestOverlap ||
                    (overlapEnl == bestOverlap && enl < bestEnl) ||
                    (overlapEnl == bestOverlap && enl == bestEnl && area < bestArea))
                {
                    bestOverlap = overlapEnl;
                    bestEnl = enl;
                    bestArea = area;
                    bestIdx = i;
                }
            }

            return chooseSubtree(node->children_[bestIdx].second.get(), bbox, level + 1);
        }

        // ── Deeper levels �?minimise area enlargement ─────────────────

        size_t bestIdx = 0;
        double bestEnl = std::numeric_limits<double>::max();
        double bestArea = std::numeric_limits<double>::max();

        for (size_t i = 0; i < childCount; ++i)
        {
            const auto& child = node->children_[i];
            const double enl = child.first.enlargement(bbox);
            const double area = child.first.area();

            if (enl < bestEnl || (enl == bestEnl && area < bestArea))
            {
                bestEnl = enl;
                bestArea = area;
                bestIdx = i;
            }
        }

        return chooseSubtree(node->children_[bestIdx].second.get(), bbox, level + 1);
    }

    // ── insertImpl ──────────────────────────────────────────────────────

    /// @brief Internal insertion routine.
    ///
    /// @param bbox   Bounding box of the entry.
    /// @param data   Payload (moved-in).
    /// @param depth  Reinsertion depth (0 = normal, 1+ = skip reinsert).
    void insertImpl(const box_type& bbox, DataT&& data, size_t depth, bool countData = true)
    {
        if (!root_)
        {
            // Empty tree �?create leaf root.
            root_ = std::make_unique<Node>(true);
            root_->mbr_ = bbox;
            root_->items_.emplace_back(bbox, std::move(data));
            if (countData) dataCount_ = 1;
            return;
        }

        // Choose leaf.
        Node* leaf = chooseSubtree(root_.get(), bbox, 0);

        // Add entry.
        leaf->items_.emplace_back(bbox, std::move(data));
        leaf->mbr_.enlarge(bbox);

        // Handle overflow.
        if (leaf->items_.size() > MaxEntries)
        {
            std::unique_ptr<Node> splitNode = nullptr;
            handleLeafOverflow(leaf, splitNode, depth);

            if (splitNode)
            {
                // A split propagated back to the root �?grow the tree.
                auto newRoot = std::make_unique<Node>(false);
                // Recompute root MBR from its children.
                box_type rootMbr = root_->mbr_;
                rootMbr.enlarge(splitNode->mbr_);
                newRoot->mbr_ = rootMbr;
                newRoot->children_.emplace_back(root_->mbr_, std::move(root_));
                newRoot->children_.emplace_back(splitNode->mbr_, std::move(splitNode));
                root_ = std::move(newRoot);
            }
        }

        if (countData) { ++dataCount_; }
    }

    // ── handleLeafOverflow ──────────────────────────────────────────────

    /// @brief Resolves an overflow in a leaf node.
    ///
    /// Tries forced reinsertion first (if @p reinsertDepth == 0 and node
    /// is not the root), then falls back to a full split.
    ///
    /// @param leaf           The overflowing leaf.
    /// @param splitNode[out] Receives the new node created by a split
    ///                       (nullptr if reinsertion sufficed).
    /// @param reinsertDepth  Reinsertion recursion counter.
    void handleLeafOverflow(Node* leaf,
                            std::unique_ptr<Node>& splitNode,
                            size_t reinsertDepth)
    {
        // Try forced reinsertion first.
        if (reinsertDepth == 0 && leaf != root_.get())
        {
            reinsertLeafEntries(leaf);
            // After reinsertion the leaf should be within capacity.
            return;
        }

        // Second overflow or root �?split.
        splitLeafNode(leaf, splitNode);
    }

    // ── reinsertLeafEntries ─────────────────────────────────────────────

    /// @brief Removes the farthest entries from @p leaf and reinserts them.
    ///
    /// The farthest kReinsertCount entries (by center-to-center distance
    /// from the node's MBR centre) are removed and reinserted individually.
    void reinsertLeafEntries(Node* leaf)
    {
        const std::array<double, Dims> center = leaf->mbr_.center();
        const size_t n = leaf->items_.size();
        const size_t removeCount = std::min(kReinsertCount, n);

        // Compute distances from each entry's centre to the node centre.
        std::vector<std::pair<double, size_t>> distIdx(n);
        for (size_t i = 0; i < n; ++i)
        {
            const auto entryCenter = leaf->items_[i].first.center();
            double sum = 0.0;
            for (size_t d = 0; d < Dims; ++d)
            {
                const double diff = entryCenter[d] - center[d];
                sum += diff * diff;
            }
            distIdx[i] = {sum, i};
        }

        // Sort by distance descending.
        std::sort(distIdx.begin(), distIdx.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });

        // Extract entries to reinsert (the farthest ones).
        std::vector<LeafEntry> toReinsert;
        toReinsert.reserve(removeCount);
        for (size_t i = 0; i < removeCount; ++i)
        {
            toReinsert.emplace_back(std::move(leaf->items_[distIdx[i].second]));
        }

        // Remove them from the leaf (keep the rest).
        // Sort indices in descending order to erase without invalidation.
        std::vector<size_t> removeIndices;
        removeIndices.reserve(removeCount);
        for (size_t i = 0; i < removeCount; ++i)
        {
            removeIndices.push_back(distIdx[i].second);
        }
        std::sort(removeIndices.begin(), removeIndices.end(), std::greater<size_t>());
        for (const size_t idx : removeIndices)
        {
            leaf->items_.erase(leaf->items_.begin() + static_cast<ptrdiff_t>(idx));
        }

        // Recompute leaf MBR.
        leaf->mbr_ = computeMBR(leaf->items_);

        // Reinsert the removed entries (depth = 1 �?no nested reinsert).
        for (auto& entry : toReinsert)
        {
            insertImpl(entry.first, std::move(entry.second), 1, false);
        }
    }

    // ── splitLeafNode ─────────────────────────────────────────────────

    /// @brief Splits an overflowing leaf into two nodes.
    ///
    /// Uses the R*-Tree quadratic split strategy.  @p leaf retains the
    /// first group; a new node is created for the second group and
    /// returned via @p splitNode.
    ///
    /// @param leaf           The overflowing leaf (modified in place).
    /// @param splitNode[out] The newly created sibling node.
    void splitLeafNode(Node* leaf, std::unique_ptr<Node>& splitNode)
    {
        const auto& entries = leaf->items_;
        const size_t n = entries.size();

        // Build box-only view for the split algorithm.
        std::vector<BoxIndex> boxIdx(n);
        for (size_t i = 0; i < n; ++i)
        {
            boxIdx[i].bbox_ = entries[i].first;
            boxIdx[i].origIndex_ = i;
        }

        // Use the R*-Tree split helpers.
        SplitResult sr = splitEntries(boxIdx);

        // Build split node with the second group.
        auto newNode = std::make_unique<Node>(true);
        for (size_t origIdx : sr.secondGroup_)
        {
            newNode->items_.emplace_back(std::move(leaf->items_[origIdx]));
        }
        newNode->mbr_ = computeMBR(newNode->items_);

        // Rebuild leaf items from the first group.
        std::vector<LeafEntry> keptItems;
        keptItems.reserve(sr.firstGroup_.size());
        for (size_t origIdx : sr.firstGroup_)
        {
            keptItems.emplace_back(std::move(leaf->items_[origIdx]));
        }
        leaf->items_ = std::move(keptItems);
        leaf->mbr_ = computeMBR(leaf->items_);

        splitNode = std::move(newNode);
    }

    // ── splitEntries (box-only, R*-Tree split) ──────────────────────────

    /// @brief Holds a box and its original index for the split algorithm.
    struct BoxIndex
    {
        box_type bbox_;
        size_t origIndex_;
    };

    /// @brief Contains the two groups resulting from a split.
    struct SplitResult
    {
        std::vector<size_t> firstGroup_;
        std::vector<size_t> secondGroup_;
    };

    /// @brief R*-Tree split: choose split axis, then split index.
    static auto splitEntries(std::vector<BoxIndex>& entries) -> SplitResult
    {
        const size_t n = entries.size();
        const size_t axis = chooseSplitAxis(entries);

        // Sort by lower bound on the chosen axis.
        std::sort(entries.begin(), entries.end(),
                  [axis](const BoxIndex& a, const BoxIndex& b)
                  {
                      return a.bbox_.min(axis) < b.bbox_.min(axis);
                  });

        const size_t splitK = chooseSplitIndex(entries, axis);

        SplitResult result;
        result.firstGroup_.reserve(splitK);
        result.secondGroup_.reserve(n - splitK);

        for (size_t i = 0; i < splitK; ++i)
        {
            result.firstGroup_.push_back(entries[i].origIndex_);
        }
        for (size_t i = splitK; i < n; ++i)
        {
            result.secondGroup_.push_back(entries[i].origIndex_);
        }

        return result;
    }

    /// @brief R*-Tree ChooseSplitAxis.
    ///
    /// For each dimension, sorts entries by lower bound and by upper
    /// bound, computes the sum of margin-values over all valid
    /// distributions, and picks the axis with the smallest sum.
    static auto chooseSplitAxis(const std::vector<BoxIndex>& entries) -> size_t
    {
        const size_t n = entries.size();
        double bestS = std::numeric_limits<double>::max();
        size_t bestAxis = 0;

        for (size_t d = 0; d < Dims; ++d)
        {
            // Sort by lower bound.
            auto sortedByMin = entries;
            std::sort(sortedByMin.begin(), sortedByMin.end(),
                      [d](const BoxIndex& a, const BoxIndex& b)
                      {
                          return a.bbox_.min(d) < b.bbox_.min(d);
                      });
            double sMin = marginSum(sortedByMin);

            // Sort by upper bound.
            auto sortedByMax = entries;
            std::sort(sortedByMax.begin(), sortedByMax.end(),
                      [d](const BoxIndex& a, const BoxIndex& b)
                      {
                          return a.bbox_.max(d) < b.bbox_.max(d);
                      });
            double sMax = marginSum(sortedByMax);

            const double s = std::min(sMin, sMax);
            if (s < bestS)
            {
                bestS = s;
                bestAxis = d;
            }
        }

        return bestAxis;
    }

    /// @brief Sum of margin-values over all valid distributions for a
    ///        sorted entry list.
    static auto marginSum(const std::vector<BoxIndex>& sorted) -> double
    {
        const size_t n = sorted.size();
        double total = 0.0;
        for (size_t k = kMinEntries; k <= n - kMinEntries; ++k)
        {
            // MBR of first group [0, k).
            box_type mbr1 = sorted[0].bbox_;
            for (size_t i = 1; i < k; ++i)
            {
                mbr1.enlarge(sorted[i].bbox_);
            }
            // MBR of second group [k, n).
            box_type mbr2 = sorted[k].bbox_;
            for (size_t i = k + 1; i < n; ++i)
            {
                mbr2.enlarge(sorted[i].bbox_);
            }
            total += mbr1.margin() + mbr2.margin();
        }
        return total;
    }

    /// @brief R*-Tree ChooseSplitIndex.
    ///
    /// Picks the split position that minimises overlap-value, then
    /// area-value.  Assumes entries are sorted by lower bound on the
    /// chosen axis.
    static auto chooseSplitIndex(const std::vector<BoxIndex>& sorted,
                                 size_t axis) -> size_t
    {
        (void)axis;
        const size_t n = sorted.size();
        size_t bestK = kMinEntries;
        double bestOverlap = std::numeric_limits<double>::max();
        double bestArea = std::numeric_limits<double>::max();

        // Pre-compute prefix and suffix MBRs for O(n) evaluation.
        std::vector<box_type> prefix(n);
        std::vector<box_type> suffix(n);

        prefix[0] = sorted[0].bbox_;
        for (size_t i = 1; i < n; ++i)
        {
            prefix[i] = prefix[i - 1];
            prefix[i].enlarge(sorted[i].bbox_);
        }

        suffix[n - 1] = sorted[n - 1].bbox_;
        for (size_t i = n; i > 1; --i)
        {
            suffix[i - 2] = suffix[i - 1];
            suffix[i - 2].enlarge(sorted[i - 2].bbox_);
        }

        for (size_t k = kMinEntries; k <= n - kMinEntries; ++k)
        {
            const box_type& mbr1 = prefix[k - 1];
            const box_type& mbr2 = suffix[k];
            const double overlap = intersectionArea(mbr1, mbr2);
            const double areaSum = mbr1.area() + mbr2.area();

            if (overlap < bestOverlap ||
                (overlap == bestOverlap && areaSum < bestArea))
            {
                bestOverlap = overlap;
                bestArea = areaSum;
                bestK = k;
            }
        }

        return bestK;
    }

    // ── searchNode ──────────────────────────────────────────────────────

    /// @brief Recursive range search.
    static void searchNode(const Node* node, const box_type& query,
                           std::vector<DataT>& result)
    {
        if (!node->mbr_.intersects(query))
        {
            return;
        }

        if (node->isLeaf_)
        {
            for (const auto& item : node->items_)
            {
                if (item.first.intersects(query))
                {
                    result.push_back(item.second);
                }
            }
        }
        else
        {
            for (const auto& child : node->children_)
            {
                if (child.first.intersects(query))
                {
                    searchNode(child.second.get(), query, result);
                }
            }
        }
    }

    // ── removeRecursive ─────────────────────────────────────────────────

    /// @brief Recursive entry removal.
    ///
    /// Searches for the exact (bbox, data) pair and removes it from its
    /// leaf.  Walks back up the tree, updating MBRs and handling
    /// underflows by collecting orphaned entries.
    ///
    /// @return @c true if the entry was found and removed.
    static auto removeRecursive(Node* node, Node* parent, size_t parentIdx,
                                const box_type& bbox, const DataT& data,
                                std::vector<LeafEntry>& orphanedItems) -> bool
    {
        if (node->isLeaf_)
        {
            // Scan leaf items for a match.
            for (size_t i = 0; i < node->items_.size(); ++i)
            {
                if (node->items_[i].first == bbox && node->items_[i].second == data)
                {
                    // Remove the matching entry.
                    node->items_.erase(node->items_.begin() + static_cast<ptrdiff_t>(i));

                    if (parent != nullptr)
                    {
                        condenseAfterRemove(node, parent, parentIdx, orphanedItems);
                    }
                    return true;
                }
            }
            return false; // Not found in this leaf.
        }

        // Internal node �?search children.
        for (size_t i = 0; i < node->children_.size(); ++i)
        {
            auto& child = node->children_[i];
            if (!child.first.intersects(bbox))
            {
                continue;
            }

            if (removeRecursive(child.second.get(), node, i, bbox, data, orphanedItems))
            {
                // Recompute this node's MBR from remaining children.
                if (!node->children_.empty())
                {
                    node->mbr_ = computeMBR(node->children_);
                }
                return true;
            }
        }

        return false;
    }

    // ── condenseAfterRemove ─────────────────────────────────────────────

    /// @brief Walks up from the modified child, handling underflows.
    ///
    /// If @p childNode is under capacity, it is removed from @p parent
    /// and its entries (leaf items or entire subtrees) are added to the
    /// orphaned list for later reinsertion.
    static void condenseAfterRemove(Node* childNode, Node* parent,
                                    size_t childIdx,
                                    std::vector<LeafEntry>& orphanedItems)
    {
        // Check for underflow.
        const size_t entryCount = childNode->isLeaf_
                                      ? childNode->items_.size()
                                      : childNode->children_.size();

        if (entryCount < kMinEntries)
        {
            // Collect orphaned entries.
            if (childNode->isLeaf_)
            {
                for (auto& item : childNode->items_)
                {
                    orphanedItems.emplace_back(std::move(item));
                }
            }
            else
            {
                // Internal node �?recursively collect all leaf items.
                collectLeafItems(childNode, orphanedItems);
            }

            // Remove the node from its parent.
            parent->children_.erase(parent->children_.begin() + static_cast<ptrdiff_t>(childIdx));
        }
        else
        {
            // Node is fine �?just update parent's MBR for this child.
            parent->children_[childIdx].first = childNode->mbr_;
        }

        // Update parent's MBR.
        if (!parent->children_.empty())
        {
            parent->mbr_ = computeMBR(parent->children_);
        }
    }

    // ── collectLeafItems ────────────────────────────────────────────────

    /// @brief Recursively collects all leaf items from a subtree.
    static void collectLeafItems(Node* node, std::vector<LeafEntry>& out)
    {
        if (node->isLeaf_)
        {
            for (auto& item : node->items_)
            {
                out.emplace_back(std::move(item));
            }
            node->items_.clear();
        }
        else
        {
            for (auto& child : node->children_)
            {
                collectLeafItems(child.second.get(), out);
            }
            node->children_.clear();
        }
    }
};

} // namespace cppforge::data_structure::spatial
