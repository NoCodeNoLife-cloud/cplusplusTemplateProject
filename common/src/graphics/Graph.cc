#include "src/graphics/Graph.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <string>

#include "src/graphics/Edge.hpp"

namespace common::graphics {
    Graph::Graph(const int32_t n) : num_nodes_(n) {
        if (n < 0) {
            DLOG(ERROR) << fmt::format("Graph initialization failed - negative node count: {}", n);
            throw std::invalid_argument("Graph::Graph: Number of nodes cannot be negative");
        }
        adj_list_.resize(static_cast<size_t>(num_nodes_));
        DLOG(INFO) << fmt::format("Graph initialized with {} nodes", n);
    }

    auto Graph::addEdge(const int32_t from, const int32_t to, const int32_t weight) -> void {
        if (from < 0 || from >= num_nodes_ || to < 0 || to >= num_nodes_) {
            DLOG(ERROR) << fmt::format("Graph addEdge failed - node index out of range: from={}, to={}, node_count={}", from, to, num_nodes_);
            throw std::out_of_range("Graph::addEdge: Node index out of range. Requested: from=" + std::to_string(from) + ", to=" + std::to_string(to) + ", but graph has " + std::to_string(num_nodes_) + " nodes");
        }
        Edge edge(to, weight);
        adj_list_[static_cast<size_t>(from)].emplace_back(edge);
        DLOG(INFO) << fmt::format("Graph addEdge - from={} to={} weight={}", from, to, weight);
    }

    auto Graph::getAdjList(const int32_t node) const -> const std::vector<Edge> & {
        if (node < 0 || node >= num_nodes_) {
            DLOG(ERROR) << fmt::format("Graph getAdjList failed - node index out of range: node={}, node_count={}", node, num_nodes_);
            throw std::out_of_range("Graph::getAdjList: Node index out of range. Requested: " + std::to_string(node) + ", but graph has " + std::to_string(num_nodes_) + " nodes");
        }
        return adj_list_[static_cast<size_t>(node)];
    }

    auto Graph::getNodeCount() const noexcept -> int32_t {
        return num_nodes_;
    }

    auto Graph::isEmpty() const noexcept -> bool {
        return num_nodes_ == 0;
    }
}
