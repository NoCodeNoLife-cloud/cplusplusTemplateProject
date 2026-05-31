/**
 * @file Point3D.hpp
 * @brief Point3D class declaration
 * @details This header defines the Point3D class that provides functionality for Graphics and geometry utilities.
 */

#pragma once
#include <iostream>

namespace common::graphics
{
    /// @brief A 3D point class
    /// @details This class represents a point in 3D space with x, y and z coordinates.
    ///          It provides basic operations such as addition, subtraction, and comparison.
    class Point3D
    {
    public:
        Point3D() noexcept;

        Point3D(double x, double y, double z) noexcept;

        /// @brief Get the X coordinate
        /// @return X coordinate value
        [[nodiscard]] double getX() const noexcept;

        /// @brief Get the Y coordinate
        /// @return Y coordinate value
        [[nodiscard]] double getY() const noexcept;

        /// @brief Get the Z coordinate
        /// @return Z coordinate value
        [[nodiscard]] double getZ() const noexcept;

        /// @brief Set the X coordinate
        /// @param x X coordinate value
        void setX(double x) noexcept;

        /// @brief Set the Y coordinate
        /// @param y Y coordinate value
        void setY(double y) noexcept;

        /// @brief Set the Z coordinate
        /// @param z Z coordinate value
        void setZ(double z) noexcept;

        /// @brief Add another Point3D to this point
        /// @param other Point3D to add
        /// @return Reference to this point
        Point3D& operator+=(const Point3D& other) noexcept;

        /// @brief Subtract another Point3D from this point
        /// @param other Point3D to subtract
        /// @return Reference to this point
        Point3D& operator-=(const Point3D& other) noexcept;

        /// @brief Negate this point
        /// @return Negated point
        [[nodiscard]] Point3D operator-() const noexcept;

        /// @brief Calculate Euclidean distance to another point
        /// @param other Point to calculate distance to
        /// @return Euclidean distance
        [[nodiscard]] double distanceTo(const Point3D& other) const noexcept;

        /// @brief Calculate squared distance to another point (avoids sqrt)
        /// @param other Point to calculate distance to
        /// @return Squared Euclidean distance
        [[nodiscard]] double distanceSquaredTo(const Point3D& other) const noexcept;

        // Friend operators
        friend Point3D operator+(Point3D lhs, const Point3D& rhs) noexcept;

        friend Point3D operator-(Point3D lhs, const Point3D& rhs) noexcept;

        friend bool operator==(const Point3D& lhs, const Point3D& rhs) noexcept;

        friend bool operator!=(const Point3D& lhs, const Point3D& rhs) noexcept;

        friend std::ostream& operator<<(std::ostream& os, const Point3D& point);

    private:
        double x_{0.0};
        double y_{0.0};
        double z_{0.0};
    };
}