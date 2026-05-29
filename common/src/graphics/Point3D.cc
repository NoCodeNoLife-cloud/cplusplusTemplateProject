/**
 * @file Point3D.cc
 * @brief Point3D class implementation
 * @details This file contains the implementation of the Point3D class methods for Graphics and geometry utilities.
 */

#include "src/graphics/Point3D.hpp"

#include <iostream>

namespace common::graphics
{
    Point3D::Point3D() noexcept = default;

    Point3D::Point3D(const double x, const double y, const double z) noexcept : x_(x), y_(y), z_(z)
    {
    }

    auto Point3D::getX() const noexcept -> double
    {
        return x_;
    }

    auto Point3D::getY() const noexcept -> double
    {
        return y_;
    }

    auto Point3D::getZ() const noexcept -> double
    {
        return z_;
    }

    auto Point3D::setX(const double x) noexcept -> void
    {
        x_ = x;
    }

    auto Point3D::setY(const double y) noexcept -> void
    {
        y_ = y;
    }

    auto Point3D::setZ(const double z) noexcept -> void
    {
        z_ = z;
    }

    auto Point3D::operator+=(const Point3D& other) noexcept -> Point3D&
    {
        x_ += other.x_;
        y_ += other.y_;
        z_ += other.z_;
        return *this;
    }

    auto Point3D::operator-=(const Point3D& other) noexcept -> Point3D&
    {
        x_ -= other.x_;
        y_ -= other.y_;
        z_ -= other.z_;
        return *this;
    }

    auto Point3D::operator-() const noexcept -> Point3D
    {
        return {-x_, -y_, -z_};
    }

    auto Point3D::distanceTo(const Point3D& other) const noexcept -> double
    {
        return std::sqrt(distanceSquaredTo(other));
    }

    auto Point3D::distanceSquaredTo(const Point3D& other) const noexcept -> double
    {
        const double dx = x_ - other.x_;
        const double dy = y_ - other.y_;
        const double dz = z_ - other.z_;
        return dx * dx + dy * dy + dz * dz;
    }

    auto operator+(Point3D lhs, const Point3D& rhs) noexcept -> Point3D
    {
        lhs += rhs;
        return lhs;
    }

    auto operator-(Point3D lhs, const Point3D& rhs) noexcept -> Point3D
    {
        lhs -= rhs;
        return lhs;
    }

    auto operator==(const Point3D& lhs, const Point3D& rhs) noexcept -> bool
    {
        constexpr double epsilon = 1e-9;
        return std::abs(lhs.getX() - rhs.getX()) < epsilon && std::abs(lhs.getY() - rhs.getY()) < epsilon && std::abs(lhs.getZ() - rhs.getZ()) < epsilon;
    }

    auto operator!=(const Point3D& lhs, const Point3D& rhs) noexcept -> bool
    {
        return !(lhs == rhs);
    }

    auto operator<<(std::ostream& os, const Point3D& point) -> std::ostream&
    {
        os << "(" << point.getX() << ", " << point.getY() << ", " << point.getZ() << ")";
        return os;
    }
}
