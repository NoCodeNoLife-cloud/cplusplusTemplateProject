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

    auto Edge::to() const noexcept -> int32_t
    {
        return to_;
    }

    auto Edge::setTo(const int32_t to) noexcept -> void
    {
        to_ = to;
    }

    auto Edge::weight() const noexcept -> int32_t
    {
        return weight_;
    }

    auto Edge::setWeight(const int32_t weight) noexcept -> void
    {
        weight_ = weight;
    }

    auto Edge::operator==(const Edge& other) const noexcept -> bool
    {
        return to_ == other.to_ && weight_ == other.weight_;
    }

    auto Edge::operator!=(const Edge& other) const noexcept -> bool
    {
        return !(*this == other);
    }
}
