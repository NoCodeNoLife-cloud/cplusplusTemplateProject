/**
 * @file SparseTable.hpp
 * @brief Sparse Table — O(1) range query on static arrays (idempotent ops)
 * @details A sparse table is a static data structure that precomputes the
 *          results of an idempotent merge operation for all intervals whose
 *          length is a power of two.  Range queries are answered in O(1) by
 *          merging two overlapping intervals of maximum power-of-two length.
 *          The construction runs in O(n log n) time and uses O(n log n)
 *          memory.  This implementation is restricted to idempotent merge
 *          operations (e.g. min, max, gcd, lcm) so that overlapping intervals
 *          can be safely combined.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Complexity
 * - build:                      O(n log n)
 * - query (idempotent merge):   O(1)
 * - Memory:                     O(n log n)
 *
 * @par Reference
 * Bender & Farach-Colton, "The LCA Problem Revisited" (2000), LATIN.
 * https://doi.org/10.1007/10719839_9
 */

#pragma once

#include <bit>           // std::bit_width
#include <concepts>      // std::semiregular, std::input_iterator, std::regular_invocable
#include <cstddef>       // size_t
#include <initializer_list>
#include <iterator>      // std::input_iterator
#include <stdexcept>     // std::out_of_range
#include <type_traits>   // std::invoke_result_t, std::convertible_to
#include <utility>       // std::move, std::swap
#include <vector>

namespace common::data_structure::tree::range
{
    /// @brief Detail helpers and custom merge operations.
    namespace detail
    {
        /// @brief Default merge operation: returns the smaller of two values.
        ///
        /// @par Thread Safety
        /// This functor is stateless and safe for concurrent invocation.
        struct MinOp
        {
            /// @brief Returns the smaller of two values.
            /// @tparam T The value type (deduced).
            /// @param a The first operand.
            /// @param b The second operand.
            /// @return A const reference to the smaller of @p a and @p b.
            template <typename T>
            [[nodiscard]] constexpr auto operator()(const T& a, const T& b) const
                noexcept(noexcept(a < b)) -> const T&
            {
                return a < b ? a : b;
            }
        };
    } // namespace detail

    /// @brief Concept for an idempotent merge operation.
    ///
    /// An operation is idempotent if applying it to the same value twice
    /// yields that value:  Op(x, x) == x.  Examples include min, max, gcd,
    /// and lcm.  This property allows the sparse table to answer range
    /// queries by combining two potentially overlapping intervals.
    ///
    /// @tparam Op The operation to check.
    /// @tparam T The element type.
    template <typename Op, typename T>
    concept IdempotentMerge = std::regular_invocable<Op, const T&, const T&>
                           && std::convertible_to<
                                  std::invoke_result_t<Op, const T&, const T&>,
                                  T>;

    /// @brief A sparse table for O(1) range queries with idempotent merge.
    ///
    /// Given an array of elements and an idempotent merge operation (e.g.
    /// min, max, gcd), the sparse table precomputes answers for all
    /// power-of-two-length intervals.  Any arbitrary range query [l, r) can
    /// be answered in O(1) by merging two overlapping power-of-two intervals.
    ///
    /// @par Thread Safety
    /// This class is **not** thread-safe.  External synchronisation is
    /// required for concurrent read/write or write/write access.  Concurrent
    /// read-only access is safe only if no mutating operation has been
    /// performed.
    ///
    /// @par Usage Example
    /// @code
    /// // Range minimum query
    /// SparseTable<int> st({9, 3, 7, 1, 8, 2});
    /// auto min_val = st.query(1, 5);  // returns 1 (min of {3,7,1,8})
    ///
    /// // Range maximum query
    /// auto max_st = SparseTable<int, std::ranges::greater>(
    ///     std::vector{4, 2, 9, 1});
    /// auto max_val = max_st.query(0, 3);  // returns 9
    /// @endcode
    ///
    /// @tparam T The element type.  Must satisfy std::semiregular.
    /// @tparam MergeOp The idempotent merge operation.  Must satisfy
    ///         IdempotentMerge<MergeOp, T>.  Defaults to detail::MinOp.
    template <std::semiregular T, typename MergeOp = detail::MinOp>
        requires IdempotentMerge<MergeOp, T>
    class SparseTable
    {
    public:
        /// @brief Default constructor creates an empty sparse table.
        SparseTable() = default;

