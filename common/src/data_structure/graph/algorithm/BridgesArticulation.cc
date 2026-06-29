/**
 * @file BridgesArticulation.cc
 * @brief Bridges and articulation points detection implementation
 * @details Uses a single DFS pass to compute tin (entry time) and low
 *          (lowest reachable tin via subtree + one back edge) for each node.
 *
 *          Bridge detection: edge (u,v) is a bridge if low[v] > tin[u]
 *          (v cannot reach u or any ancestor without crossing (u,v)).
 *
 *          Articulation point detection:
 *          - Root of DFS tree: articulation point if it has â‰?2 children
 *          - Non-root: node u is articulation point if low[v] >= tin[u]
 *            for any child v (v cannot reach u's ancestors without going
 *            through u).
 *
 *          Time complexity: O(V+E).
 */

#include <cppforge/data_structure/graph/algorithm/BridgesArticulation.hpp>

#include <algorithm>

namespace cppforge::data_structure::graph::algorithm
{
    namespace
    {
        /// @brief DFS to compute tin/low and detect bridges/articulation points
        /// @param tin Entry time (discovery order) for each node
        /// @param low Lowest tin reachable via subtree + one back edge
        /// @param timer Current discovery time counter
        void dfsBridges(const Graph& graph, int32_t u, int32_t parent_node,
                        std::vector<int32_t>& tin,
                        std::vector<int32_t>& low,
                        std::vector<bool>& visited,
                        int32_t& timer,
                        BridgesArticulationResult& result)
        {
            const size_t u_idx = static_cast<size_t>(u);
            visited[u_idx] = true;
            tin[u_idx] = timer;
            low[u_idx] = timer;
            ++timer;
            int32_t children = 0;

            for (const auto& edge : graph.getAdjList(u))
            {
                const int32_t v = edge.to();
                const size_t v_idx = static_cast<size_t>(v);

                // Skip the edge back to parent
                if (v == parent_node) continue;

                if (visited[v_idx])
                {
                    // Back edge: update lowlink with the neighbor's tin
                    low[u_idx] = std::min(low[u_idx], tin[v_idx]);
                }
                else
                {
                    // Tree edge: recurse, then update lowlink
                    dfsBridges(graph, v, u, tin, low, visited, timer, result);
                    low[u_idx] = std::min(low[u_idx], low[v_idx]);

                    // Bridge: no back edge from v's subtree to u or its ancestors
                    if (low[v_idx] > tin[u_idx])
                    {
                        result.bridges.emplace_back(u, v);
                    }

                    // Articulation point (non-root): child cannot reach u's ancestors
                    if (parent_node != -1 && low[v_idx] >= tin[u_idx])
                    {
                        result.articulation_points.push_back(u);
                    }

                    ++children;
                }
            }

            // Articulation point (root): has at least two children in DFS tree
            if (parent_node == -1 && children > 1)
            {
                result.articulation_points.push_back(u);
            }
        }
    }

    BridgesArticulationResult BridgesArticulation::compute(const Graph& graph)
    {
        const int32_t n = graph.getNodeCount();
        BridgesArticulationResult result;

        if (n == 0) return result;

        std::vector<int32_t> tin(static_cast<size_t>(n), -1);
        std::vector<int32_t> low(static_cast<size_t>(n), -1);
        std::vector<bool> visited(static_cast<size_t>(n), false);
        int32_t timer = 0;

        // Run DFS from each unvisited node (handles disconnected graphs)
        for (int32_t i = 0; i < n; ++i)
        {
            if (!visited[static_cast<size_t>(i)])
            {
                dfsBridges(graph, i, -1, tin, low, visited, timer, result);
            }
        }

        return result;
    }
}
