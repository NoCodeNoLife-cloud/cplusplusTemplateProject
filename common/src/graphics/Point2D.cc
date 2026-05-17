/**
 * @file Point2D.cc
 * @brief Point2D class implementation
 * @details This file contains the implementation of the Point2D class methods for Graphics and geometry utilities.
 */

#include "src/graphics/Point2D.hpp"

#include <iostream>

namespace common::graphics {
Point2D::Point2D() noexcept = default;

Point2D::Point2D(const double x, const double y) noexcept : x_(x), y_(y) {
}

auto Point2D::getX() const noexcept -> double {
    return x_;
}

auto Point2D::getY() const noexcept -> double {
    return y_;
}

auto Point2D::setX(const double x) noexcept -> void {
    x_ = x;
}

auto Point2D::setY(const double y) noexcept -> void {
    y_ = y;
}

auto Point2D::operator+=(const Point2D& other) noexcept -> Point2D& {
    x_ += other.x_;
    y_ += other.y_;
    return *this;
}

auto Point2D::operator-=(const Point2D& other) noexcept -> Point2D& {
    x_ -= other.x_;
    y_ -= other.y_;
    return *this;
}

auto Point2D::operator-() const noexcept -> Point2D {
    return {-x_, -y_};
}

auto Point2D::distanceTo(const Point2D& other) const noexcept -> double {
    return std::sqrt(distanceSquaredTo(other));
}

auto Point2D::distanceSquaredTo(const Point2D& other) const noexcept -> double {
    const double dx = x_ - other.x_;
    const double dy = y_ - other.y_;
    return dx * dx + dy * dy;
}

auto operator+(Point2D lhs, const Point2D& rhs) noexcept -> Point2D {
    lhs += rhs;
    return lhs;
}

auto operator-(Point2D lhs, const Point2D& rhs) noexcept -> Point2D {
    lhs -= rhs;
    return lhs;
}

auto operator==(const Point2D& lhs, const Point2D& rhs) noexcept -> bool {
    constexpr double epsilon = 1e-9;
    return std::abs(lhs.getX() - rhs.getX()) < epsilon && std::abs(lhs.getY() - rhs.getY()) < epsilon;
}

auto operator!=(const Point2D& lhs, const Point2D& rhs) noexcept -> bool {
    return !(lhs == rhs);
}

auto operator<<(std::ostream& os, const Point2D& point) -> std::ostream& {
    os << "(" << point.getX() << ", " << point.getY() << ")";
    return os;
}
}