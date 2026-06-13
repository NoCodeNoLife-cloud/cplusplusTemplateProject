/**
 * @file BTree.hpp
 * @brief B-tree with configurable minimum degree — O(log n) all operations
 * @details A self-balancing tree that maintains sorted data with multiple keys
 *          per node.  Each node can hold multiple keys and child pointers,
 *          minimising I/O for block-oriented storage systems.  This
 *          implementation follows CLRS (Chapter 18) with configurable minimum
 *          degree t: internal nodes have [t-1, 2t-1] keys.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Complexity
 * - search / insert / remove: O(log n) worst-case
 * - Order t = 100: typical depth <= 3 for 10^6 elements
 *
 * @par Reference
 * Cormen, Leiserson, Rivest, Stein, "Introduction to Algorithms" (CLRS),
 * Chapter 18: B-Trees.
 */

#pragma once
#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace common::data_structure::tree
{
    /// @brief A B-tree (balanced tree) with configurable minimum degree
    /// @details Properties (t = Order, minimum degree >= 2):
    ///          - Every node has at most 2t-1 keys
    ///          - Every non-root node has at least t-1 keys
    ///          - Every node has at most 2t children
    ///          - Every non-root, non-leaf node has at least t children
    ///          - All leaves appear at the same depth
    /// @tparam T The type of data stored. Must be comparable.
    /// @tparam Compare Comparison functor (default std::less<T>).
    /// @tparam Order Minimum degree t (t >= 2). Default 3.
    template <typename T, typename Compare = std::less<T>, int32_t Order = 3>
    class BTree
    {
        static_assert(Order >= 2, "BTree minimum degree must be at least 2");

    private:
        /// @brief Internal B-tree node
        struct Node
        {
            std::vector<T> keys_;                             ///< Sorted keys in this node
            std::vector<std::unique_ptr<Node>> children_;     ///< Child pointers (size = count_ + 1 for internal)
            int32_t count_;                                   ///< Number of valid keys
            bool leaf_;                                       ///< True if this is a leaf node

            explicit Node(bool leaf)
                : count_(0)
                , leaf_(leaf)
            {
                keys_.reserve(2 * Order);
                if (!leaf)
                    children_.reserve(2 * Order + 1);
            }

            /// @brief Check if the node is full (has the maximum allowed keys)
            [[nodiscard]] bool isFull() const { return count_ == 2 * Order - 1; }

            /// @brief Check if the node has fewer than the minimum allowed keys
            [[nodiscard]] bool isUnderflow() const { return count_ < Order - 1; }

            /// @brief Find the index of the first key >= the given key
            [[nodiscard]] int32_t findKey(const T& key, const Compare& comp) const
            {
                int32_t idx = 0;
                while (idx < count_ && comp(keys_[idx], key))
                    ++idx;
                return idx;
            }
        };

        std::unique_ptr<Node> root_;
        int32_t size_;
        Compare comp_;

        // ── Insert helpers ──────────────────────────────────────────────

        /// @brief Split the child at [idx] of parent, promoting the median key
        void splitChild(Node* parent, int32_t idx)
        {
            auto child = std::move(parent->children_[idx]);
            auto sibling = std::make_unique<Node>(child->leaf_);

            // Copy the upper half of keys to the new sibling
            sibling->count_ = Order - 1;
            // Manual loop to avoid MSVC checked-iterator crash with assign(begin+5) on 5-element vector
            sibling->keys_.reserve(2 * Order);
            for (int32_t i = Order; i < 2 * Order - 1; ++i)
                sibling->keys_.push_back(child->keys_[i]);

            // Copy the upper half of children (if internal)
            if (!child->leaf_)
            {
                sibling->children_.reserve(2 * Order + 1);
                for (int32_t i = Order; i <= 2 * Order - 1; ++i)
                    sibling->children_.push_back(std::move(child->children_[i]));
            }

            // The median key moves up to the parent
            T median = std::move(child->keys_[Order - 1]);
            child->count_ = Order - 1;
            child->keys_.resize(Order - 1);
            if (!child->leaf_)
                child->children_.resize(Order);

            // Put the left child back into parent before inserting the sibling
            parent->children_[idx] = std::move(child);
            parent->keys_.insert(parent->keys_.begin() + idx, std::move(median));
            parent->children_.insert(parent->children_.begin() + idx + 1, std::move(sibling));
            ++parent->count_;
        }

        /// @brief Recursively insert into a non-full node
        void insertNonFull(Node* node, const T& key)
        {
            if (node->leaf_)
            {
                // Find the insertion position; also check for duplicates
                int32_t idx = 0;
                while (idx < node->count_ && comp_(node->keys_[idx], key))
                    ++idx;

                // If key already exists, no duplicate
                if (idx < node->count_ && !comp_(node->keys_[idx], key) && !comp_(key, node->keys_[idx]))
                    return;

                // Insert the key in sorted order at position idx
                node->keys_.resize(node->count_ + 1);
                for (int32_t i = node->count_ - 1; i >= idx; --i)
                    node->keys_[i + 1] = std::move(node->keys_[i]);
                node->keys_[idx] = key;
                ++node->count_;
                ++size_;
                return;
            }

            // Internal node: find the child to descend into
            int32_t idx = node->findKey(key, comp_);

            // If the key already exists at this node, no duplicate
            if (idx < node->count_)
            {
                if (!comp_(node->keys_[idx], key) && !comp_(key, node->keys_[idx]))
                    return;
            }

            // Ensure the target child is not full before descending
            if (node->children_[idx]->isFull())
            {
                splitChild(node, idx);
                // After split, determine which child contains the correct range
                if (comp_(node->keys_[idx], key))
                    ++idx;
            }
            insertNonFull(node->children_[idx].get(), key);
        }

        // ── Remove helpers ──────────────────────────────────────────────

        /// @brief Get the predecessor key (maximum key in the left subtree)
        T getPredecessor(Node* node) const
        {
            while (!node->leaf_)
                node = node->children_[node->count_].get();
            return node->keys_[node->count_ - 1];
        }

        /// @brief Get the successor key (minimum key in the right subtree)
        T getSuccessor(Node* node) const
        {
            while (!node->leaf_)
                node = node->children_[0].get();
            return node->keys_[0];
        }

        /// @brief Borrow a key from child[idx-1] to child[idx] via parent
        void borrowFromPrev(Node* parent, int32_t idx)
        {
            auto child = parent->children_[idx].get();
            auto sibling = parent->children_[idx - 1].get();

            // Move parent key down to the front of child
            child->keys_.insert(child->keys_.begin(), T{});
            child->keys_[0] = std::move(parent->keys_[idx - 1]);
            ++child->count_;

            if (!child->leaf_)
            {
                // Move sibling's last child to child's front
                child->children_.insert(child->children_.begin(), nullptr);
                child->children_[0] = std::move(sibling->children_[sibling->count_]);
            }

            // Move sibling's last key up to parent
            parent->keys_[idx - 1] = std::move(sibling->keys_[sibling->count_ - 1]);
            --sibling->count_;
            sibling->keys_.pop_back();
            if (!sibling->leaf_)
                sibling->children_.pop_back();
        }

        /// @brief Borrow a key from child[idx+1] to child[idx] via parent
        void borrowFromNext(Node* parent, int32_t idx)
        {
            auto child = parent->children_[idx].get();
            auto sibling = parent->children_[idx + 1].get();

            child->keys_.push_back(std::move(parent->keys_[idx]));
            ++child->count_;

            parent->keys_[idx] = std::move(sibling->keys_[0]);

            // Shift sibling's keys left
            for (int32_t i = 0; i < sibling->count_ - 1; ++i)
                sibling->keys_[i] = std::move(sibling->keys_[i + 1]);
            --sibling->count_;
            sibling->keys_.pop_back();

    if (!child->leaf_)
    {
        child->children_.push_back(std::move(sibling->children_[0]));
        // Shift children left by 1 (i <= count_ to move the last child into the vacated slot)
        for (int32_t i = 0; i <= sibling->count_; ++i)
            sibling->children_[i] = std::move(sibling->children_[i + 1]);
        sibling->children_.pop_back();
    }
}

/// @brief Merge child[idx] with child[idx+1], pulling the separator key from parent
        void merge(Node* parent, int32_t idx)
        {
            auto left = std::move(parent->children_[idx]);
            auto right = std::move(parent->children_[idx + 1]);

            // Pull the separator key from parent into left
            left->keys_.push_back(std::move(parent->keys_[idx]));
            ++left->count_;

            // Copy all keys from right to left
            for (int32_t i = 0; i < right->count_; ++i)
                left->keys_.push_back(std::move(right->keys_[i]));

            if (!left->leaf_)
            {
                // Copy all children from right to left
                for (int32_t i = 0; i <= right->count_; ++i)
                    left->children_.push_back(std::move(right->children_[i]));
            }

            left->count_ += right->count_;

            // Remove the separator key from parent
            for (int32_t i = idx; i < parent->count_ - 1; ++i)
                parent->keys_[i] = std::move(parent->keys_[i + 1]);
            parent->keys_.pop_back();

            // Put the merged left child back at its original slot, then compact children
            parent->children_[idx] = std::move(left);
            for (int32_t i = idx + 1; i < parent->count_; ++i)
                parent->children_[i] = std::move(parent->children_[i + 1]);
            parent->children_.pop_back();
            --parent->count_;
        }

        /// @brief Ensure child[idx] has at least Order keys (borrow or merge)
        void fill(Node* parent, int32_t idx)
        {
            if (idx > 0 && parent->children_[idx - 1]->count_ >= Order)
                borrowFromPrev(parent, idx);
            else if (idx < parent->count_ && parent->children_[idx + 1]->count_ >= Order)
                borrowFromNext(parent, idx);
            else
            {
                if (idx < parent->count_)
                    merge(parent, idx);
                else
                    merge(parent, idx - 1);
            }
        }

        /// @brief Recursively remove a key from a subtree rooted at node
        void removeFromNode(Node* node, const T& key)
        {
            int32_t idx = node->findKey(key, comp_);

            if (node->leaf_)
            {
                // Key not found
                if (idx >= node->count_ || comp_(key, node->keys_[idx]) || comp_(node->keys_[idx], key))
                    return;

                // Remove the key from the leaf
                for (int32_t i = idx; i < node->count_ - 1; ++i)
                    node->keys_[i] = std::move(node->keys_[i + 1]);
                node->keys_.pop_back();
                --node->count_;
                --size_;
                return;
            }

            // Internal node
            if (idx < node->count_ && !comp_(node->keys_[idx], key) && !comp_(key, node->keys_[idx]))
            {
                // Case 1: key is found at this internal node
                if (node->children_[idx]->count_ >= Order)
                {
                    // Case 1a: replace with predecessor
                    T pred = getPredecessor(node->children_[idx].get());
                    node->keys_[idx] = pred;
                    removeFromNode(node->children_[idx].get(), pred);
                }
                else if (node->children_[idx + 1]->count_ >= Order)
                {
                    // Case 1b: replace with successor
                    T succ = getSuccessor(node->children_[idx + 1].get());
                    node->keys_[idx] = succ;
                    removeFromNode(node->children_[idx + 1].get(), succ);
                }
                else
                {
                    // Case 1c: merge children[idx] and children[idx+1], then recurse
                    merge(node, idx);
                    removeFromNode(node->children_[idx].get(), key);
                }
            }
            else
            {
                // Case 2: key is not in this node, descend to the appropriate child
                // findKey returns the index where key belongs: children[idx] holds key range
                int32_t childIdx = idx;

                // Ensure the target child has at least Order keys before descending
                if (node->children_[childIdx]->count_ == Order - 1)
                    fill(node, childIdx);

                // After a merge operation, the child count decreases
                if (childIdx > node->count_)
                    childIdx = node->count_;

                removeFromNode(node->children_[childIdx].get(), key);
            }
        }

        // ── Traversal ───────────────────────────────────────────────────

        void inorder(Node* node, std::vector<T>& result) const
        {
            if (!node) return;
            if (node->leaf_)
            {
                for (int32_t i = 0; i < node->count_; ++i)
                    result.push_back(node->keys_[i]);
            }
            else
            {
                for (int32_t i = 0; i < node->count_; ++i)
                {
                    inorder(node->children_[i].get(), result);
                    result.push_back(node->keys_[i]);
                }
                inorder(node->children_[node->count_].get(), result);
            }
        }

        [[nodiscard]] int32_t computeHeight(Node* node) const
        {
            if (!node) return 0;
            if (node->leaf_) return 1;
            return 1 + computeHeight(node->children_[0].get());
        }

        // ── Verification ────────────────────────────────────────────────

        bool verifyNode(Node* node, const T* lower, const T* upper) const
        {
            if (!node) return true;

            // Key count bounds
            if (node != root_.get() && node->count_ < Order - 1)
                return false;
            if (node->count_ > 2 * Order - 1)
                return false;

            // Keys are sorted and within bounds
            for (int32_t i = 0; i < node->count_; ++i)
            {
                if (lower && comp_(node->keys_[i], *lower))
                    return false;
                if (upper && comp_(*upper, node->keys_[i]))
                    return false;
                if (i > 0 && !comp_(node->keys_[i - 1], node->keys_[i]))
                    return false;
            }

            if (node->leaf_)
            {
                if (!node->children_.empty())
                    return false;
                return true;
            }

            // Internal node must have exactly count_ + 1 children
            if (static_cast<int32_t>(node->children_.size()) != node->count_ + 1)
                return false;
            if (node->count_ < 0)
                return false;

            for (int32_t i = 0; i <= node->count_; ++i)
            {
                const T* newLower = (i == 0) ? lower : &node->keys_[i - 1];
                const T* newUpper = (i == node->count_) ? upper : &node->keys_[i];
                if (!verifyNode(node->children_[i].get(), newLower, newUpper))
                    return false;
            }
            return true;
        }

    public:
        /// @brief Default constructor
        BTree() : root_(nullptr), size_(0) {}

        /// @brief Move constructor
        BTree(BTree&& other) noexcept
            : root_(std::move(other.root_))
            , size_(std::exchange(other.size_, 0))
        {
        }

        /// @brief Move assignment operator
        BTree& operator=(BTree&& other) noexcept
        {
            if (this != &other)
            {
                root_ = std::move(other.root_);
                size_ = std::exchange(other.size_, 0);
            }
            return *this;
        }

        BTree(const BTree&) = delete;
        BTree& operator=(const BTree&) = delete;

        ~BTree() = default;

        // ── Public API ──────────────────────────────────────────────────

        /// @brief Insert a key into the B-tree
        /// @param key The key to insert. No effect if key already exists.
        void insert(const T& key)
        {
            if (!root_)
            {
                root_ = std::make_unique<Node>(true);
                root_->keys_.push_back(key);
                root_->count_ = 1;
                ++size_;
                return;
            }

            // If the root is full, grow the tree one level
            if (root_->isFull())
            {
                auto newRoot = std::make_unique<Node>(false);
                newRoot->children_.push_back(std::move(root_));
                root_ = std::move(newRoot);
                splitChild(root_.get(), 0);
            }
            insertNonFull(root_.get(), key);
        }

        /// @brief Remove a key from the B-tree
        /// @param key The key to remove
        /// @return true if the key was found and removed, false otherwise
        bool remove(const T& key)
        {
            if (!root_) return false;
            int32_t oldSize = size_;
            removeFromNode(root_.get(), key);

            // Shrink the tree if the root becomes empty
            if (root_->count_ == 0)
            {
                if (root_->leaf_)
                    root_.reset();
                else if (!root_->children_.empty())
                    root_ = std::move(root_->children_[0]);
            }
            return oldSize != size_;
        }

        /// @brief Check if the tree contains a key
        /// @param key The key to search for
        /// @return true if the key exists
        [[nodiscard]] bool contains(const T& key) const
        {
            Node* node = root_.get();
            while (node)
            {
                int32_t idx = node->findKey(key, comp_);
                if (idx < node->count_ && !comp_(node->keys_[idx], key) && !comp_(key, node->keys_[idx]))
                    return true;
                if (node->leaf_) return false;
                node = node->children_[idx].get();
            }
            return false;
        }

        /// @brief Get the number of elements in the tree
        [[nodiscard]] int32_t size() const { return size_; }

        /// @brief Check if the tree is empty
        [[nodiscard]] bool empty() const { return size_ == 0; }

        /// @brief Remove all elements from the tree
        void clear()
        {
            root_.reset();
            size_ = 0;
        }

        /// @brief Get all keys in sorted order
        [[nodiscard]] std::vector<T> inorder() const
        {
            std::vector<T> result;
            inorder(root_.get(), result);
            return result;
        }

        /// @brief Get the height of the tree (number of levels)
        [[nodiscard]] int32_t height() const
        {
            return computeHeight(root_.get());
        }

        /// @brief Verify all B-tree structural invariants
        /// @details Checks key count bounds, sorted order, subtree bounds, and leaf structure.
        [[nodiscard]] bool verify() const
        {
            if (!root_) return true;
            return verifyNode(root_.get(), nullptr, nullptr);
        }
    };
}
