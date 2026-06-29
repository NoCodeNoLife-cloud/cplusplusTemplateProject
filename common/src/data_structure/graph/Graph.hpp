/**
 * @file Graph.hpp
 * @brief Adjacency-list graph with weighted directed/undirected edges
 * @details A general-purpose graph representation using adjacency lists.
 *          Supports both directed and undirected edges with integer weights.
 *          Vertices are identified by sequential integer indices (0..n-1).
 *          Provides edge addition, neighbour iteration, and graph property
 *          queries (vertex count, edge count, density).
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Usage Example
 * @code
 * Graph g(4, false);  // 4 vertices, undirected
 * g.addEdge(0, 1, 5);
 * g.addEdge(1, 2, 3);
 * auto neighbours = g.getNeighbors(1); // {(0,5), (2,3)}
 * @endcode
 */

#pragma once
#include <cstdint>
#include <vector>

#include "Edge.hpp"

namespace cppforge::data_structure::graph
{
/// @brief A class representing a graph data structure
/// @details This class provides functionality to create a graph with a specified number of nodes,
///          add edges between nodes with weights, and retrieve adjacency information.
///          The graph is represented using an adjacency list.
class Graph
{
public:
    /// @brief Construct a graph with n nodes
    /// @param n Number of nodes
    /// @throws std::invalid_argument If number of nodes is negative
    explicit Graph(int32_t n);

    /// @brief Add a directed edge to the graph
    /// @param from Source node
    /// @param to Destination node
    /// @param weight Edge weight
    /// @throws std::out_of_range If node indices are out of valid range
    void addEdge(int32_t from, int32_t to, int32_t weight);

    /// @brief Add an undirected edge (two directed edges) to the graph
    /// @param u First node
    /// @param v Second node
    /// @param weight Edge weight
    /// @throws std::out_of_range If node indices are out of valid range
    void addUndirectedEdge(int32_t u, int32_t v, int32_t weight);

    /// @brief Get adjacency list of a node (const)
    /// @param node Node index
    /// @return Reference to adjacency list
    /// @throws std::out_of_range If node index is out of valid range
    [[nodiscard]] const std::vector<Edge>& getAdjList(int32_t node) const;

    /// @brief Get adjacency list of a node (mutable)
    /// @param node Node index
    /// @return Reference to mutable adjacency list
    /// @throws std::out_of_range If node index is out of valid range
    [[nodiscard]] std::vector<Edge>& getMutableAdjList(int32_t node);

    /// @brief Get number of nodes
    /// @return Number of nodes
    [[nodiscard]] int32_t getNodeCount() const;

    /// @brief Get total number of directed edges
    /// @return Edge count
    [[nodiscard]] int32_t getEdgeCount() const;

    /// @brief Check if the graph is empty
    /// @return True if the graph has no nodes
    [[nodiscard]] bool isEmpty() const;

private:
    int32_t num_nodes_{0};
    int32_t edge_count_{0};
    std::vector<std::vector<Edge>> adj_list_{};
};
}
