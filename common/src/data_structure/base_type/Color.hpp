/**
 * @file Color.hpp
 * @brief RGBA colour representation with enum and utility functions
 * @details Defines a set of named colours via enum class (Color::Red, etc.)
 *          and provides conversion utilities between enum values and
 *          RGBA byte quadruplets.  Useful for graphics, console output
 *          colourisation, and UI components.
 *
 * @par Thread Safety
 * This enum is const-only thread-safe (immutable).
 */

#pragma once

namespace cppforge::data_structure
{
    /// @brief Enumeration representing various colors.
    /// Each color is assigned a unique integer value.
    enum class Color : int
    {
        /// @brief Red color (value 0)
        Red = 0,
        /// @brief Black color (value 1)
        Black = 1,
        /// @brief Green color (value 2)
        Green = 2,
        /// @brief Blue color (value 3)
        Blue = 3,
        /// @brief White color (value 4)
        White = 4,
        /// @brief Yellow color (value 5)
        Yellow = 5,
        /// @brief Cyan color (value 6)
        Cyan = 6,
        /// @brief Magenta color (value 7)
        Magenta = 7,
    };
}
