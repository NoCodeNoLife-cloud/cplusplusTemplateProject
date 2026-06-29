/**
 * @file KruskalMST.hpp
 * @brief Kruskal's Minimum Spanning Tree algorithm
 * @details Computes a Minimum Spanning Tree (MST) for an undirected weighted graph.
 *          Sorts all edges by weight and greedily adds edges that do not form a cycle,
 *          using a Union-Find (Disjoint Set Union) data structure for cycle detection.
 *          Time complexity: O(ElogE). The graph should be undirected.
 */

#pragma once
#include <cstdint>

#include <cppforge/data_structure/graph/Graph.hpp>
#include <cppforge/data_structure/graph/algorithm/GraphTraits.hpp>

namespace cppforge::data_structure::graph::algorithm
{
    /// @brief Kruskal's Minimum Spanning Tree algorithm
    /// @details Processes edges in ascending weight order, adding each edge if it
    ///          connects two different components (Union-Find based cycle detection).
    class KruskalMST
    {
    public:
        KruskalMST() = delete;

        /// @brief Compute the Minimum Spanning Tree using Kruskal's algorithm
        /// @param graph The undirected weighted graph
        /// @return MSTResult containing the MST edges and total weight
        static MSTResult compute(const Graph& graph);
    };
}
