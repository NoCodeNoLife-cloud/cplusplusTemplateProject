/**
 * @file Graph.cc
 * @brief Graph class implementation
 * @details This file contains the implementation of the Graph class methods for Graphics and geometry utilities.
 */

#include "graphics/Graph.hpp"

#include <fmt/format.h>
#include <vector>
#include <stdexcept>
#include <string>
#include <glog/logging.h>

#include "graphics/Edge.hpp"

namespace common::graphics
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
            throw std::out_of_range("Graph::addEdge: Node index out of range. Requested: from=" + std::to_string(from) + ", to=" + std::to_string(to) + ", but graph has " + std::to_string(num_nodes_) + " nodes");
        }
        Edge edge(to, weight);
        adj_list_[static_cast<size_t>(from)].emplace_back(edge);
    }

    const std::vector<Edge>& Graph::getAdjList(const int32_t node) const
    {
        if (node < 0 || node >= num_nodes_)
        {
            DLOG(WARNING) << fmt::format("Graph getAdjList: Node index out of range. node={}, num_nodes={}", node, num_nodes_);
            throw std::out_of_range("Graph::getAdjList: Node index out of range. Requested: " + std::to_string(node) + ", but graph has " + std::to_string(num_nodes_) + " nodes");
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