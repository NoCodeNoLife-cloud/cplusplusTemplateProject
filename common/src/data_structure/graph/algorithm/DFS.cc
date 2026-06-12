/**
 * @file DFS.cc
 * @brief Depth-First Search algorithm implementation
 * @details Uses an iterative stack-based approach to explore the graph.
 *          Supports both single-source traversal and full-graph traversal
 *          (covering all connected components).
 */

#include "data_structure/graph/algorithm/DFS.hpp"

#include <vector>

namespace common::data_structure::graph::algorithm
{
    namespace
    {
        /// @brief Internal iterative DFS implementation
        /// @details Uses a vector as a stack. When a node is popped, if it has
        ///          not been added to the order yet, it is recorded. Neighbors
        ///          are pushed onto the stack for later exploration.
        void dfsIterative(const Graph& graph, int32_t start,
                          TraversalResult& result, IGraphVisitor* visitor)
        {
            const auto start_idx = static_cast<size_t>(start);
            result.parent[start_idx] = -1;
            result.distance[start_idx] = 0;

            std::vector<int32_t> st;
            st.push_back(start);

            while (!st.empty())
            {
                const int32_t u = st.back();
                st.pop_back();
                const auto u_idx = static_cast<size_t>(u);

                // Add to order if not already added (handles duplicates from stack)
                if (result.order.empty() || result.order.back() != u)
                {
                    result.order.push_back(u);
                    if (visitor) visitor->onNodeDiscover(u);
                }

                // Explore all neighbors
                for (const auto& edge : graph.getAdjList(u))
                {
                    const int32_t v = edge.to();
                    const auto v_idx = static_cast<size_t>(v);
                    if (result.distance[v_idx] == INT32_MAX)
                    {
                        // Unvisited neighbor: set distance, parent, and push to stack
                        result.distance[v_idx] = result.distance[u_idx] + 1;
                        result.parent[v_idx] = u;
                        st.push_back(v);
                        if (visitor) visitor->onEdgeTraverse(u, v, edge.weight());
                    }
                    else if (visitor)
                    {
                        // Already visited neighbor = back edge
                        visitor->onEdgeBack(u, v, edge.weight());
                    }
                }

                if (visitor) visitor->onNodeFinish(u);
            }
        }
    }

    TraversalResult DFS::traverse(const Graph& graph, const int32_t start,
                                   IGraphVisitor* visitor)
    {
        const int32_t n = graph.getNodeCount();
        TraversalResult result;
        result.order.reserve(static_cast<size_t>(n));
        result.parent.assign(static_cast<size_t>(n), -1);
        result.distance.assign(static_cast<size_t>(n), INT32_MAX);

        if (n == 0 || start < 0 || start >= n)
        {
            return result;
        }

        dfsIterative(graph, start, result, visitor);
        return result;
    }

    TraversalResult DFS::traverseAll(const Graph& graph, IGraphVisitor* visitor)
    {
        const int32_t n = graph.getNodeCount();
        TraversalResult result;
        result.order.reserve(static_cast<size_t>(n));
        result.parent.assign(static_cast<size_t>(n), -1);
        result.distance.assign(static_cast<size_t>(n), INT32_MAX);

        // Iterate over all nodes, starting DFS from each unvisited one
        for (int32_t i = 0; i < n; ++i)
        {
            if (result.distance[static_cast<size_t>(i)] == INT32_MAX)
            {
                dfsIterative(graph, i, result, visitor);
            }
        }

        return result;
    }

    std::vector<int32_t> DFS::findPath(const Graph& graph,
                                        const int32_t start, const int32_t target)
    {
        // Perform single-source DFS from start, then reconstruct path
        const auto result = traverse(graph, start);
        return result.reconstructPath(target);
    }
}
