/**
 * @file TreeToolkit.hpp
 * @brief Utility functions for tree traversal, printing, and validation
 * @details Provides static utility methods for common tree operations:
 *          - Tree traversal (preorder, inorder, postorder, level-order)
 *          - Pretty-printing of tree structures
 *          - Validation of BST/Avl/RB tree invariants
 *          These utilities work with any node type following a standard
 *          left/right child pointer convention.
 *
 * @par Thread Safety
 * Static utility methods do not modify tree state and are safe to call
 * concurrently on different tree instances.
 */

#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace cppforge::data_structure::tree::core
{
    /// @brief Utility class for generic tree operations.
    /// @details Provides static methods for common recursive tree computations
    ///          (e.g., height and size) that work with any node type exposing
    ///          left_ and right_ members.
    class TreeToolkit
    {
    public:
        TreeToolkit() = delete;

        /// @brief Compute the height of a tree (edges on longest root-to-leaf path)
        /// @tparam Node The node type (must have .left_ and .right_ unique_ptr-like members)
        /// @param node Pointer to the root node, may be null
        /// @return Height in edges (-1 for null, 0 for leaf)
        template <typename Node>
        [[nodiscard]] static int32_t height(const Node* node)
        {
            if (!node) return -1;
            return 1 + std::max(height(node->left_.get()), height(node->right_.get()));
        }

        /// @brief Count the number of nodes in a tree
        /// @tparam Node The node type (must have .left_ and .right_ unique_ptr-like members)
        /// @param node Pointer to the root node, may be null
        /// @return Number of nodes (0 for null)
        template <typename Node>
        [[nodiscard]] static size_t size(const Node* node)
        {
            if (!node) return 0;
            return 1 + size(node->left_.get()) + size(node->right_.get());
        }
    };
}
