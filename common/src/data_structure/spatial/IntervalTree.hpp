/**
 * @file IntervalTree.hpp
 * @brief IntervalTree — AVL-based interval tree for 1-D interval queries
 * @details A balanced interval tree implemented on top of an AVL self-balancing
 *          binary search tree.  Each node stores a half-open interval [low, high)
 *          and an augmented max_high_ value for O(log n) overlap and point queries.
 *          Supports insertion, deletion, interval overlap queries, and point
 *          stabbing queries.  All public methods are thread-safe.
 *
 * Reference: Cormen et al., "Introduction to Algorithms" (3rd ed.), Ch. 14.3:
 *            Interval Trees.
 *
 * @par Thread Safety
 * All public methods are thread-safe.  Concurrent access is serialised via an
 * internal std::mutex.  Queries hold the lock for the entire duration of the
 * traversal.
 *
 * @par Template Parameters
 * @tparam ValueT  Interval endpoint type (must satisfy std::regular and
 *                 std::totally_ordered).
 * @tparam DataT   Payload type associated with each interval.  Must support
 *                 equality comparison (operator==) for duplicate detection and
 *                 removal.
 *
 * @par Usage Example
 * @code
 *   // Integer interval tree with string payload
 *   IntervalTree<int, std::string> tree;
 *
 *   // Insert intervals
 *   tree.insert(Interval<int>(0, 10), "A");
 *   tree.insert(Interval<int>(5, 15), "B");
 *   tree.insert(Interval<int>(20, 30), "C");
 *
 *   // Overlap query
 *   auto overlapping = tree.queryOverlap(Interval<int>(8, 12));
 *   // -> {"A", "B"}
 *
 *   // Point query
 *   auto containing = tree.queryPoint(25);
 *   // -> {"C"}
 *
 *   // Removal
 *   tree.remove(Interval<int>(0, 10), "A");
 * @endcode
 */

#pragma once

#include "data_structure/spatial/Interval.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

namespace common::data_structure::spatial
{

// ═════════════════════════════════════════════════════════════════════════
//  IntervalTree
// ═════════════════════════════════════════════════════════════════════════

/// @brief AVL-based interval tree for 1-D interval indexing and queries.
///
/// Stores (interval, payload) pairs in a balanced binary search tree ordered
/// by interval (low first, then high).  Each node maintains its subtree's
/// maximum high value as augmented metadata, enabling O(log n + k) overlap
/// and point-stabbing queries.
///
/// @par Complexity
/// - Insert: O(log n)
/// - Remove: O(log n)
/// - queryOverlap: O(log n + k), where k is the number of results
/// - queryPoint: O(log n + k)
/// - size / empty: O(1)
///
/// @par Thread Safety
/// All public methods are thread-safe.  Internal mutex serialises all access.
///
/// @par Usage Example
/// @code
///   IntervalTree<double, int> tree;
///   tree.insert(Interval<double>(0.0, 1.0), 42);
///   auto results = tree.queryPoint(0.5);
/// @endcode
///
/// @tparam ValueT  Interval endpoint type.
/// @tparam DataT   Payload type.
template <typename ValueT, typename DataT>
    requires std::regular<ValueT> && std::totally_ordered<ValueT>
class IntervalTree final
{
    // ── Internal node ───────────────────────────────────────────────────

    /// @brief A node in the interval tree.
    struct Node
    {
        Interval<ValueT> interval_;          ///< The stored interval [low, high).
        DataT            data_;              ///< Payload data.
        ValueT           max_high_;          ///< Maximum high value in this subtree.
        int32_t          height_ = 1;        ///< AVL height (leaf = 1).
        std::unique_ptr<Node> left_;         ///< Left child (intervals with lower low).
        std::unique_ptr<Node> right_;        ///< Right child (intervals with higher or equal low).

        /// @brief Constructs a node with a copy of the payload.
        Node(Interval<ValueT> interval, const DataT& data)
            : interval_(std::move(interval))
            , data_(data)
            , max_high_(interval_.high())
        {
        }

        /// @brief Constructs a node with a moved payload.
        Node(Interval<ValueT> interval, DataT&& data)
            noexcept(std::is_nothrow_move_constructible_v<DataT>)
            : interval_(std::move(interval))
            , data_(std::move(data))
            , max_high_(interval_.high())
        {
        }
    };

public:
    // ── Construction / Destruction ───────────────────────────────────────

    IntervalTree() = default;
    ~IntervalTree() = default;

