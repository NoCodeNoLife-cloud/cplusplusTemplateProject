/**
 * @file Point3D.cc
 * @brief Point3D class implementation
 * @details This file contains the implementation of the Point3D class methods for Graphics and geometry utilities.
 */

#include "graphics/Point3D.hpp"

#include <iostream>

namespace common::graphics
{
    Point3D::Point3D()  = default;

    Point3D::Point3D(const double x, const double y, const double z)  : x_(x), y_(y), z_(z)
    {
    }

    double Point3D::getX() const
    {
        return x_;
    }

    double Point3D::getY() const
    {
        return y_;
    }

    double Point3D::getZ() const
    {
        return z_;
    }

    void Point3D::setX(const double x)
    {
        x_ = x;
    }

    void Point3D::setY(const double y)
    {
        y_ = y;
    }

    void Point3D::setZ(const double z)
    {
        z_ = z;
    }

    Point3D& Point3D::operator+=(const Point3D& other)
    {
        x_ += other.x_;
        y_ += other.y_;
        z_ += other.z_;
        return *this;
    }

    Point3D& Point3D::operator-=(const Point3D& other)
    {
        x_ -= other.x_;
        y_ -= other.y_;
        z_ -= other.z_;
        return *this;
    }

    Point3D Point3D::operator-() const
    {
        return {-x_, -y_, -z_};
    }

    double Point3D::distanceTo(const Point3D& other) const
    {
        return std::sqrt(distanceSquaredTo(other));
    }

    double Point3D::distanceSquaredTo(const Point3D& other) const
    {
        const double dx = x_ - other.x_;
        const double dy = y_ - other.y_;
        const double dz = z_ - other.z_;
        return dx * dx + dy * dy + dz * dz;
    }

    Point3D operator+(Point3D lhs, const Point3D& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    Point3D operator-(Point3D lhs, const Point3D& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    bool operator==(const Point3D& lhs, const Point3D& rhs)
    {
        constexpr double epsilon = 1e-9;
        return std::abs(lhs.getX() - rhs.getX()) < epsilon && std::abs(lhs.getY() - rhs.getY()) < epsilon && std::abs(lhs.getZ() - rhs.getZ()) < epsilon;
    }

    bool operator!=(const Point3D& lhs, const Point3D& rhs)
    {
        return !(lhs == rhs);
    }

    std::ostream& operator<<(std::ostream& os, const Point3D& point)
    {
        os << "(" << point.getX() << ", " << point.getY() << ", " << point.getZ() << ")";
        return os;
    }
}
