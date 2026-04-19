#pragma once
#include <memory>
#include <cstdint>

namespace common::data_structure::tree {
    /// @brief A tree node class that contains a value and pointers to left and right children.
    /// @tparam T The type of the value stored in the node.
    template<typename T>
    class TreeNode {
    public:
        /// @brief Construct a TreeNode with the given value
        /// @param value The value to store in the node
        explicit TreeNode(T value) noexcept;

        T data{};
        std::shared_ptr<TreeNode> left_{};
        std::shared_ptr<TreeNode> right_{};
        int32_t height_{1};
    };

    template<typename T>
    TreeNode<T>::TreeNode(T value) noexcept : data(value) {
    }
}
