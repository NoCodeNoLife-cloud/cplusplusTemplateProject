/**
 * @file Point2D.hpp
 * @brief Point2D class declaration
 * @details This header defines the Point2D class that provides functionality for Graphics and geometry utilities.
 */

#pragma once
#include <iostream>

namespace common::graphics
{
    /// @brief A 2D point class
    /// @details This class represents a point in 2D space with x and y coordinates.
    ///          It provides basic operations such as addition, subtraction, and comparison.
    class Point2D
    {
    public:
        Point2D() noexcept;

        Point2D(double x, double y) noexcept;

        /// @brief Get the X coordinate
        /// @return X coordinate value
        [[nodiscard]] double getX() const noexcept;

        /// @brief Get the Y coordinate
        /// @return Y coordinate value
        [[nodiscard]] double getY() const noexcept;

        /// @brief Set the X coordinate
        /// @param x X coordinate value
        void setX(double x) noexcept;

        /// @brief Set the Y coordinate
        /// @param y Y coordinate value
        void setY(double y) noexcept;

        /// @brief Add another Point2D to this point
        /// @param other Point2D to add
        /// @return Reference to this point
        Point2D& operator+=(const Point2D& other) noexcept;

        /// @brief Subtract another Point2D from this point
        /// @param other Point2D to subtract
        /// @return Reference to this point
        Point2D& operator-=(const Point2D& other) noexcept;

        /// @brief Negate this point
        /// @return Negated point
        [[nodiscard]] Point2D operator-() const noexcept;

        /// @brief Calculate Euclidean distance to another point
        /// @param other Point to calculate distance to
        /// @return Euclidean distance
        [[nodiscard]] double distanceTo(const Point2D& other) const noexcept;

        /// @brief Calculate squared distance to another point (avoids sqrt)
        /// @param other Point to calculate distance to
        /// @return Squared Euclidean distance
        [[nodiscard]] double distanceSquaredTo(const Point2D& other) const noexcept;

        // Friend operators
        friend Point2D operator+(Point2D lhs, const Point2D& rhs) noexcept;

        friend Point2D operator-(Point2D lhs, const Point2D& rhs) noexcept;

        friend bool operator==(const Point2D& lhs, const Point2D& rhs) noexcept;

        friend bool operator!=(const Point2D& lhs, const Point2D& rhs) noexcept;

        friend std::ostream& operator<<(std::ostream& os, const Point2D& point);

    private:
        double x_{0.0};
        double y_{0.0};
    };
}
