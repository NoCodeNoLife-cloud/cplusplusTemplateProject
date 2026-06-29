/**
 * @file TarjanSCC.hpp
 * @brief Tarjan's Strongly Connected Components algorithm
 * @details Finds all strongly connected components in a directed graph using DFS.
 *          Each node is assigned an index and lowlink value. When a node's index
 *          equals its lowlink, it is the root of an SCC. Nodes in the same SCC
 *          are mutually reachable via directed paths.
 *          Time complexity: O(V+E).
 */

#pragma once
#include <cstdint>

#include "data_structure/graph/Graph.hpp"
#include "data_structure/graph/algorithm/GraphTraits.hpp"

namespace cppforge::data_structure::graph::algorithm
{
    /// @brief Tarjan's algorithm for finding strongly connected components
    /// @details A single DFS pass assigns indices and tracks reachable ancestors
    ///          via lowlink values. When lowlink == index, an SCC root is found.
    class TarjanSCC
    {
    public:
        TarjanSCC() = delete;

        /// @brief Find all strongly connected components in a directed graph
        /// @param graph The directed graph
        /// @return SCCResult containing the list of SCCs and component ID mapping
        static SCCResult compute(const Graph& graph);
    };
}
