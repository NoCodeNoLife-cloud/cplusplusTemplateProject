/**
 * @file BellmanFord.hpp
 * @brief Bellman-Ford shortest path algorithm
 * @details Computes single-source shortest paths in graphs that may contain
 *          negative edge weights. Also detects negative-weight cycles.
 *          Uses edge relaxation repeated V-1 times, with an extra V-th pass
 *          to detect negative cycles. Time complexity: O(VE).
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
    /// @brief Bellman-Ford shortest path algorithm
    /// @details Supports negative edge weights and detects negative cycles.
    ///          Slower than Dijkstra but more general. Use Dijkstra for
    ///          graphs with only non-negative weights.
    class BellmanFord
    {
    public:
        BellmanFord() = delete;

        /// @brief Compute shortest paths from source, supporting negative edge weights
        /// @param graph The graph (may contain negative edge weights)
        /// @param start Source node
        /// @return ShortestPathResult with distances and predecessor information
        static ShortestPathResult shortestPath(const Graph& graph, int32_t start);

        /// @brief Check if the graph contains a negative-weight cycle
        /// @details Uses Bellman-Ford's detection: after V-1 relaxations,
        ///          if another relaxation is possible, a negative cycle exists.
        /// @param graph The graph to check
        /// @return True if a negative-weight cycle is detected
        static bool hasNegativeCycle(const Graph& graph);
    };
}
