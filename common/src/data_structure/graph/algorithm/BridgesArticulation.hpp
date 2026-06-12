/**
 * @file BridgesArticulation.hpp
 * @brief Bridges and articulation points detection (Tarjan's DFS-based algorithm)
 * @details Identifies critical edges (bridges) and critical nodes (articulation points)
 *          in an undirected graph. Uses DFS with tin (entry time) and low (lowest
 *          reachable tin) values. An edge (u,v) is a bridge if low[v] > tin[u].
 *          A node u is an articulation point if it has a child v with low[v] >= tin[u],
 *          or if it is the root of the DFS tree with at least two children.
 *          Time complexity: O(V+E).
 */

#pragma once
#include <cstdint>

#include "data_structure/graph/Graph.hpp"
#include "data_structure/graph/algorithm/GraphTraits.hpp"

namespace common::data_structure::graph::algorithm
{
    /// @brief Bridges and articulation points detection
    /// @details Uses a single DFS pass to compute tin and low values, identifying
    ///          bridges and articulation points in an undirected graph.
    class BridgesArticulation
    {
    public:
        BridgesArticulation() = delete;

        /// @brief Find all bridges and articulation points in an undirected graph
        /// @param graph The undirected graph
        /// @return BridgesArticulationResult containing bridges and articulation points
        static BridgesArticulationResult compute(const Graph& graph);
    };
}
