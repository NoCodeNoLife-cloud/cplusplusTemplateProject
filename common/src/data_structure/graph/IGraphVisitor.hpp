/**
 * @file IGraphVisitor.hpp
 * @brief Visitor interface for graph traversal algorithms
 * @details Defines the IGraphVisitor interface which allows callers to observe
 *          and intercept graph traversal events (node visits, edge traversals, etc.).
 *          Used by BFS and DFS algorithms to provide custom behavior during traversal.
 */

#pragma once
#include <cstdint>

namespace cppforge::data_structure::graph
{
    /// @brief Visitor interface for graph traversal callbacks
    /// @details Implement this interface to observe or intercept traversal events.
    ///          All methods have default empty implementations, so implementors only
    ///          need to override the events they care about.
    class IGraphVisitor
    {
    public:
        virtual ~IGraphVisitor() = default;

        /// @brief Called when a node is visited (BFS) or discovered (DFS)
        /// @param node The node being visited
        virtual void onNodeVisit(int32_t node) {}

        /// @brief Called when a node is first discovered in DFS
        /// @param node The node being discovered
        virtual void onNodeDiscover(int32_t node) {}

        /// @brief Called when a node's exploration is complete in DFS
        /// @param node The node that has finished exploration
        virtual void onNodeFinish(int32_t node) {}

        /// @brief Called when an edge is traversed forward to an unvisited node
        /// @param from Source node of the edge
        /// @param to Target node of the edge
        /// @param weight Weight of the edge
        virtual void onEdgeTraverse(int32_t from, int32_t to, int32_t weight) {}

        /// @brief Called when a DFS traversal encounters an already-visited node (back edge)
        /// @param from Source node of the edge
        /// @param to Target node of the edge
        /// @param weight Weight of the edge
        virtual void onEdgeBack(int32_t from, int32_t to, int32_t weight) {}
    };
}