    // Disable copy (std::mutex is non-copyable).
    IntervalTree(const IntervalTree&) = delete;
    auto operator=(const IntervalTree&) -> IntervalTree& = delete;

    /// @brief Move constructor.
    ///
    /// Both mutexes are locked during the move to ensure thread safety.
    /// The moved-from object is left in a valid empty state.
    IntervalTree(IntervalTree&& other) noexcept
    {
        std::lock(mtx_, other.mtx_);
        std::lock_guard lk_this(mtx_, std::adopt_lock);
        std::lock_guard lk_other(other.mtx_, std::adopt_lock);

        root_ = std::move(other.root_);
        size_ = other.size_;
        other.size_ = 0;
    }

    /// @brief Move-assignment operator.
    ///
    /// Both mutexes are locked using std::lock to avoid deadlock.
    auto operator=(IntervalTree&& other) noexcept -> IntervalTree&
    {
        if (this != &other)
        {
            std::lock(mtx_, other.mtx_);
            std::lock_guard lk_this(mtx_, std::adopt_lock);
            std::lock_guard lk_other(other.mtx_, std::adopt_lock);

            root_ = std::move(other.root_);
            size_ = other.size_;
            other.size_ = 0;
        }
        return *this;
    }

    // ── Modifiers ───────────────────────────────────────────────────────

    /// @brief Inserts an interval with a copy of the payload.
    ///
    /// If an exact match (same interval low, interval high, and data) already
    /// exists, the tree is not modified and @c false is returned.
    ///
    /// @param interval The interval to insert.
    /// @param data     The payload to associate.
    /// @return @c true if a new entry was inserted, @c false if a duplicate
    ///         entry already existed.
    auto insert(const Interval<ValueT>& interval, const DataT& data) -> bool
    {
        std::lock_guard lock(mtx_);
        const size_t oldSize = size_;
        root_ = insertImpl(std::move(root_), interval, data);
        return size_ > oldSize;
    }

    /// @brief Inserts an interval with a moved payload.
    ///
    /// If an exact match (same interval low, interval high, and data) already
    /// exists, the tree is not modified and @c false is returned.
    ///
    /// @param interval The interval to insert.
    /// @param data     The payload to move into the tree.
    /// @return @c true if a new entry was inserted, @c false if a duplicate
    ///         entry already existed.
    auto insert(const Interval<ValueT>& interval, DataT&& data) -> bool
    {
        std::lock_guard lock(mtx_);
        const size_t oldSize = size_;
        root_ = insertImpl(std::move(root_), interval, std::move(data));
        return size_ > oldSize;
    }

    /// @brief Removes an exact (interval, data) match from the tree.
    ///
    /// Both the interval endpoints and the payload must match exactly for
    /// the entry to be removed.
    ///
    /// @param interval The interval to match.
    /// @param data     The payload to match.
    /// @return @c true if a matching entry was found and removed, @c false
    ///         if no such entry existed.
    [[nodiscard]] auto remove(const Interval<ValueT>& interval, const DataT& data) -> bool
    {
        std::lock_guard lock(mtx_);
        const size_t oldSize = size_;
        root_ = removeImpl(std::move(root_), interval, data);
        return size_ < oldSize;
    }

    /// @brief Removes all entries from the tree.
    ///
    /// After this call, the tree is empty (size == 0, height == 0).
    void clear()
    {
        std::lock_guard lock(mtx_);
        root_.reset();
        size_ = 0;
    }

    // ── Queries ─────────────────────────────────────────────────────────

    /// @brief Finds all payloads whose intervals overlap with @p query.
    ///
    /// Two intervals overlap if they share at least one point.  The traversal
    /// prunes the left subtree when its max_high <= query.low().
    ///
    /// @param  query The query interval.
    /// @return A vector of matching payloads (order unspecified).
    [[nodiscard]] auto queryOverlap(const Interval<ValueT>& query) const -> std::vector<DataT>
    {
        std::lock_guard lock(mtx_);
        std::vector<DataT> results;
        queryOverlapImpl(root_.get(), query, results);
        return results;
    }

    /// @brief Finds all payloads whose intervals contain @p point.
    ///
    /// An interval contains a point p iff low <= p && p < high (half-open
    /// semantics).
    ///
    /// @param  point The query point.
    /// @return A vector of matching payloads (order unspecified).
    [[nodiscard]] auto queryPoint(const ValueT& point) const -> std::vector<DataT>
    {
        std::lock_guard lock(mtx_);
        std::vector<DataT> results;
        queryPointImpl(root_.get(), point, results);
        return results;
    }

