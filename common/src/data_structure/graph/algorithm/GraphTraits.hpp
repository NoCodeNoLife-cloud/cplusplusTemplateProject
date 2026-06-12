/**
 * @file GraphTraits.hpp
 * @brief Common result types for graph algorithms
 * @details This header defines all result structures returned by graph algorithms.
 *          Each algorithm produces a structured result containing distances, paths,
 *          component information, or other algorithm-specific data. Result types
 *          include helper methods like reconstructPath() for path retrieval.
 */

#pragma once
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

namespace common::data_structure::graph::algorithm
{
    /// @brief Represents an edge with full source-target-weight information
    /// @details Unlike the internal Edge class (which only stores to + weight),
    ///          EdgeInfo stores the complete triple (from, to, weight) needed by
    ///          algorithms like Kruskal's MST that operate on edge lists.
    struct EdgeInfo
    {
        int32_t from;
        int32_t to;
        int32_t weight;
    };

    /// @brief Result of a graph traversal (BFS or DFS)
    /// @details Contains the visit order, parent pointers for path reconstruction,
    ///          and distance (number of edges) from the source node to each node.
    struct TraversalResult
    {
        std::vector<int32_t> order;      ///< Nodes in the order they were visited
        std::vector<int32_t> parent;     ///< parent[node] = predecessor in traversal (-1 for root)
        std::vector<int32_t> distance;   ///< distance[node] = number of edges from source (INT32_MAX for unreachable)

        /// @brief Reconstruct the path from source to the given target node
        /// @param to Target node
        /// @return Path from source to target (inclusive), empty if unreachable
        [[nodiscard]] std::vector<int32_t> reconstructPath(int32_t to) const
        {
            std::vector<int32_t> path;
            if (to < 0 || static_cast<size_t>(to) >= parent.size())
            {
                return path;
            }
            if (distance[to] == INT32_MAX)
            {
                return path;
            }
            for (int32_t v = to; v != -1; v = parent[static_cast<size_t>(v)])
            {
                path.push_back(v);
            }
            std::reverse(path.begin(), path.end());
            return path;
        }
    };

    /// @brief Result of a single-source shortest path algorithm (Dijkstra, Bellman-Ford)
    /// @details Contains distances and predecessor information for reconstructing
    ///          the shortest path from the source to any reachable node.
    struct ShortestPathResult
    {
        int32_t source{-1};                      ///< Source node of the shortest path computation
        int32_t target{-1};                      ///< Target node (-1 if computing all reachable nodes)
        std::vector<int32_t> distance;           ///< distance[node] = shortest distance from source (INT32_MAX for unreachable)
        std::vector<int32_t> previous;           ///< previous[node] = predecessor on shortest path (-1 for source/unreachable)

        /// @brief Check if the source is valid and distances were computed
        /// @return True if the result contains valid distance data
        [[nodiscard]] bool hasPath() const
        {
            return !distance.empty();
        }

        /// @brief Reconstruct the shortest path from source to the given target node
        /// @param to Target node
        /// @return Shortest path from source to target (inclusive), empty if unreachable
        [[nodiscard]] std::vector<int32_t> reconstructPath(int32_t to) const
        {
            std::vector<int32_t> path;
            if (to < 0 || static_cast<size_t>(to) >= distance.size())
            {
                return path;
            }
            if (distance[to] == INT32_MAX)
            {
                return path;
            }
            for (int32_t v = to; v != -1; v = previous[static_cast<size_t>(v)])
            {
                path.push_back(v);
            }
            std::reverse(path.begin(), path.end());
            return path;
        }
    };

    /// @brief Result of all-pairs shortest path (Floyd-Warshall)
    /// @details Contains the full distance matrix and next-node matrix for path
    ///          reconstruction between any pair of nodes.
    struct AllPairsShortestPathResult
    {
        std::vector<std::vector<int32_t>> distances;  ///< distances[u][v] = shortest distance from u to v
        std::vector<std::vector<int32_t>> next;        ///< next[u][v] = the next node on the shortest path from u to v
        bool has_negative_cycle{false};                ///< Whether the graph contains a negative-weight cycle

        /// @brief Reconstruct the shortest path between two nodes
        /// @param from Source node
        /// @param to Target node
        /// @return Shortest path from 'from' to 'to' (inclusive), empty if unreachable or negative cycle exists
        [[nodiscard]] std::vector<int32_t> reconstructPath(int32_t from, int32_t to) const
        {
            std::vector<int32_t> path;
            if (has_negative_cycle || from < 0 || to < 0 ||
                static_cast<size_t>(from) >= next.size() || static_cast<size_t>(to) >= next.size())
            {
                return path;
            }
            if (next[from][to] == -1)
            {
                return path;
            }
            path.push_back(from);
            int32_t current = from;
            while (current != to)
            {
                current = next[current][to];
                if (current == -1 || current == from)
                {
                    path.clear();
                    return path;
                }
                path.push_back(current);
            }
            return path;
        }
    };

    /// @brief Result of topological sort
    /// @details Contains the topological ordering of nodes and a flag indicating
    ///          whether the graph contains a cycle (making topological sort impossible).
    struct TopologicalSortResult
    {
        std::vector<int32_t> order;   ///< Nodes in topological order (empty if cycle exists)
        bool has_cycle{false};        ///< True if the graph contains a directed cycle
    };

    /// @brief Result of cycle detection
    /// @details Contains whether a cycle was found and the actual cycle nodes if detected.
    struct CycleResult
    {
        bool has_cycle{false};          ///< Whether a cycle was detected
        std::vector<int32_t> cycle;     ///< Nodes forming a cycle (empty if no cycle found)
    };

    /// @brief Result of bipartite graph checking
    /// @details Contains whether the graph is bipartite and a valid 2-coloring.
    struct BipartiteResult
    {
        bool is_bipartite{true};         ///< Whether the graph is bipartite
        std::vector<int32_t> color;      ///< color[node] = 0 or 1 (-1 for unvisited nodes)
    };

    /// @brief Result of minimum spanning tree computation
    /// @details Contains the edges forming the MST and the total weight.
    struct MSTResult
    {
        std::vector<EdgeInfo> edges;     ///< Edges in the minimum spanning tree
        int32_t total_weight{0};         ///< Sum of edge weights in the MST
    };

    /// @brief Result of strongly connected components (Tarjan's algorithm)
    /// @details Contains the list of SCCs, each as a vector of node indices,
    ///          and a mapping from each node to its component index.
    struct SCCResult
    {
        std::vector<std::vector<int32_t>> components;  ///< List of SCCs, each is a vector of nodes
        std::vector<int32_t> component_id;              ///< component_id[node] = index into components[]
    };

    /// @brief Result of bridges and articulation points detection
    /// @details Contains lists of bridges (critical edges) and articulation points
    ///          (critical nodes) whose removal disconnects the graph.
    struct BridgesArticulationResult
    {
        std::vector<std::pair<int32_t, int32_t>> bridges;  ///< List of bridge edges (u, v)
        std::vector<int32_t> articulation_points;           ///< List of articulation point nodes
    };
}
