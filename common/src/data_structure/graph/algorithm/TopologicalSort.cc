/**
 * @file TopologicalSort.cc
 * @brief Topological sort algorithms implementation
 * @details Two implementations:
 *          1. sort(): DFS-based â€?performs DFS post-order traversal, reverses the result
 *          2. sortKahn(): Kahn's algorithm â€?repeatedly removes nodes with zero in-degree
 *          Both detect cycles and return an empty order if the graph is cyclic.
 */

#include "data_structure/graph/algorithm/TopologicalSort.hpp"

#include <queue>
#include <stack>
#include <algorithm>

namespace cppforge::data_structure::graph::algorithm
{
    namespace
    {
        /// @brief DFS-based topological sort helper
        /// @param state 0=unvisited, 1=in-progress, 2=finished
        /// @param order Output: nodes in post-order (reversed later)
        /// @return False if a cycle is detected
        bool dfsTopo(const Graph& graph, int32_t u,
                     std::vector<int32_t>& state,
                     std::vector<int32_t>& order)
        {
            const size_t u_idx = static_cast<size_t>(u);
            state[u_idx] = 1;  // Mark as in-progress

            for (const auto& edge : graph.getAdjList(u))
            {
                const int32_t v = edge.to();
                const size_t v_idx = static_cast<size_t>(v);

                if (state[v_idx] == 1) return false;  // Back edge = cycle
                if (state[v_idx] == 0)
                {
                    if (!dfsTopo(graph, v, state, order))
                    {
                        return false;
                    }
                }
            }

            state[u_idx] = 2;  // Mark as finished
            order.push_back(u);  // Post-order: add after exploring all descendants
            return true;
        }
    }

    TopologicalSortResult TopologicalSort::sort(const Graph& graph)
    {
        const int32_t n = graph.getNodeCount();
        TopologicalSortResult result;
        std::vector<int32_t> state(static_cast<size_t>(n), 0);

        for (int32_t i = 0; i < n; ++i)
        {
            if (state[static_cast<size_t>(i)] == 0)
            {
                if (!dfsTopo(graph, i, state, result.order))
                {
                    result.has_cycle = true;
                    result.order.clear();
                    return result;
                }
            }
        }

        // Reverse post-order to get topological order
        std::reverse(result.order.begin(), result.order.end());
        return result;
    }

    TopologicalSortResult TopologicalSort::sortKahn(const Graph& graph)
    {
        const int32_t n = graph.getNodeCount();
        TopologicalSortResult result;
        result.order.reserve(static_cast<size_t>(n));

        // Compute in-degree for each node
        std::vector<int32_t> in_degree(static_cast<size_t>(n), 0);
        for (int32_t u = 0; u < n; ++u)
        {
            for (const auto& edge : graph.getAdjList(u))
            {
                ++in_degree[static_cast<size_t>(edge.to())];
            }
        }

        // Queue all nodes with zero in-degree
        std::queue<int32_t> q;
        for (int32_t i = 0; i < n; ++i)
        {
            if (in_degree[static_cast<size_t>(i)] == 0)
            {
                q.push(i);
            }
        }

        int32_t visited_count = 0;
        while (!q.empty())
        {
            const int32_t u = q.front();
            q.pop();
            result.order.push_back(u);
            ++visited_count;

            // Decrease in-degree of all neighbors
            for (const auto& edge : graph.getAdjList(u))
            {
                const int32_t v = edge.to();
                if (--in_degree[static_cast<size_t>(v)] == 0)
                {
                    q.push(v);
                }
            }
        }

        // If not all nodes were visited, a cycle exists
        if (visited_count != n)
        {
            result.has_cycle = true;
            result.order.clear();
        }

        return result;
    }
}