    // ── Status ──────────────────────────────────────────────────────────

    /// @brief Returns the number of entries stored in the tree.
    /// @return Entry count.
    [[nodiscard]] auto size() const -> size_t
    {
        std::lock_guard lock(mtx_);
        return size_;
    }

    /// @brief Checks whether the tree is empty.
    /// @return @c true if there are no entries.
    [[nodiscard]] auto empty() const -> bool
    {
        std::lock_guard lock(mtx_);
        return size_ == 0;
    }

    /// @brief Returns the height of the tree.
    ///
    /// A leaf node has height 1.  An empty tree has height 0.
    ///
    /// @return The height measured in number of levels (0 if empty).
    [[nodiscard]] auto height() const -> size_t
    {
        std::lock_guard lock(mtx_);
        return static_cast<size_t>(getHeight(root_.get()));
    }

    /// @brief Verifies internal consistency (AVL balance + max_high_).
    ///
    /// Performs a depth-first traversal of the entire tree and checks:
    ///   - Every node's AVL balance factor is within [-1, 1]
    ///   - Every node's max_high_ equals
    ///     max(interval.high(), left->max_high_, right->max_high_)
    ///
    /// @return @c true if the tree is internally consistent.
    [[nodiscard]] auto verifyIntegrity() const -> bool
    {
        std::lock_guard lock(mtx_);
        return verifyNode(root_.get());
    }

private:
    // ── Mutable state (guarded by mtx_) ────────────────────────────────

    mutable std::mutex mtx_;
    std::unique_ptr<Node> root_;
    size_t size_ = 0;

    // ═══════════════════════════════════════════════════════════════════════
    //  AVL helpers
    // ═══════════════════════════════════════════════════════════════════════

    /// @brief Returns the height of a node (nullptr-safe).
    /// @param node The node (may be nullptr).
    /// @return The height, or 0 if node is nullptr.
    static auto getHeight(const Node* node) -> int32_t
    {
        return node ? node->height_ : 0;
    }

    /// @brief Computes the balance factor of a node.
    ///
    /// Balance = left_height - right_height.  A node is balanced when
    /// the absolute value is <= 1.
    ///
    /// @param  node The node (must not be nullptr).
    /// @return The balance factor.
    static auto getBalance(const Node* node) -> int32_t
    {
        return getHeight(node->left_.get()) - getHeight(node->right_.get());
    }

    /// @brief Recursively verifies internal consistency of a subtree.
    ///
    /// Checks AVL balance factors and max_high_ augmentation invariants.
    ///
    /// @param node The subtree root (may be nullptr).
    /// @return @c true if the subtree is internally consistent.
    static auto verifyNode(const Node* node) -> bool
    {
        if (!node) return true;

        // ── Check AVL balance factor ──
        const int32_t balance = getBalance(node);
        if (balance > 1 || balance < -1) return false;

        // ── Check max_high_ augmentation ──
        // max_high_ should be max(interval.high(), left->max_high_, right->max_high_)
        ValueT expected_max = node->interval_.high();
        if (node->left_ && node->left_->max_high_ > expected_max)
            expected_max = node->left_->max_high_;
        if (node->right_ && node->right_->max_high_ > expected_max)
            expected_max = node->right_->max_high_;

        if (node->max_high_ != expected_max) return false;

        // ── Recurse ──
        return verifyNode(node->left_.get()) && verifyNode(node->right_.get());
    }

    /// @brief Updates the height and max_high_ of a node from its children.
    ///
    /// This function must be called after any modification to the node's
    /// children.  It computes:
    ///   height_ = 1 + max(left_height, right_height)
    ///   max_high_ = max(interval_.high(), left_max_high, right_max_high)
    ///
    /// @param node The node to update (must not be nullptr).
    static void updateStats(Node* node)
    {
        // ── height ──
        node->height_ = 1 + std::max(getHeight(node->left_.get()),
                                     getHeight(node->right_.get()));

        // ── max_high (augmented metadata) ──
        node->max_high_ = node->interval_.high();
        if (node->left_ && node->left_->max_high_ > node->max_high_)
        {
            node->max_high_ = node->left_->max_high_;
        }
        if (node->right_ && node->right_->max_high_ > node->max_high_)
        {
            node->max_high_ = node->right_->max_high_;
        }
    }

    // ── AVL rotations ───────────────────────────────────────────────────

