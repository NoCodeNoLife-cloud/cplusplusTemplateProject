/**
 * @file TreeNode.hpp
 * @brief TreeNode class declaration
 * @details This header defines the TreeNode class that provides functionality for Advanced data structures including trees and skip lists.
 */

#pragma once
#include <cstdint>
#include <memory>
#include <utility>

namespace common::data_structure::tree
{
    /// @brief A tree node class that contains a value and pointers to left and right children.
    /// @tparam T The type of the value stored in the node.
    template <typename T>
    class TreeNode
    {
    public:
        /// @brief Construct a TreeNode with the given value
        /// @param value The value to store in the node
        explicit TreeNode(T value) ;

        T data{};
        std::unique_ptr<TreeNode> left_{};
        std::unique_ptr<TreeNode> right_{};
        int32_t height_{1};
    };

    template <typename T>
    TreeNode<T>::TreeNode(T value)  : data(std::move(value))
    {
    }
}