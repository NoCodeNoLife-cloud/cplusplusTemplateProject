/**
 * @file Graph.hpp
 * @brief Graph class declaration
 * @details This header defines the Graph class that provides functionality for graph data structures.
 */

#pragma once
#include <cstdint>
#include <vector>

#include "Edge.hpp"

namespace common::data_structure::graph
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

        /// @brief Add an edge to the graph
        /// @param from Source node
        /// @param to Destination node
        /// @param weight Edge weight
        /// @throws std::out_of_range If node indices are out of valid range
        void addEdge(int32_t from, int32_t to, int32_t weight);

        /// @brief Get adjacency list of a node
        /// @param node Node index
        /// @return Reference to adjacency list
        /// @throws std::out_of_range If node index is out of valid range
        [[nodiscard]] const std::vector<Edge>& getAdjList(int32_t node) const;

        /// @brief Get number of nodes
        /// @return Number of nodes
        [[nodiscard]] int32_t getNodeCount() const ;

        /// @brief Check if the graph is empty
        /// @return True if the graph has no nodes
        [[nodiscard]] bool isEmpty() const ;

    private:
        int32_t num_nodes_{0};
        std::vector<std::vector<Edge>> adj_list_{};
    };
}
