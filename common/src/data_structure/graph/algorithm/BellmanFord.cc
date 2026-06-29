/**
 * @file BellmanFord.cc
 * @brief Bellman-Ford shortest path algorithm implementation
 * @details Principle: repeatedly relax all edges V-1 times. Since the shortest
 *          path in a graph with V nodes can have at most V-1 edges, V-1 rounds
 *          of relaxation are sufficient. If an edge can still be relaxed on the
 *          V-th round, a negative-weight cycle exists.
 *
 *          shortestPath(): Runs V-1 relaxations from a single source.
 *          hasNegativeCycle(): Initializes all distances to 0 (virtual source)
 *          and detects if any edge can still be relaxed after V-1 rounds.
 */

#include <cppforge/data_structure/graph/algorithm/BellmanFord.hpp>

namespace cppforge::data_structure::graph::algorithm
{
    ShortestPathResult BellmanFord::shortestPath(const Graph& graph, const int32_t start)
    {
        const int32_t n = graph.getNodeCount();
        ShortestPathResult result;
        result.source = start;
        result.distance.assign(static_cast<size_t>(n), INT32_MAX);
        result.previous.assign(static_cast<size_t>(n), -1);

        if (n == 0 || start < 0 || start >= n)
        {
            return result;
        }

        const auto start_idx = static_cast<size_t>(start);
        result.distance[start_idx] = 0;

        // Relax all edges V-1 times
        for (int32_t i = 0; i < n - 1; ++i)
        {
            bool updated = false;

            // Iterate over all edges: for each node u, check all outgoing edges
            for (int32_t u = 0; u < n; ++u)
            {
                const auto u_idx = static_cast<size_t>(u);
                if (result.distance[u_idx] == INT32_MAX) continue;

                for (const auto& edge : graph.getAdjList(u))
                {
                    const int32_t v = edge.to();
                    const auto v_idx = static_cast<size_t>(v);
                    const int32_t new_dist = result.distance[u_idx] + edge.weight();

                    if (new_dist < result.distance[v_idx])
                    {
                        result.distance[v_idx] = new_dist;
                        result.previous[v_idx] = u;
                        updated = true;
                    }
                }
            }

            // Early termination if no update occurred
            if (!updated) break;
        }

        return result;
    }

    bool BellmanFord::hasNegativeCycle(const Graph& graph)
    {
        const int32_t n = graph.getNodeCount();
        if (n == 0) return false;

        // Initialize all distances to 0 (acts as a virtual super-source
        // connected to all nodes with weight 0)
        std::vector<int32_t> dist(static_cast<size_t>(n), 0);

        // Run V-1 relaxations
        for (int32_t i = 0; i < n - 1; ++i)
        {
            bool updated = false;
            for (int32_t u = 0; u < n; ++u)
            {
                const auto u_idx = static_cast<size_t>(u);
                if (dist[u_idx] == INT32_MAX) continue;

                for (const auto& edge : graph.getAdjList(u))
                {
                    const int32_t v = edge.to();
                    const auto v_idx = static_cast<size_t>(v);
                    const int32_t new_dist = dist[u_idx] + edge.weight();

                    if (new_dist < dist[v_idx])
                    {
                        dist[v_idx] = new_dist;
                        updated = true;
                    }
                }
            }
            if (!updated) return false;  // Converged early = no negative cycle
        }

        // V-th pass: if still relaxing, a negative cycle exists
        for (int32_t u = 0; u < n; ++u)
        {
            const auto u_idx = static_cast<size_t>(u);
            if (dist[u_idx] == INT32_MAX) continue;

            for (const auto& edge : graph.getAdjList(u))
            {
                const int32_t v = edge.to();
                const auto v_idx = static_cast<size_t>(v);
                if (dist[u_idx] + edge.weight() < dist[v_idx])
                {
                    return true;  // Can still relax â†?negative cycle
                }
            }
        }

        return false;
    }
}
