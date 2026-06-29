/**
 * @file UnionSet.hpp
 * @brief Union-Find (Disjoint Set Union) with path compression + union by rank
 * @details A near-O(α(n)) Union-Find data structure (Disjoint Set Union)
 *          implementing path compression and union by rank.  Uses two
 *          unordered_maps for parent and rank storage, allowing arbitrary
 *          element types (not just integer indices).  Elements are registered
 *          lazily on first access.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access as the internal maps are mutated on read (path
 * compression).
 *
 * @par Complexity
 * - find:       Amortised O(α(n))  (inverse Ackermann)
 * - unionSets:  Amortised O(α(n))
 * - connected:  Amortised O(α(n))
 *
 * @par Usage Example
 * @code
 * UnionSet<int> dsu;
 * dsu.unionSets(1, 2);
 * dsu.unionSets(2, 3);
 * assert(dsu.connected(1, 3));  // transitive
 * assert(!dsu.connected(1, 4));
 * @endcode
 */

#pragma once
#include <cstdint>
#include <unordered_map>

namespace cppforge::data_structure
{
    /// @brief A Union-Find (Disjoint Set Union) with path compression + union by rank.
    ///
    /// @tparam T Element type (any hashable type usable as unordered_map key).
    ///
    /// Uses two unordered_maps for storage:
    /// - parent: element �?its representative parent
    /// - rank:   element �?rank (tree height upper bound) for union by rank
    ///
    /// @par Thread Safety
    /// This class is **not** thread-safe.  The internal maps are mutated on
    /// every find() call due to path compression, so external synchronisation
    /// is required even for concurrent reads.
    ///
    /// @par Memory
    /// Elements are registered lazily on first access via ensureRegistered().
    /// Each element consumes two entries (parent + rank) in the hash maps.
    template <typename T>
    class UnionSet
    {
    public:
        /// @brief Default constructor creates an empty UnionSet
        UnionSet();

        /// @brief Finds the root of the set containing element x with path compression.
        /// @param x The element to find the root for.
        /// @return The root of the set containing element x.
        [[nodiscard]] T find(const T& x) const;

        /// @brief Unites the sets that contain elements x and y.
        /// @param x First element
        /// @param y Second element
        /// @return True if the sets were successfully united, false if they were already in the same set.
        [[nodiscard]] bool unionSets(const T& x, const T& y);

        /// @brief Checks if elements x and y are in the same set.
        /// @param x First element
        /// @param y Second element
        /// @return True if x and y are connected (in the same set), false otherwise.
        [[nodiscard]] bool connected(const T& x, const T& y) const;

    private:
        /// @brief Ensures that the element x is registered in the UnionSet.
        /// @param x The element to register.
        void ensureRegistered(const T& x) const;

        mutable std::unordered_map<T, T> parent_{};
        mutable std::unordered_map<T, uint32_t> rank_{};
    };

    template <typename T>
    UnionSet<T>::UnionSet() = default;

    template <typename T>
    T UnionSet<T>::find(const T& x) const
    {
        ensureRegistered(x);
        T root = x;
        while (parent_[root] != root)
        {
            root = parent_[root];
        }
        T cur = x;
        while (parent_[cur] != root)
        {
            T next = parent_[cur];
            parent_[cur] = root;
            cur = next;
        }
        return root;
    }

    template <typename T>
    bool UnionSet<T>::unionSets(const T& x, const T& y)
    {
        T rootX = find(x);
        T rootY = find(y);

        if (rootX == rootY)
        {
            return false;
        }

        if (rank_[rootX] < rank_[rootY])
        {
            parent_[rootX] = rootY;
        }
        else if (rank_[rootX] > rank_[rootY])
        {
            parent_[rootY] = rootX;
        }
        else
        {
            parent_[rootY] = rootX;
            ++rank_[rootX];
        }
        return true;
    }

    template <typename T>
    bool UnionSet<T>::connected(const T& x, const T& y) const
    {
        return find(x) == find(y);
    }

    template <typename T>
    void UnionSet<T>::ensureRegistered(const T& x) const
    {
        parent_.try_emplace(x, x);
        rank_.try_emplace(x, 0);
    }
}