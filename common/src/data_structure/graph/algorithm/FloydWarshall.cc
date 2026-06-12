/**
 * @file FloydWarshall.cc
 * @brief Floyd-Warshall all-pairs shortest path implementation
 * @details Uses dynamic programming: dp[i][j] = min(dp[i][j], dp[i][k] + dp[k][j])
 *          for k = 0..V-1. Initializes the distance matrix with edge weights and
 *          INT32_MAX/2 for non-edges (to avoid overflow during addition).
 *          After computation, checks the diagonal: if any dist[i][i] < 0,
 *          a negative-weight cycle exists.
 *
 *          The next[][] matrix stores the first step on the shortest path from
 *          i to j, enabling path reconstruction.
 */

#include "data_structure/graph/algorithm/FloydWarshall.hpp"

namespace common::data_structure::graph::algorithm
{
    AllPairsShortestPathResult FloydWarshall::compute(const Graph& graph)
    {
        const int32_t n = graph.getNodeCount();
        AllPairsShortestPathResult result;

        const auto size = static_cast<size_t>(n);
        // Initialize with INT32_MAX/2 to avoid overflow when adding two INF values
        result.distances.assign(size, std::vector<int32_t>(size, INT32_MAX / 2));
        result.next.assign(size, std::vector<int32_t>(size, -1));

        // Distance from a node to itself is 0
        for (int32_t i = 0; i < n; ++i)
        {
            result.distances[static_cast<size_t>(i)][static_cast<size_t>(i)] = 0;
            result.next[static_cast<size_t>(i)][static_cast<size_t>(i)] = i;
        }

        // Initialize with direct edge weights
        for (int32_t u = 0; u < n; ++u)
        {
            const auto u_idx = static_cast<size_t>(u);
            for (const auto& edge : graph.getAdjList(u))
            {
                const int32_t v = edge.to();
                const auto v_idx = static_cast<size_t>(v);
                result.distances[u_idx][v_idx] = edge.weight();
                result.next[u_idx][v_idx] = v;
            }
        }

        // Main DP: for each intermediate node k, check if better path exists
        for (int32_t k = 0; k < n; ++k)
        {
            const auto k_idx = static_cast<size_t>(k);
            for (int32_t i = 0; i < n; ++i)
            {
                const auto i_idx = static_cast<size_t>(i);
                for (int32_t j = 0; j < n; ++j)
                {
                    const auto j_idx = static_cast<size_t>(j);
                    const int32_t new_dist = result.distances[i_idx][k_idx] + result.distances[k_idx][j_idx];
                    if (new_dist < result.distances[i_idx][j_idx])
                    {
                        result.distances[i_idx][j_idx] = new_dist;
                        result.next[i_idx][j_idx] = result.next[i_idx][k_idx];
                    }
                }
            }
        }

        // Check for negative-weight cycles: any node reachable from itself
        // with negative distance indicates a negative cycle
        for (int32_t i = 0; i < n; ++i)
        {
            if (result.distances[static_cast<size_t>(i)][static_cast<size_t>(i)] < 0)
            {
                result.has_negative_cycle = true;
                break;
            }
        }

        return result;
    }
}
