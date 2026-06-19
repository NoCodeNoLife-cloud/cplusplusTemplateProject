/**
 * @file Treap.hpp
 * @brief Randomized BST combining tree (key) and heap (priority)
 * @details A treap (tree + heap) is a randomized binary search tree where each node
 *          has a random priority. The tree maintains BST ordering by key and heap
 *          ordering by priority (max-heap), providing expected O(log n) operations.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 */

#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <random>
#include <utility>
#include <vector>

namespace common::data_structure::tree::balanced
{
    /// @brief A randomized binary search tree (Treap)
    /// @details Treaps combine BST and heap properties. Each node stores a key and a
    ///          random priority. The tree is a BST by key and a max-heap by priority.
    ///          This randomized balancing gives expected O(log n) time for insert,
    ///          remove, and find operations. Unlike deterministic balanced trees,
    ///          no balance factor or color tracking is needed.
    /// @tparam T The type of data stored in the tree. Must be comparable with <, >, and ==.
    template <typename T>
    class Treap
    {
    private:
        /// @brief Internal tree node containing data, priority, and child pointers
        struct Node
        {
            T data;
            int32_t priority;
            std::unique_ptr<Node> left;
            std::unique_ptr<Node> right;

            /// @brief Construct a node with a random priority
            explicit Node(T value)
                : data(std::move(value))
                , priority(randomPriority())
            {
            }
        };

    public:
        Treap() = default;

        /// @brief Move constructor
        Treap(Treap&& other) noexcept
            : root_(std::move(other.root_))
            , size_(std::exchange(other.size_, 0))
        {
        }

        /// @brief Move assignment operator
        Treap& operator=(Treap&& other) noexcept
        {
            if (this != &other)
            {
                root_ = std::move(other.root_);
                size_ = std::exchange(other.size_, 0);
            }
            return *this;
        }

        Treap(const Treap&) = delete;
        Treap& operator=(const Treap&) = delete;

        /// @brief Insert a value into the treap
        /// @param value The value to insert
        void insert(const T& value)
        {
            root_ = insertImpl(std::move(root_), value);
        }

        /// @brief Remove a value from the treap
        /// @param value The value to remove
        void remove(const T& value)
        {
            root_ = removeImpl(std::move(root_), value);
        }

        /// @brief Check if a value exists in the treap
        /// @param value The value to find
        /// @return true if the value is found, false otherwise
        [[nodiscard]] bool find(const T& value) const
        {
            return findImpl(root_.get(), value);
        }

        /// @brief Find a value and return it
        /// @param value The value to find
        /// @return The stored value if found, std::nullopt otherwise
        [[nodiscard]] std::optional<T> findValue(const T& value) const
        {
            const auto* node = findNode(root_.get(), value);
            if (node) return node->data;
            return std::nullopt;
        }

        /// @brief Check if the treap is empty
        [[nodiscard]] bool empty() const
        {
            return size_ == 0;
        }

        /// @brief Get the number of nodes in the treap
        [[nodiscard]] size_t size() const
        {
            return size_;
        }

        /// @brief Remove all nodes from the treap
        void clear()
        {
            root_.reset();
            size_ = 0;
        }

        /// @brief Return the inorder traversal as a vector
        /// @return Vector of elements in sorted order
        [[nodiscard]] std::vector<T> inorder() const
        {
            std::vector<T> result;
            inorderRecursive(root_.get(), result);
            return result;
        }

        /// @brief Verify the treap invariants (BST order + heap order)
        /// @details For testing purposes only. Checks that inorder is sorted and
        ///          each node's priority is >= its children's priorities.
        [[nodiscard]] bool verify() const
        {
            auto sorted = inorder();
            for (size_t i = 1; i < sorted.size(); ++i)
            {
                if (sorted[i - 1] > sorted[i]) return false;
            }
            return verifyHeap(root_.get());
        }

    private:
        std::unique_ptr<Node> root_{};
        size_t size_{0};

