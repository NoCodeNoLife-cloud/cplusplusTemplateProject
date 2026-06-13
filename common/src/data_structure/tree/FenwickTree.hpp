/**
 * @file FenwickTree.hpp
 * @brief Fenwick Tree (Binary Indexed Tree) — O(log n) prefix sum & point update
 * @details A Fenwick tree provides O(log n) point updates and prefix-sum queries
 *          using a compact array representation.  Each index i stores a partial
 *          sum covering the range (i - LSB(i), i].  Supports range sum queries
 *          via two prefix-sum calls.  Uses 1-based indexing internally.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Complexity
 * - add / prefixSum / rangeSum: O(log n)
 * - Memory:                     O(n)
 *
 * @par Reference
 * Fenwick, "A New Data Structure for Cumulative Frequency Tables"
 * (1994), Software: Practice and Experience 24(3).
 */

#pragma once
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <vector>

namespace common::data_structure::tree
{
    /// @brief A Fenwick Tree (Binary Indexed Tree) for point updates and prefix/range sum queries.
    /// @tparam T The type of elements stored. Must support default construction, copy,
    ///         assignment, and addition/subtraction.
    template <std::semiregular T>
    class FenwickTree
    {
    public:
        /// @brief Default constructor creates an empty Fenwick tree.
        FenwickTree() = default;

        /// @brief Constructs a Fenwick tree with the given number of elements.
        /// @param n The number of elements, all initialized to T{}.
        /// @throws std::bad_alloc If memory allocation fails.
        explicit FenwickTree(size_t n);

        /// @brief Constructs a Fenwick tree with the given number of elements.
        /// @param n The number of elements.
        /// @param init_value The initial value for all elements.
        /// @throws std::bad_alloc If memory allocation fails.
        FenwickTree(size_t n, const T& init_value);

        /// @brief Constructs a Fenwick tree from an iterator range.
        /// @tparam InputIt The iterator type.
        /// @param first The beginning of the range.
        /// @param last The end of the range.
        /// @throws std::bad_alloc If memory allocation fails.
        template <std::input_iterator InputIt>
        FenwickTree(InputIt first, InputIt last);

        /// @brief Constructs a Fenwick tree from an initializer list.
        /// @param init The initializer list.
        /// @throws std::bad_alloc If memory allocation fails.
        FenwickTree(std::initializer_list<T> init);

        /// @brief Adds a delta value to the element at the given position.
        /// @param pos The position to update (0-indexed).
        /// @param delta The value to add.
        /// @throws std::out_of_range If pos is out of bounds.
        void add(size_t pos, const T& delta);

        /// @brief Computes the prefix sum of elements in [0, pos).
        /// @param pos The exclusive upper bound.
        /// @return The sum of elements in the range [0, pos).
        /// @throws std::out_of_range If pos is out of bounds.
        [[nodiscard]] T sum(size_t pos) const;

        /// @brief Computes the sum of elements in the range [l, r).
        /// @param l The left bound (inclusive).
        /// @param r The right bound (exclusive).
        /// @return The sum of elements in the range [l, r).
        /// @throws std::out_of_range If the range is invalid.
        [[nodiscard]] T sum(size_t l, size_t r) const;

        /// @brief Accesses the element at the given position.
        /// @param pos The position (0-indexed).
        /// @return The element at the position.
        /// @throws std::out_of_range If pos is out of bounds.
        [[nodiscard]] T at(size_t pos) const;

        /// @brief Checks whether the Fenwick tree is empty.
        /// @return True if the tree has no elements, false otherwise.
        [[nodiscard]] bool empty() const noexcept;

        /// @brief Returns the number of elements in the Fenwick tree.
        /// @return The number of elements.
        [[nodiscard]] size_t size() const noexcept;

        /// @brief Swaps the contents of this Fenwick tree with another.
        /// @param other The Fenwick tree to swap with.
        void swap(FenwickTree& other) noexcept;

    private:
        size_t n_{};
        std::vector<T> tree_;

        /// @brief Returns the least significant bit of the given index.
        [[nodiscard]] static size_t lowbit(size_t x) noexcept;
    };

    template <std::semiregular T>
    FenwickTree<T>::FenwickTree(size_t n)
        : n_(n), tree_(n + 1)
    {
    }

    template <std::semiregular T>
    FenwickTree<T>::FenwickTree(size_t n, const T& init_value)
        : n_(n), tree_(n + 1)
    {
        for (size_t i = 0; i < n_; ++i)
        {
            add(i, init_value);
        }
    }

    template <std::semiregular T>
    template <std::input_iterator InputIt>
    FenwickTree<T>::FenwickTree(InputIt first, InputIt last)
    {
        std::vector<T> data(first, last);
        n_ = data.size();
        tree_.resize(n_ + 1);
        for (size_t i = 0; i < n_; ++i)
        {
            add(i, data[i]);
        }
    }

    template <std::semiregular T>
    FenwickTree<T>::FenwickTree(std::initializer_list<T> init)
    {
        n_ = init.size();
        tree_.resize(n_ + 1);
        size_t i = 0;
        for (const auto& val : init)
        {
            add(i++, val);
        }
    }

    template <std::semiregular T>
    void FenwickTree<T>::add(size_t pos, const T& delta)
    {
        if (pos >= n_)
        {
            throw std::out_of_range("Position out of bounds");
        }
        for (size_t i = pos + 1; i <= n_; i += lowbit(i))
        {
            tree_[i] += delta;
        }
    }

    template <std::semiregular T>
    T FenwickTree<T>::sum(size_t pos) const
    {
        if (pos > n_)
        {
            throw std::out_of_range("Position out of bounds");
        }
        T result{};
        for (size_t i = pos; i > 0; i -= lowbit(i))
        {
            result += tree_[i];
        }
        return result;
    }

    template <std::semiregular T>
    T FenwickTree<T>::sum(size_t l, size_t r) const
    {
        if (l >= r || r > n_)
        {
            throw std::out_of_range("Invalid query range");
        }
        return sum(r) - sum(l);
    }

    template <std::semiregular T>
    T FenwickTree<T>::at(size_t pos) const
    {
        if (pos >= n_)
        {
            throw std::out_of_range("Position out of bounds");
        }
        return sum(pos, pos + 1);
    }

    template <std::semiregular T>
    bool FenwickTree<T>::empty() const noexcept
    {
        return n_ == 0;
    }

    template <std::semiregular T>
    size_t FenwickTree<T>::size() const noexcept
    {
        return n_;
    }

    template <std::semiregular T>
    void FenwickTree<T>::swap(FenwickTree& other) noexcept
    {
        using std::swap;
        swap(n_, other.n_);
        swap(tree_, other.tree_);
    }

    template <std::semiregular T>
    size_t FenwickTree<T>::lowbit(size_t x) noexcept
    {
        return x & static_cast<size_t>(-static_cast<std::make_signed_t<size_t>>(x));
    }
}
