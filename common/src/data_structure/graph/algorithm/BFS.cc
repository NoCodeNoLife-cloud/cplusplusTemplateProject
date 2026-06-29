/**
 * @file BFS.cc
 * @brief Breadth-First Search algorithm implementation
 * @details Uses a queue-based approach (simulated with vector + head index) to
 *          explore the graph level by level. Guarantees shortest path in terms
 *          of number of edges for unweighted graphs.
 */

#include "data_structure/graph/algorithm/BFS.hpp"

#include <vector>

namespace cppforge::data_structure::graph::algorithm
{
    TraversalResult BFS::traverse(const Graph& graph, const int32_t start,
                                   IGraphVisitor* visitor)
    {
        const int32_t n = graph.getNodeCount();
        TraversalResult result;
        const auto n_sz = static_cast<size_t>(n);
        result.order.reserve(n_sz);
        result.parent.assign(n_sz, -1);
        result.distance.assign(n_sz, INT32_MAX);

        if (n == 0 || start < 0 || start >= n)
        {
            return result;
        }

        // Queue simulated with vector + head index to avoid std::queue
        // template instantiation issues in some toolchains
        std::vector<int32_t> q;
        size_t head = 0;
        result.distance[static_cast<size_t>(start)] = 0;
        q.push_back(start);

        while (head < q.size())
        {
            const int32_t u = q[head++];

            result.order.push_back(u);
            if (visitor) visitor->onNodeVisit(u);

            // Explore all neighbors of the current node
            for (const auto& edge : graph.getAdjList(u))
            {
                const int32_t v = edge.to();
                const auto v_idx = static_cast<size_t>(v);
                // If neighbor not yet visited, mark distance and enqueue
                if (result.distance[v_idx] == INT32_MAX)
                {
                    result.distance[v_idx] = result.distance[static_cast<size_t>(u)] + 1;
                    result.parent[v_idx] = u;
                    q.push_back(v);
                    if (visitor) visitor->onEdgeTraverse(u, v, edge.weight());
                }
            }
        }

        return result;
    }

    std::vector<int32_t> BFS::shortestPath(const Graph& graph,
                                            const int32_t start, const int32_t target)
    {
        // Perform BFS from start, then reconstruct path to target
        const auto result = traverse(graph, start);
        return result.reconstructPath(target);
    }
}
