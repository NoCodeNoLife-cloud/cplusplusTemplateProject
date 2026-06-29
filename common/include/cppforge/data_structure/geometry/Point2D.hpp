/**
 * @file Point2D.hpp
 * @brief 2D point with arithmetic operators, distance, and epsilon comparison
 * @details A 2D geometric point with double-precision x and y coordinates.
 *          Supports vector addition/subtraction, scalar multiplication,
 *          Euclidean distance, and equality comparison within an epsilon
 *          tolerance (default 1e-9) for floating-point safety.
 *
 * @par Thread Safety
 * This class is const-only thread-safe (immutable after construction).
 *
 * @par Usage Example
 * @code
 * Point2D a(1.0, 2.0);
 * Point2D b(4.0, 6.0);
 * double dist = a.distanceTo(b);  // 5.0
 * Point2D sum = a + b;
 * bool eq = a.equalWithEpsilon(a); // true
 * @endcode
 */

#pragma once
#include <iosfwd>

namespace cppforge::data_structure::geometry
{
    /// @brief A 2D geometric point with double precision.
    ///
    /// @details Provides arithmetic operators, Euclidean distance, and
    ///          epsilon-aware equality comparison.  Useful for computational
    ///          geometry, graphics, and physics calculations.
    ///
    /// @par Thread Safety
    /// This class is const-only thread-safe (all members are built-in types).
    class Point2D
    {
    public:
        Point2D() ;

        Point2D(double x, double y) ;

        /// @brief Get the X coordinate
        /// @return X coordinate value
        [[nodiscard]] double getX() const ;

        /// @brief Get the Y coordinate
        /// @return Y coordinate value
        [[nodiscard]] double getY() const ;

        /// @brief Set the X coordinate
        /// @param x X coordinate value
        void setX(double x) ;

        /// @brief Set the Y coordinate
        /// @param y Y coordinate value
        void setY(double y) ;

        /// @brief Add another Point2D to this point
        /// @param other Point2D to add
        /// @return Reference to this point
        Point2D& operator+=(const Point2D& other) ;

        /// @brief Subtract another Point2D from this point
        /// @param other Point2D to subtract
        /// @return Reference to this point
        Point2D& operator-=(const Point2D& other) ;

        /// @brief Negate this point
        /// @return Negated point
        [[nodiscard]] Point2D operator-() const ;

        /// @brief Calculate Euclidean distance to another point
        /// @param other Point to calculate distance to
        /// @return Euclidean distance
        [[nodiscard]] double distanceTo(const Point2D& other) const ;

        /// @brief Calculate squared distance to another point (avoids sqrt)
        /// @param other Point to calculate distance to
        /// @return Squared Euclidean distance
        [[nodiscard]] double distanceSquaredTo(const Point2D& other) const ;

        // Friend operators
        friend Point2D operator+(Point2D lhs, const Point2D& rhs) ;

        friend Point2D operator-(Point2D lhs, const Point2D& rhs) ;

        friend bool operator==(const Point2D& lhs, const Point2D& rhs) ;

        friend bool operator!=(const Point2D& lhs, const Point2D& rhs) ;

        friend std::ostream& operator<<(std::ostream& os, const Point2D& point);

    private:
        double x_{0.0};
        double y_{0.0};
    };
}