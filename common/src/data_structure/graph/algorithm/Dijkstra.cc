/**
 * @file Dijkstra.cc
 * @brief Dijkstra's shortest path algorithm — priority-queue implementation
 * @details Implements single-source shortest paths for graphs with non-negative
 *          edge weights.  Uses a std::priority_queue (min-heap) to extract the
 *          minimum-distance node in O(log V).  Each edge is relaxed at most
 *          once per node.  Time complexity: O((V + E) log V).
 *
 *          Negative edge weights are not supported; the algorithm explicitly
 *          guards against negative distances to prevent overflow.
 *
 * Reference: Dijkstra, "A Note on Two Problems in Connexion with Graphs"
 *            (1959), Numerische Mathematik 1, 269–271.
 */

#include "data_structure/graph/algorithm/Dijkstra.hpp"

#include <queue>

namespace common::data_structure::graph::algorithm
{
    namespace
    {
        /// @brief Internal state for the priority queue
        struct DijkstraState
        {
            int32_t node;
            int32_t dist;

            // Min-heap ordering: smaller dist has higher priority
            bool operator>(const DijkstraState& other) const
            {
                return dist > other.dist;
            }
        };
    }

    ShortestPathResult Dijkstra::shortestPath(const Graph& graph, const int32_t start)
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

        const size_t start_idx = static_cast<size_t>(start);
        result.distance[start_idx] = 0;

        // Min-heap: (distance, node)
        std::priority_queue<DijkstraState, std::vector<DijkstraState>, std::greater<>> pq;
        pq.push({start, 0});

        while (!pq.empty())
        {
            const auto [u, d] = pq.top();
            pq.pop();
            const size_t u_idx = static_cast<size_t>(u);

            // Skip stale entries (already found a shorter path)
            if (d != result.distance[u_idx]) continue;

            // Relax all outgoing edges of u
            for (const auto& edge : graph.getAdjList(u))
            {
                const int32_t v = edge.to();
                const size_t v_idx = static_cast<size_t>(v);
                const int32_t new_dist = d + edge.weight();

                // new_dist >= 0 check prevents negative-weight overflow issues
                if (new_dist >= 0 && new_dist < result.distance[v_idx])
                {
                    result.distance[v_idx] = new_dist;
                    result.previous[v_idx] = u;
                    pq.push({v, new_dist});
                }
            }
        }

        return result;
    }

    ShortestPathResult Dijkstra::shortestPathToTarget(const Graph& graph,
                                                       const int32_t start,
                                                       const int32_t target)
    {
        // Compute all distances from start, then set the target for convenience
        auto result = shortestPath(graph, start);
        result.target = target;
        return result;
    }
}
