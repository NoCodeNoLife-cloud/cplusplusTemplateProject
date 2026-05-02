#include "src/graphics/Graph.hpp"


#include <fmt/format.h>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <string>

#include "src/graphics/Edge.hpp"

namespace common::graphics {
    Graph::Graph(const int32_t n) : num_nodes_(n) {
        if (n < 0) {
            throw std::invalid_argument("Graph::Graph: Number of nodes cannot be negative");
        }
        adj_list_.resize(static_cast<size_t>(num_nodes_));
    }

    auto Graph::addEdge(const int32_t from, const int32_t to, const int32_t weight) -> void {
        if (from < 0 || from >= num_nodes_ || to < 0 || to >= num_nodes_) {
            throw std::out_of_range("Graph::addEdge: Node index out of range. Requested: from=" + std::to_string(from) + ", to=" + std::to_string(to) + ", but graph has " + std::to_string(num_nodes_) + " nodes");
        }
        Edge edge(to, weight);
        adj_list_[static_cast<size_t>(from)].emplace_back(edge);
    }

    auto Graph::getAdjList(const int32_t node) const -> const std::vector<Edge> & {
        if (node < 0 || node >= num_nodes_) {
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
