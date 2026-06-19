/**
 * @file Dijkstra.hpp
 * @brief Dijkstra's shortest path algorithm
 * @details Computes single-source shortest paths in graphs with non-negative edge weights.
 *          Uses a priority queue (min-heap) for efficient extraction of the minimum-distance node.
 *          Does NOT handle negative edge weights — use BellmanFord for graphs with negative weights.
 *
 * @par Thread Safety
 * This class provides only static methods and holds no internal state.
 * It is safe to call concurrently on different input instances.
 */

#pragma once
#include <cstdint>

#include "data_structure/graph/Graph.hpp"
#include "data_structure/graph/algorithm/GraphTraits.hpp"

namespace common::data_structure::graph::algorithm
{
    /// @brief Dijkstra's shortest path algorithm implementation
    /// @details Uses std::priority_queue as a min-heap. Time complexity: O((V+E)logV).
    ///          All edge weights must be non-negative.
    class Dijkstra
    {
    public:
        Dijkstra() = delete;

        /// @brief Compute shortest paths from source to all reachable nodes
        /// @param graph The graph (must have non-negative edge weights)
        /// @param start Source node
        /// @return ShortestPathResult with distances and predecessor information
        static ShortestPathResult shortestPath(const Graph& graph, int32_t start);

        /// @brief Compute shortest path from source to a specific target node
        /// @param graph The graph (must have non-negative edge weights)
        /// @param start Source node
        /// @param target Target node
        /// @return ShortestPathResult with distances, useful for reconstructPath()
        static ShortestPathResult shortestPathToTarget(const Graph& graph,
                                                       int32_t start, int32_t target);
    };
}
