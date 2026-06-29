/**
 * @file Point3D.cc
 * @brief Point3D implementation â€?3D arithmetic, distance, stream output
 * @details Implements 3D point arithmetic (addition, subtraction, scalar
 *          multiplication), Euclidean and squared-distance calculations, and
 *          stream formatting.  Delegates all mathematical operations to
 *          standard <cmath> functions.
 */

#include <cppforge/data_structure/geometry/Point3D.hpp>

#include <cmath>
#include <iomanip>
#include <iostream>

namespace cppforge::data_structure::geometry
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
        return std::abs(lhs.x_ - rhs.x_) < epsilon && std::abs(lhs.y_ - rhs.y_) < epsilon && std::abs(lhs.z_ - rhs.z_) < epsilon;
    }

    bool operator!=(const Point3D& lhs, const Point3D& rhs)
    {
        return !(lhs == rhs);
    }

    std::ostream& operator<<(std::ostream& os, const Point3D& point)
    {
        const auto saved_flags = os.flags();
        const auto saved_precision = os.precision();
        os << "(" << std::fixed << std::setprecision(1) << point.x_ << ", " << point.y_ << ", " << point.z_ << ")";
        os.flags(saved_flags);
        os.precision(saved_precision);
        return os;
    }
}
