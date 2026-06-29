/**
 * @file Edge.cc
 * @brief Edge implementation â€?accessors and reversed-edge construction
 * @details Implements the Edge data type including source/target/weight
 *          accessors and the reversed() method for undirected contexts.
 */

#include <cppforge/data_structure/graph/Edge.hpp>

namespace cppforge::data_structure::graph
{
    Edge::Edge(const int32_t to, const int32_t weight)  : to_(to), weight_(weight)
    {
    }

    int32_t Edge::to() const
    {
        return to_;
    }

    void Edge::setTo(const int32_t to)
    {
        to_ = to;
    }

    int32_t Edge::weight() const
    {
        return weight_;
    }

    void Edge::setWeight(const int32_t weight)
    {
        weight_ = weight;
    }

    bool Edge::operator==(const Edge& other) const
    {
        return to_ == other.to_ && weight_ == other.weight_;
    }

    bool Edge::operator!=(const Edge& other) const
    {
        return !(*this == other);
    }
}