    /// @brief Performs a right rotation around the given node.
    ///
    ///      node              left
    ///     /    \            /    \
    ///   left   Z    =>     A     node
    ///  /    \                   /    \
    /// A     B                 B      Z
    ///
    /// @param  node The root of the unbalanced subtree.
    /// @return The new root (the left child) after rotation.
    static auto rotateRight(std::unique_ptr<Node> node) -> std::unique_ptr<Node>
    {
        auto left = std::move(node->left_);
        node->left_ = std::move(left->right_);
        left->right_ = std::move(node);

        // Update stats: first the child (now in right of left), then the new root.
        updateStats(left->right_.get());
        updateStats(left.get());
        return left;
    }

    /// @brief Performs a left rotation around the given node.
    ///
    ///    node                 right
    ///   /    \               /    \
    ///  A     right    =>   node    Z
    ///       /    \        /    \
    ///      B      Z      A     B
    ///
    /// @param  node The root of the unbalanced subtree.
    /// @return The new root (the right child) after rotation.
    static auto rotateLeft(std::unique_ptr<Node> node) -> std::unique_ptr<Node>
    {
        auto right = std::move(node->right_);
        node->right_ = std::move(right->left_);
        right->left_ = std::move(node);

        // Update stats: first the child (now in left of right), then the new root.
        updateStats(right->left_.get());
        updateStats(right.get());
        return right;
    }

    /// @brief Rebalances the subtree rooted at @p node using AVL rotations.
    ///
    /// Assumes that both children of @p node are already balanced (their
    /// balance factors are within [-1, 1]).
    ///
    /// @param  node The subtree root to rebalance.
    /// @return The new subtree root after rebalancing (may be the same node).
    static auto rebalance(std::unique_ptr<Node> node) -> std::unique_ptr<Node>
    {
        const int32_t balance = getBalance(node.get());

        // Left-heavy (balance > +1).
        if (balance > 1)
        {
            // Left-right case: the left child is right-heavy.
            if (getBalance(node->left_.get()) < 0)
            {
                node->left_ = rotateLeft(std::move(node->left_));
            }
            // Left-left case (or after LR fix).
            return rotateRight(std::move(node));
        }

        // Right-heavy (balance < -1).
        if (balance < -1)
        {
            // Right-left case: the right child is left-heavy.
            if (getBalance(node->right_.get()) > 0)
            {
                node->right_ = rotateRight(std::move(node->right_));
            }
            // Right-right case (or after RL fix).
            return rotateLeft(std::move(node));
        }

        // Already balanced — no rotation needed.
        return node;
    }

    // ═══════════════════════════════════════════════════════════════════════
    //  Insert / Remove implementation
    // ═══════════════════════════════════════════════════════════════════════

    /// @brief Recursive insertion helper.
    ///
    /// Traverses the tree ordered by (low, high).  If an exact match
    /// (interval + data) is found, the tree is unchanged.  Otherwise a new
    /// node is created, the tree is rebalanced along the insertion path, and
    /// augmented stats are updated.
    ///
    /// @param node     The current subtree root (may be nullptr).
    /// @param interval The interval to insert.
    /// @param data     The payload to insert (moved into the tree).
    /// @return The (potentially new) subtree root after insertion and
    ///         rebalancing.
    auto insertImpl(std::unique_ptr<Node> node,
                    const Interval<ValueT>& interval,
                    DataT data) -> std::unique_ptr<Node>
    {
        // ── Base case: create a new node ──
        if (!node)
        {
            ++size_;
            return std::make_unique<Node>(interval, std::move(data));
        }

        // ── Check for duplicate (exact match) ──
        if (node->interval_.low() == interval.low() &&
            node->interval_.high() == interval.high() &&
            node->data_ == data)
        {
            return node;
        }

        // ── BST traversal (ordered by low, then high) ──
        if (interval.low() < node->interval_.low() ||
            (interval.low() == node->interval_.low() &&
             interval.high() < node->interval_.high()))
        {
            node->left_ = insertImpl(std::move(node->left_), interval, std::move(data));
        }
        else
        {
            node->right_ = insertImpl(std::move(node->right_), interval, std::move(data));
        }

        // ── Update stats and rebalance ──
        updateStats(node.get());
        return rebalance(std::move(node));
    }

    /// @brief Returns the node with the minimum interval (by low) in a subtree.
    /// @param node The subtree root (must not be nullptr).
    /// @return Pointer to the leftmost node.
    static auto findMin(Node* node) -> Node*
    {
        while (node && node->left_)
        {
            node = node->left_.get();
        }
        return node;
    }

