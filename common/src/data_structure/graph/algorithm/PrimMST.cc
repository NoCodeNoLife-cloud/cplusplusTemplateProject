/**
 * @file PrimMST.cc
 * @brief Prim's Minimum Spanning Tree algorithm implementation
 * @details Greedy algorithm that grows the MST one node at a time:
 *          1. Start with node 0 as the initial tree
 *          2. Use a min-heap to select the cheapest edge connecting a tree node
 *             to a non-tree node
 *          3. Add the selected edge and node to the tree
 *          4. Repeat until all nodes are in the tree
 *
 *          Uses a key[] array to track the minimum weight to connect each
 *          non-tree node to the current tree. Time complexity: O(ElogV).
 */

#include <cppforge/data_structure/graph/algorithm/PrimMST.hpp>

#include <queue>

namespace cppforge::data_structure::graph::algorithm
{
    namespace
    {
        /// @brief Internal state for the min-heap
        struct PrimState
        {
            int32_t node;
            int32_t key;
            bool operator>(const PrimState& other) const
            {
                return key > other.key;
            }
        };
    }

    MSTResult PrimMST::compute(const Graph& graph)
    {
        const int32_t n = graph.getNodeCount();
        MSTResult result;

        if (n == 0) return result;

        std::vector<int32_t> key(static_cast<size_t>(n), INT32_MAX);
        std::vector<int32_t> parent(static_cast<size_t>(n), -1);
        std::vector<bool> in_mst(static_cast<size_t>(n), false);

        const size_t start_idx = 0;
        key[start_idx] = 0;

        // Min-heap of (key, node)
        std::priority_queue<PrimState, std::vector<PrimState>, std::greater<>> pq;
        pq.push({0, 0});

        while (!pq.empty())
        {
            const int32_t u = pq.top().node;
            pq.pop();
            const size_t u_idx = static_cast<size_t>(u);

            if (in_mst[u_idx]) continue;
            in_mst[u_idx] = true;

            // Add edge to result (skip the root node which has no parent)
            if (parent[u_idx] != -1)
            {
                result.edges.push_back({parent[u_idx], u, key[u_idx]});
                result.total_weight += key[u_idx];
            }

            // Update keys of neighbors
            for (const auto& edge : graph.getAdjList(u))
            {
                const int32_t v = edge.to();
                const size_t v_idx = static_cast<size_t>(v);
                const int32_t w = edge.weight();

                if (!in_mst[v_idx] && w < key[v_idx])
                {
                    key[v_idx] = w;
                    parent[v_idx] = u;
                    pq.push({v, w});
                }
            }
        }

        return result;
    }
}
