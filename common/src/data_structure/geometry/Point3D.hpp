/**
 * @file Point3D.hpp
 * @brief Point3D class declaration
 * @details This header defines the Point3D class that provides functionality for geometry utilities.
 */

#pragma once
#include <iosfwd>

namespace common::data_structure::geometry
{
    /// @brief A 3D point class
    /// @details This class represents a point in 3D space with x, y and z coordinates.
    ///          It provides basic operations such as addition, subtraction, and comparison.
    class Point3D
    {
    public:
        Point3D() ;

        Point3D(double x, double y, double z) ;

        /// @brief Get the X coordinate
        /// @return X coordinate value
        [[nodiscard]] double getX() const ;

        /// @brief Get the Y coordinate
        /// @return Y coordinate value
        [[nodiscard]] double getY() const ;

        /// @brief Get the Z coordinate
        /// @return Z coordinate value
        [[nodiscard]] double getZ() const ;

        /// @brief Set the X coordinate
        /// @param x X coordinate value
        void setX(double x) ;

        /// @brief Set the Y coordinate
        /// @param y Y coordinate value
        void setY(double y) ;

        /// @brief Set the Z coordinate
        /// @param z Z coordinate value
        void setZ(double z) ;

        /// @brief Add another Point3D to this point
        /// @param other Point3D to add
        /// @return Reference to this point
        Point3D& operator+=(const Point3D& other) ;

        /// @brief Subtract another Point3D from this point
        /// @param other Point3D to subtract
        /// @return Reference to this point
        Point3D& operator-=(const Point3D& other) ;

        /// @brief Negate this point
        /// @return Negated point
        [[nodiscard]] Point3D operator-() const ;

        /// @brief Calculate Euclidean distance to another point
        /// @param other Point to calculate distance to
        /// @return Euclidean distance
        [[nodiscard]] double distanceTo(const Point3D& other) const ;

        /// @brief Calculate squared distance to another point (avoids sqrt)
        /// @param other Point to calculate distance to
        /// @return Squared Euclidean distance
        [[nodiscard]] double distanceSquaredTo(const Point3D& other) const ;

        // Friend operators
        friend Point3D operator+(Point3D lhs, const Point3D& rhs) ;

        friend Point3D operator-(Point3D lhs, const Point3D& rhs) ;

        friend bool operator==(const Point3D& lhs, const Point3D& rhs) ;

        friend bool operator!=(const Point3D& lhs, const Point3D& rhs) ;

        friend std::ostream& operator<<(std::ostream& os, const Point3D& point);

    private:
        double x_{0.0};
        double y_{0.0};
        double z_{0.0};
    };
}