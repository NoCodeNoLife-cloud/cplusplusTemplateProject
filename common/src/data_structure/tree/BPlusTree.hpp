/**
 * @file BPlusTree.hpp
 * @brief BPlusTree class declaration
 * @details A B+Tree is a variant of the B-tree where all values are stored at the leaf
 *          level, while internal nodes contain only routing keys. Leaf nodes are linked
 *          together forming a sequential list, enabling efficient range queries.
 *          This implementation uses the same minimum degree (Order) convention as BTree.
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
    /// @brief A B+Tree with configurable minimum degree
    /// @details All values reside in leaf nodes. Internal nodes store only routing keys.
    ///          Leaf nodes are doubly linked for efficient range scans. Search always
    ///          traverses to a leaf node.
    /// @tparam K Key type. Must be comparable.
    /// @tparam V Value type.
    /// @tparam Compare Comparison functor for keys (default std::less<K>).
    /// @tparam Order Minimum degree t (t >= 2). Default 3.
    template <typename K, typename V, typename Compare = std::less<K>, int32_t Order = 3>
    class BPlusTree
    {
        static_assert(Order >= 2, "BPlusTree minimum degree must be at least 2");

    private:
        /// @brief Internal tree node
        struct Node
        {
            std::vector<K> keys_;                             ///< Routing keys (internal) or data keys (leaf)
            std::vector<V> values_;                           ///< Values (only for leaf nodes)
            std::vector<std::unique_ptr<Node>> children_;     ///< Child pointers (only for internal nodes)
            bool leaf_;
            Node* prev_;     ///< Previous leaf (only for leaf nodes)
            Node* next_;     ///< Next leaf (only for leaf nodes)
            int32_t count_;

            explicit Node(bool leaf)
                : leaf_(leaf)
                , prev_(nullptr)
                , next_(nullptr)
                , count_(0)
            {
                keys_.reserve(2 * Order);
                if (leaf)
                    values_.reserve(2 * Order);
                else
                    children_.reserve(2 * Order + 1);
            }

            [[nodiscard]] bool isFull() const { return count_ == 2 * Order - 1; }
            [[nodiscard]] bool isUnderflow() const { return count_ < Order - 1; }

            [[nodiscard]] int32_t findKey(const K& key, const Compare& comp) const
            {
                int32_t idx = 0;
                while (idx < count_ && comp(keys_[idx], key))
                    ++idx;
                return idx;
            }
        };

        std::unique_ptr<Node> root_;
        Node* head_;     ///< First leaf node (for sequential scan)
        int32_t size_;
        Compare comp_;

        // ── Leaf helpers ────────────────────────────────────────────────

        /// @brief Find the leaf node that should contain the given key
        Node* findLeaf(const K& key) const
        {
            if (!root_) return nullptr;
            Node* node = root_.get();
            while (!node->leaf_)
            {
                int32_t idx = 0;
                while (idx < node->count_ && !comp_(key, node->keys_[idx]))
                    ++idx;
                node = node->children_[idx].get();
            }
            return node;
        }

        // ── Split helpers ───────────────────────────────────────────────

        /// @brief Split a full leaf child at [idx] in the parent
        /// @details After split, the left child retains Order keys (indices 0..Order-1),
        ///          the right sibling gets Order-1 keys (indices Order..2*Order-2),
        ///          and the first key of the sibling becomes the routing key in the parent.
        void splitLeaf(Node* parent, int32_t idx)
        {
            auto child = std::move(parent->children_[idx]);
            auto sibling = std::make_unique<Node>(true);

            sibling->count_ = Order - 1;
            sibling->keys_.assign(child->keys_.begin() + Order, child->keys_.end());
            sibling->values_.assign(child->values_.begin() + Order, child->values_.end());

            // Left child retains Order keys (the upper bound, not Order-1)
            child->count_ = Order;
            child->keys_.resize(Order);
            child->values_.resize(Order);

            // Link the leaf list: sibling is inserted after child
            sibling->next_ = child->next_;
            sibling->prev_ = child.get();
            if (child->next_)
                child->next_->prev_ = sibling.get();
            child->next_ = sibling.get();
            // head_ stays unchanged: child (left half) still has the smallest keys

            // The first key of the new sibling becomes the routing key in the parent
            K routingKey = sibling->keys_[0];

            // Put the left child back into parent before inserting the sibling
            parent->children_[idx] = std::move(child);
            parent->keys_.insert(parent->keys_.begin() + idx, std::move(routingKey));
            parent->children_.insert(parent->children_.begin() + idx + 1, std::move(sibling));
            ++parent->count_;
        }

        /// @brief Split a full internal child at [idx] in the parent
        /// @details The median key (at index Order-1) moves up to the parent.
        ///          The left child retains Order-1 keys, the right sibling gets Order-1 keys.
        void splitInternal(Node* parent, int32_t idx)
        {
            auto child = std::move(parent->children_[idx]);
            auto sibling = std::make_unique<Node>(false);

            sibling->count_ = Order - 1;
            // Copy the upper half of keys to sibling (manual loop avoids MSVC checked-iterator crash with assign)
            sibling->keys_.reserve(2 * Order);
            for (int32_t i = Order; i < 2 * Order - 1; ++i)
                sibling->keys_.push_back(child->keys_[i]);

            // Move the upper half of children to sibling
            sibling->children_.reserve(2 * Order + 1);
            for (int32_t i = Order; i <= 2 * Order - 1; ++i)
                sibling->children_.push_back(std::move(child->children_[i]));
            child->children_.resize(Order);

            // The median key moves up (but is NOT kept in either child)
            K median = std::move(child->keys_[Order - 1]);
            child->count_ = Order - 1;
            child->keys_.resize(Order - 1);

            // Put the left child back into parent before inserting the sibling
            parent->children_[idx] = std::move(child);
            parent->keys_.insert(parent->keys_.begin() + idx, std::move(median));
            parent->children_.insert(parent->children_.begin() + idx + 1, std::move(sibling));
            ++parent->count_;
        }

        // ── Insert ──────────────────────────────────────────────────────

        void insertNonFull(Node* node, const K& key, const V& value)
        {
            if (node->leaf_)
            {
                // Check for duplicate key → update value
                for (int32_t i = 0; i < node->count_; ++i)
                {
                    if (!comp_(node->keys_[i], key) && !comp_(key, node->keys_[i]))
                    {
                        node->values_[i] = value;
                        return;
                    }
                }

                // Insert key-value pair in sorted order
                int32_t idx = node->count_ - 1;
                node->keys_.resize(node->count_ + 1);
                node->values_.resize(node->count_ + 1);
                while (idx >= 0 && comp_(key, node->keys_[idx]))
                {
                    node->keys_[idx + 1] = std::move(node->keys_[idx]);
                    node->values_[idx + 1] = std::move(node->values_[idx]);
                    --idx;
                }
                node->keys_[idx + 1] = key;
                node->values_[idx + 1] = value;
                ++node->count_;
                ++size_;
                return;
            }

            // Internal node: find the child to descend into
            // child[0] has keys < keys_[0]
            // child[i] has keys < keys_[i] (where keys_[i-1] <= key)
            int32_t childIdx = 0;
            for (int32_t i = 0; i < node->count_; ++i)
            {
                if (comp_(key, node->keys_[i]))
                    break;
                ++childIdx;
            }

            if (node->children_[childIdx]->isFull())
            {
                if (node->children_[childIdx]->leaf_)
                    splitLeaf(node, childIdx);
                else
                    splitInternal(node, childIdx);

                // After split, decide which child the key should go into
                if (childIdx < node->count_ && comp_(node->keys_[childIdx], key))
                    ++childIdx;
            }
            insertNonFull(node->children_[childIdx].get(), key, value);
        }

        // ── Remove ──────────────────────────────────────────────────────

        /// @brief Borrow a key-value pair from the next leaf sibling
        void borrowLeafFromNext(Node* parent, int32_t idx)
        {
            auto child = parent->children_[idx].get();
            auto sibling = parent->children_[idx + 1].get();

            // Move the first key-value of sibling to the end of child
            child->keys_.push_back(std::move(sibling->keys_[0]));
            child->values_.push_back(std::move(sibling->values_[0]));
            ++child->count_;

            // Shift sibling's keys left
            for (int32_t i = 0; i < sibling->count_ - 1; ++i)
            {
                sibling->keys_[i] = std::move(sibling->keys_[i + 1]);
                sibling->values_[i] = std::move(sibling->values_[i + 1]);
            }
            --sibling->count_;
            sibling->keys_.pop_back();
            sibling->values_.pop_back();

            // Update the routing key in the parent
            parent->keys_[idx] = sibling->keys_[0];
        }

        /// @brief Borrow from the previous leaf sibling
        void borrowLeafFromPrev(Node* parent, int32_t idx)
        {
            auto child = parent->children_[idx].get();
            auto sibling = parent->children_[idx - 1].get();

            // Move the last key-value of sibling to the front of child
            child->keys_.insert(child->keys_.begin(), std::move(sibling->keys_[sibling->count_ - 1]));
            child->values_.insert(child->values_.begin(), std::move(sibling->values_[sibling->count_ - 1]));
            ++child->count_;
            --sibling->count_;
            sibling->keys_.pop_back();
            sibling->values_.pop_back();

            // Update the routing key in the parent
            parent->keys_[idx - 1] = child->keys_[0];
        }

        /// @brief Merge two sibling leaves
        void mergeLeaf(Node* parent, int32_t idx)
        {
            auto left = std::move(parent->children_[idx]);
            auto right = std::move(parent->children_[idx + 1]);

            // Copy keys and values from right to left
            for (int32_t i = 0; i < right->count_; ++i)
            {
                left->keys_.push_back(std::move(right->keys_[i]));
                left->values_.push_back(std::move(right->values_[i]));
            }
            left->count_ += right->count_;

            // Update leaf list
            left->next_ = right->next_;
            if (right->next_)
                right->next_->prev_ = left.get();

            // Remove routing key from parent
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

        /// @brief Borrow a routing key from an internal node's sibling
        void borrowInternalFromPrev(Node* parent, int32_t idx)
        {
            auto child = parent->children_[idx].get();
            auto sibling = parent->children_[idx - 1].get();

            child->keys_.insert(child->keys_.begin(), std::move(parent->keys_[idx - 1]));
            ++child->count_;

            parent->keys_[idx - 1] = std::move(sibling->keys_[sibling->count_ - 1]);
            --sibling->count_;
            sibling->keys_.pop_back();

            if (!child->leaf_)
            {
                child->children_.insert(child->children_.begin(), std::move(sibling->children_[sibling->count_]));
                sibling->children_.pop_back();
            }
        }

        void borrowInternalFromNext(Node* parent, int32_t idx)
        {
            auto child = parent->children_[idx].get();
            auto sibling = parent->children_[idx + 1].get();

            child->keys_.push_back(std::move(parent->keys_[idx]));
            ++child->count_;

            parent->keys_[idx] = std::move(sibling->keys_[0]);
            for (int32_t i = 0; i < sibling->count_ - 1; ++i)
                sibling->keys_[i] = std::move(sibling->keys_[i + 1]);
            --sibling->count_;
            sibling->keys_.pop_back();

            if (!child->leaf_)
            {
                child->children_.push_back(std::move(sibling->children_[0]));
                for (int32_t i = 0; i <= sibling->count_; ++i)
                    sibling->children_[i] = std::move(sibling->children_[i + 1]);
                sibling->children_.pop_back();
            }
        }

        void mergeInternal(Node* parent, int32_t idx)
        {
            auto left = std::move(parent->children_[idx]);
            auto right = std::move(parent->children_[idx + 1]);

            left->keys_.push_back(std::move(parent->keys_[idx]));
            ++left->count_;

            for (int32_t i = 0; i < right->count_; ++i)
                left->keys_.push_back(std::move(right->keys_[i]));
            left->count_ += right->count_;

            for (int32_t i = 0; i <= right->count_; ++i)
                left->children_.push_back(std::move(right->children_[i]));

            // Remove routing key from parent
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

        void fillInternal(Node* parent, int32_t idx)
        {
            if (idx > 0 && parent->children_[idx - 1]->count_ >= Order)
                borrowInternalFromPrev(parent, idx);
            else if (idx < parent->count_ && parent->children_[idx + 1]->count_ >= Order)
                borrowInternalFromNext(parent, idx);
            else
            {
                if (idx < parent->count_)
                    mergeInternal(parent, idx);
                else
                    mergeInternal(parent, idx - 1);
            }
        }

        /// @brief Recursively remove a key from the B+Tree
        /// @details Uses the same top-down approach as BTree: ensures the target child
        ///          has enough keys (via borrow or merge) before descending.
        void removeRecursive(Node* node, const K& key)
        {
            if (node->leaf_)
            {
                // Find and remove the key from the leaf
                int32_t idx = node->findKey(key, comp_);
                if (idx >= node->count_ || comp_(key, node->keys_[idx]) || comp_(node->keys_[idx], key))
                    return; // Key not found

                for (int32_t i = idx; i < node->count_ - 1; ++i)
                {
                    node->keys_[i] = std::move(node->keys_[i + 1]);
                    node->values_[i] = std::move(node->values_[i + 1]);
                }
                node->keys_.pop_back();
                node->values_.pop_back();
                --node->count_;
                --size_;
                return;
            }

            // Internal node: find the child to descend into.
            // keys_[i] is the first key of children_[i+1].
            // child[0] has keys < keys_[0], child[i] has keys >= keys_[i-1] and < keys_[i].
            int32_t childIdx = 0;
            while (childIdx < node->count_ && !comp_(key, node->keys_[childIdx]))
                ++childIdx;

            // Ensure the target child has enough keys before descending
            if (node->children_[childIdx]->count_ == Order - 1)
            {
                const bool childIsLeaf = node->children_[childIdx]->leaf_;

                if (childIdx > 0 && node->children_[childIdx - 1]->count_ >= Order)
                {
                    if (childIsLeaf)
                        borrowLeafFromPrev(node, childIdx);
                    else
                        borrowInternalFromPrev(node, childIdx);
                }
                else if (childIdx < node->count_ && node->children_[childIdx + 1]->count_ >= Order)
                {
                    if (childIsLeaf)
                        borrowLeafFromNext(node, childIdx);
                    else
                        borrowInternalFromNext(node, childIdx);
                }
                else
                {
                    // Must merge
                    if (childIdx < node->count_)
                    {
                        if (childIsLeaf)
                            mergeLeaf(node, childIdx);
                        else
                            mergeInternal(node, childIdx);
                    }
                    else
                    {
                        if (node->children_[childIdx - 1]->leaf_)
                            mergeLeaf(node, childIdx - 1);
                        else
                            mergeInternal(node, childIdx - 1);
                        --childIdx;
                    }
                }
            }

            // After a merge, node->count_ decreased, so childIdx might need adjustment
            if (childIdx > node->count_)
                childIdx = node->count_;

            removeRecursive(node->children_[childIdx].get(), key);
        }

        // ── Traversal ───────────────────────────────────────────────────

        void inorderLeaves(Node* leaf, std::vector<std::pair<K, V>>& result) const
        {
            while (leaf)
            {
                for (int32_t i = 0; i < leaf->count_; ++i)
                    result.emplace_back(leaf->keys_[i], leaf->values_[i]);
                leaf = leaf->next_;
            }
        }

        // ── Verification ────────────────────────────────────────────────

        bool verifyNode(Node* node, const K* lower, const K* upper) const
        {
            if (!node) return true;

            if (node != root_.get() && node->count_ < Order - 1)
                return false;
            if (node->count_ > 2 * Order - 1)
                return false;

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
                if (static_cast<int32_t>(node->values_.size()) != node->count_)
                    return false;
                // Verify leaf list links (forward consistency)
                if (node->next_ && node->next_->prev_ != node)
                    return false;
                if (node->prev_ && node->prev_->next_ != node)
                    return false;
                return true;
            }

            // Internal node must have exactly count_ + 1 children
            if (static_cast<int32_t>(node->children_.size()) != node->count_ + 1)
                return false;

            for (int32_t i = 0; i <= node->count_; ++i)
            {
                const K* newLower = (i == 0) ? lower : &node->keys_[i - 1];
                const K* newUpper = (i == node->count_) ? upper : &node->keys_[i];
                if (!verifyNode(node->children_[i].get(), newLower, newUpper))
                    return false;
            }
            return true;
        }

    public:
        /// @brief Default constructor
        BPlusTree() : root_(nullptr), head_(nullptr), size_(0) {}

        /// @brief Move constructor
        BPlusTree(BPlusTree&& other) noexcept
            : root_(std::move(other.root_))
            , head_(std::exchange(other.head_, nullptr))
            , size_(std::exchange(other.size_, 0))
        {
        }

        /// @brief Move assignment operator
        BPlusTree& operator=(BPlusTree&& other) noexcept
        {
            if (this != &other)
            {
                root_ = std::move(other.root_);
                head_ = std::exchange(other.head_, nullptr);
                size_ = std::exchange(other.size_, 0);
            }
            return *this;
        }

        BPlusTree(const BPlusTree&) = delete;
        BPlusTree& operator=(const BPlusTree&) = delete;

        ~BPlusTree() = default;

        // ── Public API ──────────────────────────────────────────────────

        /// @brief Insert a key-value pair. If the key already exists, the value is updated.
        /// @param key The key to insert
        /// @param value The associated value
        void insert(const K& key, const V& value)
        {
            if (!root_)
            {
                root_ = std::make_unique<Node>(true);
                root_->keys_.push_back(key);
                root_->values_.push_back(value);
                root_->count_ = 1;
                head_ = root_.get();
                ++size_;
                return;
            }

            if (root_->isFull())
            {
                auto newRoot = std::make_unique<Node>(false);
                newRoot->children_.push_back(std::move(root_));
                root_ = std::move(newRoot);

                if (root_->children_[0]->leaf_)
                    splitLeaf(root_.get(), 0);
                else
                    splitInternal(root_.get(), 0);
            }
            insertNonFull(root_.get(), key, value);
        }

        /// @brief Remove a key from the tree
        /// @param key The key to remove
        /// @return true if the key was found and removed, false otherwise
        bool remove(const K& key)
        {
            if (!root_) return false;
            int32_t oldSize = size_;
            removeRecursive(root_.get(), key);

            // Shrink the tree if the root becomes empty
            if (root_->count_ == 0)
            {
                if (root_->leaf_)
                {
                    root_.reset();
                    head_ = nullptr;
                }
                else
                {
                    root_ = std::move(root_->children_[0]);
                }
            }

            return oldSize != size_;
        }

        /// @brief Find the value associated with a key
        /// @param key The key to search for
        /// @return The value if found, std::nullopt otherwise
        [[nodiscard]] std::optional<V> find(const K& key) const
        {
            Node* leaf = findLeaf(key);
            if (!leaf) return std::nullopt;
            int32_t idx = leaf->findKey(key, comp_);
            if (idx < leaf->count_ && !comp_(leaf->keys_[idx], key) && !comp_(key, leaf->keys_[idx]))
                return leaf->values_[idx];
            return std::nullopt;
        }

        /// @brief Check if the tree contains a key
        [[nodiscard]] bool contains(const K& key) const
        {
            return find(key).has_value();
        }

        /// @brief Get all key-value pairs with key in [begin, end)
        /// @param begin Lower bound (inclusive)
        /// @param end Upper bound (exclusive)
        [[nodiscard]] std::vector<std::pair<K, V>> rangeScan(const K& begin, const K& end) const
        {
            std::vector<std::pair<K, V>> result;
            if (!root_) return result;

            Node* leaf = findLeaf(begin);
            if (!leaf) return result;

            // Start from the first key >= begin
            int32_t idx = 0;
            while (idx < leaf->count_ && comp_(leaf->keys_[idx], begin))
                ++idx;

            // Traverse leaves
            Node* current = leaf;
            int32_t pos = idx;
            while (current)
            {
                while (pos < current->count_)
                {
                    if (!comp_(current->keys_[pos], end))
                    {
                        // key >= end, we're done
                        if (comp_(end, current->keys_[pos]) || comp_(current->keys_[pos], end))
                            return result;
                        // If key == end, exclude (end is exclusive)
                        return result;
                    }
                    // begin <= key < end, include it
                    result.emplace_back(current->keys_[pos], current->values_[pos]);
                    ++pos;
                }
                current = current->next_;
                pos = 0;
            }
            return result;
        }

        /// @brief Get the number of elements in the tree
        [[nodiscard]] int32_t size() const { return size_; }

        /// @brief Check if the tree is empty
        [[nodiscard]] bool empty() const { return size_ == 0; }

        /// @brief Clear all elements from the tree
        void clear()
        {
            root_.reset();
            head_ = nullptr;
            size_ = 0;
        }

        /// @brief Get all key-value pairs in sorted order (via leaf list traversal)
        [[nodiscard]] std::vector<std::pair<K, V>> inorder() const
        {
            std::vector<std::pair<K, V>> result;
            inorderLeaves(head_, result);
            return result;
        }

        /// @brief Get the height of the tree (number of levels)
        [[nodiscard]] int32_t height() const
        {
            if (!root_) return 0;
            Node* node = root_.get();
            int32_t h = 1;
            while (!node->leaf_)
            {
                node = node->children_[0].get();
                ++h;
            }
            return h;
        }

        /// @brief Verify all B+Tree structural invariants
        [[nodiscard]] bool verify() const
        {
            if (!root_) return true;
            return verifyNode(root_.get(), nullptr, nullptr);
        }
    };
}
