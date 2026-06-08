/**
 * @file Graph.cc
 * @brief Graph class implementation
 * @details This file contains the implementation of the Graph class methods for graph data structures.
 */

#include "data_structure/graph/Graph.hpp"

#include <stdexcept>
#include <fmt/format.h>
#include <glog/logging.h>

#include "data_structure/graph/Edge.hpp"

namespace common::data_structure::graph
{
    Graph::Graph(const int32_t n) : num_nodes_(n)
    {
        if (n < 0)
        {
            DLOG(WARNING) << fmt::format("Graph constructor: Number of nodes cannot be negative, got {}", n);
            throw std::invalid_argument("Graph::Graph: Number of nodes cannot be negative");
        }
        adj_list_.resize(static_cast<size_t>(num_nodes_));
    }

    void Graph::addEdge(const int32_t from, const int32_t to, const int32_t weight)
    {
        if (from < 0 || from >= num_nodes_ || to < 0 || to >= num_nodes_)
        {
            DLOG(WARNING) << fmt::format("Graph addEdge: Node index out of range. from={}, to={}, num_nodes={}", from, to, num_nodes_);
            throw std::out_of_range(fmt::format("Graph::addEdge: Node index out of range. Requested: from={}, to={}, but graph has {} nodes", from, to, num_nodes_));
        }
        adj_list_[static_cast<size_t>(from)].emplace_back(to, weight);
    }

    const std::vector<Edge>& Graph::getAdjList(const int32_t node) const
    {
        if (node < 0 || node >= num_nodes_)
        {
            DLOG(WARNING) << fmt::format("Graph getAdjList: Node index out of range. node={}, num_nodes={}", node, num_nodes_);
            throw std::out_of_range(fmt::format("Graph::getAdjList: Node index out of range. Requested: {}, but graph has {} nodes", node, num_nodes_));
        }
        return adj_list_[static_cast<size_t>(node)];
    }

    int32_t Graph::getNodeCount() const
    {
        return num_nodes_;
    }

    bool Graph::isEmpty() const
    {
        return num_nodes_ == 0;
    }
}
