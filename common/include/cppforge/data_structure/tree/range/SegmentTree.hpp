/**
 * @file SegmentTree.hpp
 * @brief Segment tree for O(log n) range queries and point updates
 * @details A segment tree stores an array of elements and supports range queries
 *          (sum, min, max, etc.) and point updates in O(log n) time.  Each leaf
 *          represents a single element; each internal node represents the
 *          combined value of its segment.  Uses a recursive build/query/update
 *          approach with configurable combining operation.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Complexity
 * - build:                      O(n)
 * - rangeQuery / pointUpdate:   O(log n)
 * - Memory:                     O(4n)
 */

#pragma once
#include <concepts>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace cppforge::data_structure::tree::range
{
    /// @brief A segment tree data structure for range queries and point updates.
    /// @tparam T The type of elements stored in the segment tree.
    /// @tparam MergeOp The merge operation type used to combine elements. Defaults to std::plus<T>.
    template <typename T, typename MergeOp = std::plus<T>>
    class SegmentTree
    {
    public:
        /// @brief Default constructor creates an empty segment tree.
        SegmentTree();

        /// @brief Constructs a segment tree with the given number of elements.
        /// @param n The number of elements, all initialized to T{}.
        /// @throws std::bad_alloc If memory allocation fails.
        explicit SegmentTree(size_t n);

        /// @brief Constructs a segment tree with the given number of elements.
        /// @param n The number of elements.
        /// @param init_value The initial value for all elements.
        /// @throws std::bad_alloc If memory allocation fails.
        SegmentTree(size_t n, const T& init_value);

        /// @brief Constructs a segment tree from an iterator range.
        /// @tparam InputIt The iterator type.
        /// @param first The beginning of the range.
        /// @param last The end of the range.
        /// @throws std::bad_alloc If memory allocation fails.
        template <std::input_iterator InputIt>
        SegmentTree(InputIt first, InputIt last);

        /// @brief Constructs a segment tree from an initializer list.
        /// @param init The initializer list.
        /// @throws std::bad_alloc If memory allocation fails.
        SegmentTree(std::initializer_list<T> init);

        /// @brief Updates the value at the given position.
        /// @param pos The position to update (0-indexed).
        /// @param value The new value.
        /// @throws std::out_of_range If pos is out of bounds.
        void update(size_t pos, const T& value);

        /// @brief Queries the merged result over the range [l, r).
        /// @param l The left bound (inclusive).
        /// @param r The right bound (exclusive).
        /// @return The merged result over the range.
        /// @throws std::out_of_range If the range is invalid or the tree is empty.
        [[nodiscard]] T query(size_t l, size_t r) const;

        /// @brief Accesses the element at the given position.
        /// @param pos The position (0-indexed).
        /// @return A const reference to the element at the position.
        /// @throws std::out_of_range If pos is out of bounds.
        [[nodiscard]] const T& at(size_t pos) const;

        /// @brief Checks whether the segment tree is empty.
        /// @return True if the segment tree has no elements, false otherwise.
        [[nodiscard]] bool empty() const noexcept;

        /// @brief Returns the number of elements in the segment tree.
        /// @return The number of elements.
        [[nodiscard]] size_t size() const noexcept;

        /// @brief Swaps the contents of this segment tree with another.
        /// @param other The segment tree to swap with.
        void swap(SegmentTree& other) noexcept;

    private:
        /// @brief Recursively builds the segment tree from the underlying data.
        void buildTree(size_t node, size_t l, size_t r);

        /// @brief Recursively updates the value at the given position.
        void updateTree(size_t node, size_t l, size_t r, size_t pos, const T& value);

        /// @brief Recursively queries the segment tree over the given range.
        [[nodiscard]] std::optional<T> queryTree(size_t node, size_t l, size_t r, size_t ql, size_t qr) const;

        std::vector<T> data_;
        size_t n_{};
        MergeOp merge_{};
        std::vector<T> tree_;
    };

    template <typename T, typename MergeOp>
    SegmentTree<T, MergeOp>::SegmentTree() = default;

    template <typename T, typename MergeOp>
    SegmentTree<T, MergeOp>::SegmentTree(size_t n)
        : data_(n), n_(n), tree_(4 * n)
    {
        if (n_ > 0)
        {
            buildTree(1, 0, n_);
        }
    }

    template <typename T, typename MergeOp>
    SegmentTree<T, MergeOp>::SegmentTree(size_t n, const T& init_value)
        : data_(n, init_value), n_(n), tree_(4 * n)
    {
        if (n_ > 0)
        {
            buildTree(1, 0, n_);
        }
    }

    template <typename T, typename MergeOp>
    template <std::input_iterator InputIt>
    SegmentTree<T, MergeOp>::SegmentTree(InputIt first, InputIt last)
        : data_(first, last), n_(data_.size()), tree_(4 * data_.size())
    {
        if (n_ > 0)
        {
            buildTree(1, 0, n_);
        }
    }

    template <typename T, typename MergeOp>
    SegmentTree<T, MergeOp>::SegmentTree(std::initializer_list<T> init)
        : data_(init), n_(data_.size()), tree_(4 * data_.size())
    {
        if (n_ > 0)
        {
            buildTree(1, 0, n_);
        }
    }

    template <typename T, typename MergeOp>
    void SegmentTree<T, MergeOp>::update(size_t pos, const T& value)
    {
        if (pos >= n_)
        {
            throw std::out_of_range("Position out of bounds");
        }
        updateTree(1, 0, n_, pos, value);
    }

    template <typename T, typename MergeOp>
    T SegmentTree<T, MergeOp>::query(size_t l, size_t r) const
    {
        if (l >= r || r > n_)
        {
            throw std::out_of_range("Invalid query range");
        }
        auto result = queryTree(1, 0, n_, l, r);
        if (!result.has_value())
        {
            throw std::out_of_range("Query returned no results");
        }
        return *result;
    }

    template <typename T, typename MergeOp>
    const T& SegmentTree<T, MergeOp>::at(size_t pos) const
    {
        if (pos >= n_)
        {
            throw std::out_of_range("Position out of bounds");
        }
        return data_[pos];
    }

    template <typename T, typename MergeOp>
    bool SegmentTree<T, MergeOp>::empty() const noexcept
    {
        return n_ == 0;
    }

    template <typename T, typename MergeOp>
    size_t SegmentTree<T, MergeOp>::size() const noexcept
    {
        return n_;
    }

    template <typename T, typename MergeOp>
    void SegmentTree<T, MergeOp>::swap(SegmentTree& other) noexcept
    {
        using std::swap;
        swap(n_, other.n_);
        swap(merge_, other.merge_);
        swap(tree_, other.tree_);
        swap(data_, other.data_);
    }

    template <typename T, typename MergeOp>
    void SegmentTree<T, MergeOp>::buildTree(size_t node, size_t l, size_t r)
    {
        if (l + 1 == r)
        {
            tree_[node] = data_[l];
            return;
        }
        size_t mid = l + (r - l) / 2;
        buildTree(node * 2, l, mid);
        buildTree(node * 2 + 1, mid, r);
        tree_[node] = merge_(tree_[node * 2], tree_[node * 2 + 1]);
    }

    template <typename T, typename MergeOp>
    void SegmentTree<T, MergeOp>::updateTree(size_t node, size_t l, size_t r, size_t pos, const T& value)
    {
        if (l + 1 == r)
        {
            tree_[node] = value;
            data_[pos] = value;
            return;
        }
        size_t mid = l + (r - l) / 2;
        if (pos < mid)
        {
            updateTree(node * 2, l, mid, pos, value);
        }
        else
        {
            updateTree(node * 2 + 1, mid, r, pos, value);
        }
        tree_[node] = merge_(tree_[node * 2], tree_[node * 2 + 1]);
    }

    template <typename T, typename MergeOp>
    std::optional<T> SegmentTree<T, MergeOp>::queryTree(size_t node, size_t l, size_t r, size_t ql, size_t qr) const
    {
        if (ql >= r || qr <= l)
        {
            return std::nullopt;
        }
        if (ql <= l && r <= qr)
        {
            return tree_[node];
        }
        size_t mid = l + (r - l) / 2;
        auto left = queryTree(node * 2, l, mid, ql, qr);
        auto right = queryTree(node * 2 + 1, mid, r, ql, qr);
        if (!left.has_value())
        {
            return right;
        }
        if (!right.has_value())
        {
            return left;
        }
        return merge_(*left, *right);
    }
}
