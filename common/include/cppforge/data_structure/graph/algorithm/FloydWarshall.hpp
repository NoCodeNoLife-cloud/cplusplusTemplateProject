/**
 * @file FloydWarshall.hpp
 * @brief Floyd-Warshall all-pairs shortest path algorithm
 * @details Computes shortest paths between every pair of nodes using dynamic programming.
 *          Iteratively considers each node k as an intermediate point and checks whether
 *          the path i→k→j is shorter than the current i→j path.
 *          Time complexity: O(V³). Also detects negative-weight cycles.
 *
 * @par Thread Safety
 * This class provides only static methods and holds no internal state.
 * It is safe to call concurrently on different input instances.
 */

#pragma once
#include <cstdint>

#include <cppforge/data_structure/graph/Graph.hpp>
#include <cppforge/data_structure/graph/algorithm/GraphTraits.hpp>

namespace cppforge::data_structure::graph::algorithm
{
    /// @brief Floyd-Warshall all-pairs shortest path algorithm
    /// @details Computes the full distance matrix and a next-node matrix for
    ///          path reconstruction between any pair of nodes.
    class FloydWarshall
    {
    public:
        FloydWarshall() = delete;

        /// @brief Compute shortest paths between all pairs of nodes
        /// @param graph The graph (may contain negative edge weights)
        /// @return AllPairsShortestPathResult with full distance matrix and path reconstruction
        static AllPairsShortestPathResult compute(const Graph& graph);
    };
}
