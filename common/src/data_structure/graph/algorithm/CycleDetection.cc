/**
 * @file CycleDetection.cc
 * @brief Cycle detection algorithms implementation
 * @details Uses DFS with 3-state coloring:
 *          0 = unvisited, 1 = in current DFS path (in-progress), 2 = finished.
 *          A cycle is found when a back edge to an in-progress node is detected.
 *          For undirected graphs, the immediate parent edge is excluded.
 */

#include "data_structure/graph/algorithm/CycleDetection.hpp"

#include <vector>
#include <algorithm>

namespace cppforge::data_structure::graph::algorithm
{
    namespace
    {
        /// @brief DFS-based cycle detection for directed graphs
        /// @param state 0=unvisited, 1=in-progress, 2=finished
        /// @param cycle Output: nodes forming the detected cycle
        /// @return True if a cycle is detected
        bool dfsDetectDirected(const Graph& graph, int32_t u,
                               std::vector<int32_t>& state,
                               std::vector<int32_t>& parent,
                               std::vector<int32_t>& cycle)
        {
            const size_t u_idx = static_cast<size_t>(u);
            state[u_idx] = 1;  // Mark as in-progress

            for (const auto& edge : graph.getAdjList(u))
            {
                const int32_t v = edge.to();
                const size_t v_idx = static_cast<size_t>(v);

                if (state[v_idx] == 1)
                {
                    // Back edge found: reconstruct the cycle
                    cycle.push_back(v);
                    for (int32_t x = u; x != v; x = parent[static_cast<size_t>(x)])
                    {
                        cycle.push_back(x);
                    }
                    cycle.push_back(v);
                    std::reverse(cycle.begin(), cycle.end());
                    return true;
                }

                if (state[v_idx] == 0)
                {
                    parent[v_idx] = u;
                    if (dfsDetectDirected(graph, v, state, parent, cycle))
                    {
                        return true;
                    }
                }
            }

            state[u_idx] = 2;  // Mark as finished
            return false;
        }

        /// @brief DFS-based cycle detection for undirected graphs
        /// @details Skips the immediate parent to avoid false positive cycles
        bool dfsDetectUndirected(const Graph& graph, int32_t u, int32_t parent_node,
                                 std::vector<int32_t>& visited,
                                 std::vector<int32_t>& parent_arr,
                                 std::vector<int32_t>& cycle)
        {
            const size_t u_idx = static_cast<size_t>(u);
            visited[u_idx] = 1;

            for (const auto& edge : graph.getAdjList(u))
            {
                const int32_t v = edge.to();
                const size_t v_idx = static_cast<size_t>(v);

                // Skip the edge back to parent (would be a false positive in undirected)
                if (v == parent_node) continue;

                if (visited[v_idx] == 1)
                {
                    // Back edge found
                    cycle.push_back(v);
                    for (int32_t x = u; x != v; x = parent_arr[static_cast<size_t>(x)])
                    {
                        cycle.push_back(x);
                    }
                    cycle.push_back(v);
                    std::reverse(cycle.begin(), cycle.end());
                    return true;
                }

                if (visited[v_idx] == 0)
                {
                    parent_arr[v_idx] = u;
                    if (dfsDetectUndirected(graph, v, u, visited, parent_arr, cycle))
                    {
                        return true;
                    }
                }
            }

            return false;
        }
    }

    CycleResult CycleDetection::detectDirected(const Graph& graph)
    {
        const int32_t n = graph.getNodeCount();
        CycleResult result;
        std::vector<int32_t> state(static_cast<size_t>(n), 0);
        std::vector<int32_t> parent(static_cast<size_t>(n), -1);

        // Start DFS from each unvisited node
        for (int32_t i = 0; i < n; ++i)
        {
            if (state[static_cast<size_t>(i)] == 0)
            {
                if (dfsDetectDirected(graph, i, state, parent, result.cycle))
                {
                    result.has_cycle = true;
                    break;
                }
            }
        }

        return result;
    }

    CycleResult CycleDetection::detectUndirected(const Graph& graph)
    {
        const int32_t n = graph.getNodeCount();
        CycleResult result;
        std::vector<int32_t> visited(static_cast<size_t>(n), 0);
        std::vector<int32_t> parent(static_cast<size_t>(n), -1);

        for (int32_t i = 0; i < n; ++i)
        {
            if (visited[static_cast<size_t>(i)] == 0)
            {
                if (dfsDetectUndirected(graph, i, -1, visited, parent, result.cycle))
                {
                    result.has_cycle = true;
                    break;
                }
            }
        }

        return result;
    }
}
