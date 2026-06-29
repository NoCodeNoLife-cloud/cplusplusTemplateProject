/**
 * @file Point3D.hpp
 * @brief 3D point with arithmetic operators, distance, and epsilon comparison
 * @details A 3D geometric point with double-precision x, y, and z coordinates.
 *          Supports vector addition/subtraction, scalar multiplication,
 *          Euclidean distance, and epsilon-aware equality comparison.
 *          Extends the 2D point concept with a third spatial dimension.
 *
 * @par Thread Safety
 * This class is const-only thread-safe (immutable after construction).
 *
 * @par Usage Example
 * @code
 * Point3D a(1.0, 2.0, 3.0);
 * Point3D b(4.0, 5.0, 6.0);
 * double dist = a.distanceTo(b);
 * Point3D sum = a + b;
 * @endcode
 */

#pragma once
#include <iosfwd>

namespace cppforge::data_structure::geometry
{
    /// @brief A 3D geometric point with double precision.
    ///
    /// @details Provides arithmetic operators, Euclidean distance, and
    ///          epsilon-aware equality comparison.  Useful for computational
    ///          geometry, graphics, and physics calculations.
    ///
    /// @par Thread Safety
    /// This class is const-only thread-safe (all members are built-in types).
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