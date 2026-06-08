/**
 * @file UnionSet.hpp
 * @brief UnionSet class declaration
 * @details This header defines the UnionSet class that provides functionality for common data structures.
 */

#pragma once
#include <cstdint>
#include <unordered_map>

namespace common::data_structure
{
    /// @brief A Union-Find (Disjoint Set Union) data structure implementation.
    /// This class provides efficient operations for disjoint sets, including:
    /// - Finding the root of a set with path compression
    /// - Uniting two sets with union by rank
    /// - Checking if two elements are in the same set
    ///
    /// The implementation uses two hash maps:
    /// - parent: Maps each element to its parent in the set
    /// - rank: Maps each element to its rank (used for union by rank optimization)
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