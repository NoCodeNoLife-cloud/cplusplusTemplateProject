/**
 * @file BipartiteCheck.hpp
 * @brief Bipartite graph checking algorithm
 * @details Uses BFS-based 2-coloring to determine if a graph is bipartite.
 *          A graph is bipartite if its nodes can be partitioned into two sets
 *          such that every edge connects nodes from different sets.
 *          Equivalent to checking if the graph contains no odd-length cycles.
 */

#pragma once
#include <cstdint>

#include "data_structure/graph/Graph.hpp"
#include "data_structure/graph/algorithm/GraphTraits.hpp"

namespace cppforge::data_structure::graph::algorithm
{
    /// @brief Bipartite graph checking algorithm
    /// @details Attempts to 2-color the graph using BFS. If a conflict is found
    ///          (adjacent nodes with the same color), the graph is not bipartite.
    class BipartiteCheck
    {
    public:
        BipartiteCheck() = delete;

        /// @brief Check if a graph is bipartite using BFS-based 2-coloring
        /// @param graph The graph to check
        /// @return BipartiteResult with is_bipartite flag and color assignment
        static BipartiteResult check(const Graph& graph);
    };
}
