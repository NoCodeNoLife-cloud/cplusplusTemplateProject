/**
 * @file BFS.hpp
 * @brief Breadth-First Search algorithm
 * @details Provides BFS traversal and shortest path (in number of edges) computation.
 *          BFS explores the graph level by level, guaranteeing the shortest path
 *          in terms of edge count in unweighted graphs.
 *
 * @par Thread Safety
 * This class provides only static methods and holds no internal state.
 * It is safe to call concurrently on different input instances.
 */

#pragma once
#include <cstdint>
#include <vector>

#include "data_structure/graph/Graph.hpp"
#include "data_structure/graph/IGraphVisitor.hpp"
#include "data_structure/graph/algorithm/GraphTraits.hpp"

namespace cppforge::data_structure::graph::algorithm
{
    /// @brief Breadth-First Search algorithm implementation
    /// @details All methods are static. Use BFS::traverse() for traversal or
    ///          BFS::shortestPath() for finding the shortest unweighted path.
    class BFS
    {
    public:
        BFS() = delete;

        /// @brief Perform BFS traversal starting from a given node
        /// @param graph The graph to traverse
        /// @param start The starting node index
        /// @param visitor Optional visitor for observing traversal events
        /// @return TraversalResult containing visit order, parent pointers, and distances
        static TraversalResult traverse(const Graph& graph, int32_t start,
                                        IGraphVisitor* visitor = nullptr);

        /// @brief Find the shortest path (minimum edges) between two nodes using BFS
        /// @param graph The graph to search
        /// @param start Source node
        /// @param target Target node
        /// @return Vector of nodes forming the shortest path (inclusive), empty if unreachable
        static std::vector<int32_t> shortestPath(const Graph& graph,
                                                  int32_t start, int32_t target);
    };
}
