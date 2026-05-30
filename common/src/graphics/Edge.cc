/**
* @file Edge.cc
 * @brief Edge class implementation
 * @details This file contains the implementation of the Edge class methods for Graphics and geometry utilities.
 */

#include "src/graphics/Edge.hpp"

#include <cstdint>

namespace common::graphics
{
    Edge::Edge(const int32_t to, const int32_t weight) noexcept : to_(to), weight_(weight)
    {
    }

    int32_t Edge::to() const noexcept
    {
        return to_;
    }

    void Edge::setTo(const int32_t to) noexcept
    {
        to_ = to;
    }

    int32_t Edge::weight() const noexcept
    {
        return weight_;
    }

    void Edge::setWeight(const int32_t weight) noexcept
    {
        weight_ = weight;
    }

    bool Edge::operator==(const Edge& other) const noexcept
    {
        return to_ == other.to_ && weight_ == other.weight_;
    }

    bool Edge::operator!=(const Edge& other) const noexcept
    {
        return !(*this == other);
    }
}
