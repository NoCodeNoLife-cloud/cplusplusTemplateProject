/**
 * @file BipartiteCheck.cc
 * @brief Bipartite graph checking implementation
 * @details Uses BFS to 2-color the graph. Starting from each uncolored node,
 *          assigns color 0 and BFS-traverses, assigning alternating colors.
 *          If a neighbor has the same color as the current node, the graph
 *          is not bipartite (it contains an odd-length cycle).
 */

#include "data_structure/graph/algorithm/BipartiteCheck.hpp"

#include <queue>

namespace cppforge::data_structure::graph::algorithm
{
    BipartiteResult BipartiteCheck::check(const Graph& graph)
    {
        const int32_t n = graph.getNodeCount();
        BipartiteResult result;
        result.color.assign(static_cast<size_t>(n), -1);
        result.is_bipartite = true;

        // Check each connected component
        for (int32_t start = 0; start < n; ++start)
        {
            if (result.color[static_cast<size_t>(start)] != -1) continue;

            // BFS to 2-color this component
            std::queue<int32_t> q;
            result.color[static_cast<size_t>(start)] = 0;
            q.push(start);

            while (!q.empty() && result.is_bipartite)
            {
                const int32_t u = q.front();
                q.pop();
                const int32_t next_color = 1 - result.color[static_cast<size_t>(u)];

                for (const auto& edge : graph.getAdjList(u))
                {
                    const int32_t v = edge.to();
                    auto& v_color = result.color[static_cast<size_t>(v)];

                    if (v_color == -1)
                    {
                        v_color = next_color;
                        q.push(v);
                    }
                    else if (v_color != next_color)
                    {
                        // Conflict: adjacent nodes have same color
                        result.is_bipartite = false;
                        break;
                    }
                }
            }

            if (!result.is_bipartite) break;
        }

        return result;
    }
}
