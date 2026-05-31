/**
* @file Edge.hpp
 * @brief Edge class declaration
 * @details This header defines the Edge class that provides functionality for Graphics and geometry utilities.
 */

#pragma once
#include <cstdint>

namespace common::graphics
{
    /// @brief Data structure for representing an edge in a graph
    /// @details This class stores information about an edge, including
    ///          the target vertex and the weight of the edge. It provides
    ///          methods to get and set these values.
    class Edge
    {
    public:
        /// @brief Construct an Edge with target vertex and weight
        /// @param to Target vertex
        /// @param weight Edge weight
        explicit Edge(int32_t to, int32_t weight) ;

        /// @brief Get target vertex
        /// @return Target vertex
        [[nodiscard]] int32_t to() const ;

        /// @brief Set target vertex
        /// @param to Target vertex
        void setTo(int32_t to) ;

        /// @brief Get edge weight
        /// @return Edge weight
        [[nodiscard]] int32_t weight() const ;

        /// @brief Set edge weight
        /// @param weight Edge weight
        void setWeight(int32_t weight) ;

        /// @brief Equality operator
        /// @param other Edge to compare with
        /// @return True if edges are equal
        [[nodiscard]] bool operator==(const Edge& other) const ;

        /// @brief Inequality operator
        /// @param other Edge to compare with
        /// @return True if edges are not equal
        [[nodiscard]] bool operator!=(const Edge& other) const ;

    private:
        int32_t to_{0};
        int32_t weight_{0};
    };
};