/**
 * @file CycleDetection.hpp
 * @brief Cycle detection algorithms for directed and undirected graphs
 * @details Uses DFS-based coloring approach: nodes are marked as unvisited (0),
 *          in-progress (1), or finished (2). A cycle is detected when a back edge
 *          to an in-progress node is found.
 */

#pragma once
#include <cstdint>

#include <cppforge/data_structure/graph/Graph.hpp>
#include <cppforge/data_structure/graph/algorithm/GraphTraits.hpp>

namespace cppforge::data_structure::graph::algorithm
{
    /// @brief Cycle detection algorithms
    /// @details Supports both directed and undirected graph cycle detection.
    ///          For undirected graphs, ignores the immediate parent edge to avoid
    ///          false positives.
    class CycleDetection
    {
    public:
        CycleDetection() = delete;

        /// @brief Detect cycles in a directed graph
        /// @param graph The directed graph to check
        /// @return CycleResult with has_cycle flag and the detected cycle nodes
        static CycleResult detectDirected(const Graph& graph);

        /// @brief Detect cycles in an undirected graph
        /// @param graph The undirected graph to check
        /// @return CycleResult with has_cycle flag and the detected cycle nodes
        static CycleResult detectUndirected(const Graph& graph);
    };
}
