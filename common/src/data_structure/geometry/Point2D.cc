/**
 * @file Point2D.cc
 * @brief Point2D class implementation
 * @details This file contains the implementation of the Point2D class methods for geometry utilities.
 */

#include "data_structure/geometry/Point2D.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>

namespace common::data_structure::geometry
{
    Point2D::Point2D()  = default;

    Point2D::Point2D(const double x, const double y)  : x_(x), y_(y)
    {
    }

    double Point2D::getX() const
    {
        return x_;
    }

    double Point2D::getY() const
    {
        return y_;
    }

    void Point2D::setX(const double x)
    {
        x_ = x;
    }

    void Point2D::setY(const double y)
    {
        y_ = y;
    }

    Point2D& Point2D::operator+=(const Point2D& other)
    {
        x_ += other.x_;
        y_ += other.y_;
        return *this;
    }

    Point2D& Point2D::operator-=(const Point2D& other)
    {
        x_ -= other.x_;
        y_ -= other.y_;
        return *this;
    }

    Point2D Point2D::operator-() const
    {
        return {-x_, -y_};
    }

    double Point2D::distanceTo(const Point2D& other) const
    {
        return std::sqrt(distanceSquaredTo(other));
    }

    double Point2D::distanceSquaredTo(const Point2D& other) const
    {
        const double dx = x_ - other.x_;
        const double dy = y_ - other.y_;
        return dx * dx + dy * dy;
    }

    Point2D operator+(Point2D lhs, const Point2D& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    Point2D operator-(Point2D lhs, const Point2D& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    bool operator==(const Point2D& lhs, const Point2D& rhs)
    {
        constexpr double epsilon = 1e-9;
        return std::abs(lhs.x_ - rhs.x_) < epsilon && std::abs(lhs.y_ - rhs.y_) < epsilon;
    }

    bool operator!=(const Point2D& lhs, const Point2D& rhs)
    {
        return !(lhs == rhs);
    }

    std::ostream& operator<<(std::ostream& os, const Point2D& point)
    {
        const auto saved_flags = os.flags();
        const auto saved_precision = os.precision();
        os << "(" << std::fixed << std::setprecision(1) << point.x_ << ", " << point.y_ << ")";
        os.flags(saved_flags);
        os.precision(saved_precision);
        return os;
    }
}
