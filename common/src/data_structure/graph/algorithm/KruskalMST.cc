/**
 * @file KruskalMST.cc
 * @brief Kruskal's Minimum Spanning Tree algorithm implementation
 * @details Sorts all edges by weight, then greedily adds edges that connect
 *          two different connected components (detected via Union-Find).
 *          Time complexity: O(ElogE) from sorting.
 *
 *          The internal UnionFind class implements path compression (in find)
 *          and union by rank (in unite) for near-constant time operations.
 */

#include "data_structure/graph/algorithm/KruskalMST.hpp"

#include <algorithm>

namespace cppforge::data_structure::graph::algorithm
{
    namespace
    {
        /// @brief Union-Find (Disjoint Set Union) with path compression + union by rank
        class UnionFind
        {
        public:
            explicit UnionFind(int32_t n)
                : parent_(static_cast<size_t>(n)),
                  rank_(static_cast<size_t>(n), 0)
            {
                for (int32_t i = 0; i < n; ++i)
                {
                    parent_[static_cast<size_t>(i)] = i;
                }
            }

            /// @brief Find the representative of the set containing x (with path compression)
            int32_t find(int32_t x)
            {
                const size_t x_idx = static_cast<size_t>(x);
                if (parent_[x_idx] != x)
                {
                    parent_[x_idx] = find(parent_[x_idx]);
                }
                return parent_[x_idx];
            }

            /// @brief Union the sets containing a and b (by rank)
            /// @return True if the sets were different (successful union)
            bool unite(int32_t a, int32_t b)
            {
                const int32_t ra = find(a);
                const int32_t rb = find(b);
                if (ra == rb) return false;

                const size_t ra_idx = static_cast<size_t>(ra);
                const size_t rb_idx = static_cast<size_t>(rb);

                if (rank_[ra_idx] < rank_[rb_idx])
                {
                    parent_[ra_idx] = rb;
                }
                else if (rank_[ra_idx] > rank_[rb_idx])
                {
                    parent_[rb_idx] = ra;
                }
                else
                {
                    parent_[rb_idx] = ra;
                    ++rank_[ra_idx];
                }
                return true;
            }

        private:
            std::vector<int32_t> parent_;
            std::vector<int32_t> rank_;
        };
    }

    MSTResult KruskalMST::compute(const Graph& graph)
    {
        const int32_t n = graph.getNodeCount();
        MSTResult result;

        if (n == 0) return result;

        // Collect all edges (undirected: only add each edge once via u <= v)
        std::vector<EdgeInfo> all_edges;
        for (int32_t u = 0; u < n; ++u)
        {
            for (const auto& edge : graph.getAdjList(u))
            {
                if (u <= edge.to())
                {
                    all_edges.push_back({u, edge.to(), edge.weight()});
                }
            }
        }

        // Sort edges by weight ascending
        std::sort(all_edges.begin(), all_edges.end(),
                  [](const EdgeInfo& a, const EdgeInfo& b)
                  {
                      return a.weight < b.weight;
                  });

        // Greedily add edges that don't form a cycle
        UnionFind uf(n);
        for (const auto& e : all_edges)
        {
            if (uf.unite(e.from, e.to))
            {
                result.edges.push_back(e);
                result.total_weight += e.weight;
            }
        }

        return result;
    }
}
