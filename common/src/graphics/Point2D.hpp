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
        [[nodiscard]] auto getX() const noexcept -> double;

        /// @brief Get the Y coordinate
        /// @return Y coordinate value
        [[nodiscard]] auto getY() const noexcept -> double;

        /// @brief Set the X coordinate
        /// @param x X coordinate value
        auto setX(double x) noexcept -> void;

        /// @brief Set the Y coordinate
        /// @param y Y coordinate value
        auto setY(double y) noexcept -> void;

        /// @brief Add another Point2D to this point
        /// @param other Point2D to add
        /// @return Reference to this point
        auto operator+=(const Point2D& other) noexcept -> Point2D&;

        /// @brief Subtract another Point2D from this point
        /// @param other Point2D to subtract
        /// @return Reference to this point
        auto operator-=(const Point2D& other) noexcept -> Point2D&;

        /// @brief Negate this point
        /// @return Negated point
        [[nodiscard]] auto operator-() const noexcept -> Point2D;

        /// @brief Calculate Euclidean distance to another point
        /// @param other Point to calculate distance to
        /// @return Euclidean distance
        [[nodiscard]] auto distanceTo(const Point2D& other) const noexcept -> double;

        /// @brief Calculate squared distance to another point (avoids sqrt)
        /// @param other Point to calculate distance to
        /// @return Squared Euclidean distance
        [[nodiscard]] auto distanceSquaredTo(const Point2D& other) const noexcept -> double;

        // Friend operators
        friend auto operator+(Point2D lhs, const Point2D& rhs) noexcept -> Point2D;

        friend auto operator-(Point2D lhs, const Point2D& rhs) noexcept -> Point2D;

        friend auto operator==(const Point2D& lhs, const Point2D& rhs) noexcept -> bool;

        friend auto operator!=(const Point2D& lhs, const Point2D& rhs) noexcept -> bool;

        friend auto operator<<(std::ostream& os, const Point2D& point) -> std::ostream&;

    private:
        double x_{0.0};
        double y_{0.0};
    };
}