        /// @brief Constructs a sparse table with the given number of elements.
        /// @param n The number of elements, all initialized to T{}.
        /// @throws std::bad_alloc If memory allocation fails.
        explicit SparseTable(size_t n)
            : SparseTable(n, T{})
        {
        }

        /// @brief Constructs a sparse table with the given number of elements.
        /// @param n The number of elements.
        /// @param init_value The initial value for all elements.
        /// @throws std::bad_alloc If memory allocation fails.
        SparseTable(size_t n, const T& init_value)
        {
            if (n > 0)
            {
                buildFrom(std::vector<T>(n, init_value));
            }
        }

        /// @brief Constructs a sparse table from an iterator range.
        /// @tparam InputIt The iterator type.
        /// @param first The beginning of the range.
        /// @param last The end of the range.
        /// @throws std::bad_alloc If memory allocation fails.
        template <std::input_iterator InputIt>
        SparseTable(InputIt first, InputIt last)
        {
            buildFrom(std::vector<T>(first, last));
        }

        /// @brief Constructs a sparse table from an initializer list.
        /// @param init The initializer list.
        /// @throws std::bad_alloc If memory allocation fails.
        SparseTable(std::initializer_list<T> init)
        {
            buildFrom(std::vector<T>(init));
        }

        /// @brief Answers a range query over [l, r) in O(1).
        ///
        /// Uses the idempotent property to merge two overlapping
        /// power-of-two intervals that together cover the full range.
        ///
        /// @param l The left bound (inclusive).
        /// @param r The right bound (exclusive).
        /// @return The merged value over the range [l, r).
        /// @throws std::out_of_range If the sparse table is empty or the range
        ///         is invalid.
        [[nodiscard]] auto query(size_t l, size_t r) const -> T
        {
            if (n_ == 0)
            {
                throw std::out_of_range("Cannot query an empty SparseTable");
            }
            if (l >= r || r > n_)
            {
                throw std::out_of_range("Invalid query range [l, r)");
            }

            auto len = r - l;
            if (len == 1)
            {
                return st_[0][l];   // single-element fast path
            }

            auto k    = static_cast<size_t>(std::bit_width(len)) - 1;
            auto half = size_t{1} << k;
            return merge_(st_[k][l], st_[k][r - half]);
        }

        /// @brief Accesses the element at the given position.
        /// @param pos The position (0-indexed).
        /// @return A const reference to the element at the position.
        /// @throws std::out_of_range If pos is out of bounds.
        [[nodiscard]] auto at(size_t pos) const -> const T&
        {
            if (pos >= n_)
            {
                throw std::out_of_range("Position out of bounds");
            }
            return st_[0][pos];
        }

        /// @brief Checks whether the sparse table is empty.
        /// @return True if the sparse table has no elements, false otherwise.
        [[nodiscard]] auto empty() const noexcept -> bool
        {
            return n_ == 0;
        }

        /// @brief Returns the number of elements in the sparse table.
        /// @return The number of elements.
        [[nodiscard]] auto size() const noexcept -> size_t
        {
            return n_;
        }

        /// @brief Swaps the contents of this sparse table with another.
        /// @param other The sparse table to swap with.
        void swap(SparseTable& other) noexcept
        {
            using std::swap;
            swap(n_, other.n_);
            swap(merge_, other.merge_);
            swap(st_, other.st_);
        }

    private:
        /// @brief Constructs the sparse table from a data vector.
        ///
        /// Builds the table bottom-up: level 0 is the original data, and
        /// each subsequent level k merges pairs from level k-1 with stride
        /// 2^{k-1}.
        ///
        /// @param data The source data vector (moved into level 0).
        void buildFrom(std::vector<T>&& data)
        {
            n_ = data.size();
            if (n_ == 0)
            {
                return;
            }

            st_.reserve(static_cast<size_t>(std::bit_width(n_)));
            st_.push_back(std::move(data));   // st_[0] = original data

            auto k_max = static_cast<size_t>(std::bit_width(n_));
            for (size_t k = 1; k < k_max; ++k)
            {
                auto half = size_t{1} << (k - 1);
                auto len  = n_ - (size_t{1} << k) + 1;

                std::vector<T> row(len);
                const auto& prev = st_[k - 1];
                for (size_t i = 0; i < len; ++i)
                {
                    row[i] = merge_(prev[i], prev[i + half]);
                }
                st_.push_back(std::move(row));
            }
        }

        size_t n_{};
        [[no_unique_address]] MergeOp merge_{};
        std::vector<std::vector<T>> st_;
    };
} // namespace common::data_structure::tree::range
