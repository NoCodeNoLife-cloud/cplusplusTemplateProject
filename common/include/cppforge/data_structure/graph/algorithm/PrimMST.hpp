/**
 * @file PrimMST.hpp
 * @brief Prim's Minimum Spanning Tree algorithm
 * @details Computes a Minimum Spanning Tree (MST) for an undirected weighted graph.
 *          Uses a priority queue to greedily add the minimum-weight edge connecting
 *          the current tree to a node outside it. Time complexity: O(ElogV).
 *          The graph should be undirected (edges added via addUndirectedEdge).
 */

#pragma once
#include <cstdint>

#include <cppforge/data_structure/graph/Graph.hpp>
#include <cppforge/data_structure/graph/algorithm/GraphTraits.hpp>

namespace cppforge::data_structure::graph::algorithm
{
    /// @brief Prim's Minimum Spanning Tree algorithm
    /// @details Grows the MST one node at a time by adding the cheapest edge
    ///          from the current tree to a new node. For undirected graphs.
    class PrimMST
    {
    public:
        PrimMST() = delete;

        /// @brief Compute the Minimum Spanning Tree using Prim's algorithm
        /// @param graph The undirected weighted graph
        /// @return MSTResult containing the MST edges and total weight
        static MSTResult compute(const Graph& graph);
    };
}
