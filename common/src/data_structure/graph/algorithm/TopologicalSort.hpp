/**
 * @file TopologicalSort.hpp
 * @brief Topological sorting algorithms for directed acyclic graphs (DAGs)
 * @details Provides two implementations: DFS-based (recursive post-order) and
 *          Kahn's algorithm (BFS-based with in-degree counting). Both detect
 *          cycles and return an empty order if the graph is not a DAG.
 */

#pragma once
#include <cstdint>

#include "data_structure/graph/Graph.hpp"
#include "data_structure/graph/algorithm/GraphTraits.hpp"

namespace cppforge::data_structure::graph::algorithm
{
    /// @brief Topological sort algorithms for directed acyclic graphs
    /// @details sort() uses DFS-based post-order traversal, sortKahn() uses
    ///          Kahn's algorithm with in-degree tracking. Both detect cycles.
    class TopologicalSort
    {
    public:
        TopologicalSort() = delete;

        /// @brief Compute topological order using DFS-based post-order traversal
        /// @param graph The directed graph to sort
        /// @return TopologicalSortResult with the order (empty if cycle detected)
        static TopologicalSortResult sort(const Graph& graph);

        /// @brief Compute topological order using Kahn's algorithm (BFS + in-degree)
        /// @details More intuitive: repeatedly removes nodes with zero in-degree.
        /// @param graph The directed graph to sort
        /// @return TopologicalSortResult with the order (empty if cycle detected)
        static TopologicalSortResult sortKahn(const Graph& graph);
    };
}