        /// @brief Generate a random priority for a new node
        /// @return Random int32_t value
        static int32_t randomPriority()
        {
            static std::mt19937 gen(std::random_device{}());
            static std::uniform_int_distribution<int32_t> dist;
            return dist(gen);
        }

        /// @brief Recursively verify max-heap ordering of priorities
        /// @param node Current node to check
        /// @return true if heap property holds for the entire subtree
        static bool verifyHeap(const Node* node)
        {
            if (!node) return true;
            if (node->left && node->left->priority > node->priority) return false;
            if (node->right && node->right->priority > node->priority) return false;
            return verifyHeap(node->left.get()) && verifyHeap(node->right.get());
        }

        /// @brief Find a value and return whether it exists
        static bool findImpl(const Node* node, const T& value)
        {
            return findNode(node, value) != nullptr;
        }

        /// @brief Find a node by value in a binary search tree
        /// @param node Current subtree root
        /// @param value The value to find
        /// @return Pointer to the node if found, nullptr otherwise
        static const Node* findNode(const Node* node, const T& value)
        {
            if (!node) return nullptr;
            if (value == node->data) return node;
            return value < node->data
                ? findNode(node->left.get(), value)
                : findNode(node->right.get(), value);
        }

        /// @brief Recursive inorder traversal collecting values
        void inorderRecursive(const Node* node, std::vector<T>& result) const
        {
            if (!node) return;
            inorderRecursive(node->left.get(), result);
            result.push_back(node->data);
            inorderRecursive(node->right.get(), result);
        }

        /// @brief Right rotation to restore heap order
        /// @param node The pivot node (becomes right child after rotation)
        /// @return New subtree root (the left child of the original node)
        std::unique_ptr<Node> rotateRight(std::unique_ptr<Node> node)
        {
            auto left = std::move(node->left);
            node->left = std::move(left->right);
            left->right = std::move(node);
            return left;
        }

        /// @brief Left rotation to restore heap order
        /// @param node The pivot node (becomes left child after rotation)
        /// @return New subtree root (the right child of the original node)
        std::unique_ptr<Node> rotateLeft(std::unique_ptr<Node> node)
        {
            auto right = std::move(node->right);
            node->right = std::move(right->left);
            right->left = std::move(node);
            return right;
        }

        /// @brief Recursive BST insertion with priority-based rotation
        /// @param node Current subtree root
        /// @param value Value to insert
        /// @return Updated subtree root after potential rotation
        std::unique_ptr<Node> insertImpl(std::unique_ptr<Node> node, const T& value)
        {
            if (!node)
            {
                ++size_;
                return std::make_unique<Node>(value);
            }

            if (value < node->data)
            {
                node->left = insertImpl(std::move(node->left), value);
                if (node->left->priority > node->priority)
                {
                    node = rotateRight(std::move(node));
                }
            }
            else if (value > node->data)
            {
                node->right = insertImpl(std::move(node->right), value);
                if (node->right->priority > node->priority)
                {
                    node = rotateLeft(std::move(node));
                }
            }

            return node;
        }

        /// @brief Recursive removal by rotating target down to a leaf
        /// @param node Current subtree root
        /// @param value Value to remove
        /// @return Updated subtree root
        std::unique_ptr<Node> removeImpl(std::unique_ptr<Node> node, const T& value)
        {
            if (!node) return nullptr;

            if (value < node->data)
            {
                node->left = removeImpl(std::move(node->left), value);
                return node;
            }

            if (value > node->data)
            {
                node->right = removeImpl(std::move(node->right), value);
                return node;
            }

            // Found the node to remove
            if (!node->left && !node->right)
            {
                --size_;
                return nullptr;
            }

            if (!node->left)
            {
                --size_;
                return std::move(node->right);
            }

            if (!node->right)
            {
                --size_;
                return std::move(node->left);
            }

            // Two children: rotate the higher-priority child up
            if (node->left->priority > node->right->priority)
            {
                node = rotateRight(std::move(node));
                node->right = removeImpl(std::move(node->right), value);
            }
            else
            {
                node = rotateLeft(std::move(node));
                node->left = removeImpl(std::move(node->left), value);
            }

            return node;
        }
    };
}
