/**
 * @file DFS.hpp
 * @brief Depth-First Search algorithm
 * @details Provides DFS traversal (single-source and full graph), path finding,
 *          and visitor callback support. DFS explores as deep as possible along
 *          each branch before backtracking.
 */

#pragma once
#include <cstdint>
#include <vector>

#include "data_structure/graph/Graph.hpp"
#include "data_structure/graph/IGraphVisitor.hpp"
#include "data_structure/graph/algorithm/GraphTraits.hpp"

namespace common::data_structure::graph::algorithm
{
    /// @brief Depth-First Search algorithm implementation
    /// @details All methods are static. Supports single-source traversal,
    ///          full-graph traversal (covering all components), and path finding.
    class DFS
    {
    public:
        DFS() = delete;

        /// @brief Perform DFS traversal starting from a given node
        /// @param graph The graph to traverse
        /// @param start The starting node index
        /// @param visitor Optional visitor for observing traversal events
        /// @return TraversalResult containing visit order, parent pointers, and distances
        static TraversalResult traverse(const Graph& graph, int32_t start,
                                        IGraphVisitor* visitor = nullptr);

        /// @brief Perform DFS traversal covering all nodes in the graph
        /// @details Iterates over all nodes and starts a new DFS from each
        ///          unvisited node, ensuring all components are visited.
        /// @param graph The graph to traverse
        /// @param visitor Optional visitor for observing traversal events
        /// @return TraversalResult covering all nodes in the graph
        static TraversalResult traverseAll(const Graph& graph,
                                           IGraphVisitor* visitor = nullptr);

        /// @brief Find a path between two nodes using DFS
        /// @param graph The graph to search
        /// @param start Source node
        /// @param target Target node
        /// @return A path from start to target if one exists, empty otherwise
        static std::vector<int32_t> findPath(const Graph& graph,
                                              int32_t start, int32_t target);
    };
}
