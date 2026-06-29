/**
 * @file RedBlackTreeNode.hpp
 * @brief Red-Black tree node with colour and sentinel-null support
 * @details A specialised tree node for Red-Black tree implementations.
 *          Extends the base TreeNode concept with a colour flag (red/black)
 *          and support for a sentinel NIL leaf node to simplify edge-case
 *          handling during insertion and deletion fix-ups.
 *
 * @par Reference
 * Guibas & Sedgewick, "A Dichromatic Framework for Balanced Trees" (1978).
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 */

#pragma once
#include <memory>

#include <cppforge/data_structure/base_type/Color.hpp>

namespace cppforge::data_structure::tree::node
{
    /// @brief A node in a Red-Black Tree
    /// @tparam T The type of data stored in the node
    template <typename T>
    class RedBlackTreeNode
    {
    public:
        explicit RedBlackTreeNode(const T& value) ;

        [[nodiscard]] T getData() const ;
        void setData(const T& value) ;
        [[nodiscard]] std::shared_ptr<RedBlackTreeNode> getLeft() const ;
        void setLeft(std::shared_ptr<RedBlackTreeNode> node) ;
        [[nodiscard]] std::shared_ptr<RedBlackTreeNode> getRight() const ;
        void setRight(std::shared_ptr<RedBlackTreeNode> node) ;
        [[nodiscard]] std::shared_ptr<RedBlackTreeNode> getParent() const ;
        void setParent(std::shared_ptr<RedBlackTreeNode> node) ;
        [[nodiscard]] Color getColor() const ;
        void setColor(Color c) ;

    private:
        T data_{};
        std::shared_ptr<RedBlackTreeNode> left_{};
        std::shared_ptr<RedBlackTreeNode> right_{};
        std::shared_ptr<RedBlackTreeNode> parent_{};
        Color color_{Color::Red};
    };

    template <typename T>
    RedBlackTreeNode<T>::RedBlackTreeNode(const T& value)  : data_(value)
    {
    }

    template <typename T>
    T RedBlackTreeNode<T>::getData() const { return data_; }

    template <typename T>
    void RedBlackTreeNode<T>::setData(const T& value) { data_ = value; }

    template <typename T>
    std::shared_ptr<RedBlackTreeNode<T>> RedBlackTreeNode<T>::getLeft() const { return left_; }

    template <typename T>
    void RedBlackTreeNode<T>::setLeft(std::shared_ptr<RedBlackTreeNode<T>> node) { left_ = node; }

    template <typename T>
    std::shared_ptr<RedBlackTreeNode<T>> RedBlackTreeNode<T>::getRight() const { return right_; }

    template <typename T>
    void RedBlackTreeNode<T>::setRight(std::shared_ptr<RedBlackTreeNode<T>> node) { right_ = node; }

    template <typename T>
    std::shared_ptr<RedBlackTreeNode<T>> RedBlackTreeNode<T>::getParent() const { return parent_; }

    template <typename T>
    void RedBlackTreeNode<T>::setParent(std::shared_ptr<RedBlackTreeNode<T>> node) { parent_ = node; }

    template <typename T>
    Color RedBlackTreeNode<T>::getColor() const { return color_; }

    template <typename T>
    void RedBlackTreeNode<T>::setColor(const Color c) { color_ = c; }
}