    /// @brief Recursive removal helper.
    ///
    /// Searches for an exact (interval, data) match and removes the
    /// corresponding node.  Handles all three cases: leaf, single child,
    /// and two children (replacement with inorder successor).
    ///
    /// @param node     The current subtree root (may be nullptr).
    /// @param interval The interval to match.
    /// @param data     The payload to match.
    /// @return The (potentially new) subtree root after removal and
    ///         rebalancing, or nullptr if the subtree became empty.
    auto removeImpl(std::unique_ptr<Node> node,
                    const Interval<ValueT>& interval,
                    const DataT& data) -> std::unique_ptr<Node>
    {
        if (!node)
        {
            return nullptr;
        }

        // ── Search for the node to delete ──
        if (interval.low() < node->interval_.low() ||
            (interval.low() == node->interval_.low() &&
             interval.high() < node->interval_.high()))
        {
            node->left_ = removeImpl(std::move(node->left_), interval, data);
        }
        else if (interval.low() > node->interval_.low() ||
                 interval.high() > node->interval_.high() ||
                 !(node->data_ == data))
        {
            // Continue right when:
            // - interval is greater, OR
            // - intervals match but data differs (same-interval entries are
            //   placed in the right subtree).
            node->right_ = removeImpl(std::move(node->right_), interval, data);
        }
        else
        {
            // ── Found the exact match — delete this node ──

            // Case 1: leaf or single child
            if (!node->left_)
            {
                --size_;
                return std::move(node->right_);
            }
            if (!node->right_)
            {
                --size_;
                return std::move(node->left_);
            }

            // Case 2: two children — replace with inorder successor
            // (the minimum node in the right subtree).
            // NOTE: size_ is NOT decremented here because the current node
            // is being overwritten, not removed.  The recursive call below
            // will remove the successor node and decrement size_ once.
            Node* successor = findMin(node->right_.get());
            node->interval_ = successor->interval_;
            node->data_ = successor->data_;

            // Recursively delete the successor from the right subtree.
            node->right_ = removeImpl(std::move(node->right_),
                                      successor->interval_,
                                      successor->data_);
        }

        // ── Update stats and rebalance ──
        updateStats(node.get());
        return rebalance(std::move(node));
    }

    // ═══════════════════════════════════════════════════════════════════════
    //  Query implementation
    // ═══════════════════════════════════════════════════════════════════════

    /// @brief Recursive overlap query implementation.
    ///
    /// Always checks the current node.  The left subtree is traversed only
    /// when its max_high exceeds the query's low (pruning optimisation).
    /// The right subtree is always traversed (no pruning possible for
    /// interval trees ordered by low).
    ///
    /// @param node    The current node (may be nullptr).
    /// @param query   The query interval.
    /// @param results Output vector collecting matching payloads.
    static void queryOverlapImpl(const Node* node,
                                 const Interval<ValueT>& query,
                                 std::vector<DataT>& results)
    {
        if (!node)
        {
            return;
        }

        // ── Check current node ──
        if (node->interval_.overlaps(query))
        {
            results.push_back(node->data_);
        }

        // ── Left subtree (pruned) ──
        if (node->left_ && node->left_->max_high_ > query.low())
        {
            queryOverlapImpl(node->left_.get(), query, results);
        }

        // ── Right subtree (always traverse) ──
        if (node->right_)
        {
            queryOverlapImpl(node->right_.get(), query, results);
        }
    }

    /// @brief Recursive point query implementation.
    ///
    /// Checks whether the current node's interval contains the point.
    /// Left subtree is traversed only when its max_high exceeds the point.
    /// Right subtree is always traversed (intervals with equal low may also
    /// contain the point).
    ///
    /// @param node    The current node (may be nullptr).
    /// @param point   The query point.
    /// @param results Output vector collecting matching payloads.
    static void queryPointImpl(const Node* node,
                               const ValueT& point,
                               std::vector<DataT>& results)
    {
        if (!node)
        {
            return;
        }

        // ── Check current node ──
        if (node->interval_.contains(point))
        {
            results.push_back(node->data_);
        }

        // ── Left subtree (pruned) ──
        // Intervals ending after the point may contain it.
        if (node->left_ && node->left_->max_high_ > point)
        {
            queryPointImpl(node->left_.get(), point, results);
        }

        // ── Right subtree ──
        // Intervals with equal or higher low may still contain the point
        // if their low <= point.  No pruning is possible.
        if (node->right_)
        {
            queryPointImpl(node->right_.get(), point, results);
        }
    }
};

} // namespace common::data_structure::spatial
