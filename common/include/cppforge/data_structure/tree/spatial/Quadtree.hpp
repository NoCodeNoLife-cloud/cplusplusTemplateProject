/**
 * @file Quadtree.hpp
 * @brief Spatial Quadtree for 2D point data with recursive subdivision
 * @details A tree data structure that recursively subdivides a 2D bounding box
 *          into four quadrants.  Each node stores points within its region;
 *          when a node exceeds capacity, it splits into four children.
 *          Commonly used for spatial indexing, collision detection, and
 *          image compression.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Complexity
 * - insert / query:              O(log n) average
 * - range search:                O(log n + k) where k = result count
 */

#pragma once
#include <cmath>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace cppforge::data_structure::tree::spatial
{
    /// @brief A 2D point with x and y coordinates
    /// Used as the spatial key for Quadtree operations.
    struct Point
    {
        double x_{};
        double y_{};

        Point() = default;

        /// @brief Construct a point with the given coordinates
        /// @param x The x-coordinate
        /// @param y The y-coordinate
        Point(const double x, const double y)
            : x_(x), y_(y)
        {
        }
    };

    /// @brief A quadtree data structure for spatial partitioning
    /// A quadtree recursively subdivides a 2D space into four quadrants (NW, NE, SW, SE).
    /// Each leaf node stores at most one point-value pair. When a second point falls into
    /// the same leaf, the leaf subdivides into four children and redistributes the points.
    /// @tparam T The type of value associated with each point
    template <typename T>
    class Quadtree
    {
    public:
        /// @brief Construct a quadtree with a given square boundary
        /// @param center_x The x-coordinate of the center of the boundary
        /// @param center_y The y-coordinate of the center of the boundary
        /// @param half_dim Half of the side length of the square boundary
        Quadtree(double center_x, double center_y, double half_dim);

        /// @brief Insert a point with an associated value into the quadtree
        /// @param point The point to insert
        /// @param value The value to associate with the point
        /// @throws std::runtime_error If the point lies outside the quadtree boundary
        void insert(const Point& point, const T& value);

        /// @brief Find all points within a rectangular region
        /// @param center The center of the query region
        /// @param half_dim Half of the side length of the query region
        /// @return A vector of (point, value) pairs within the query region
        [[nodiscard]] std::vector<std::pair<Point, T>> queryRange(const Point& center, double half_dim) const;

        /// @brief Check if the quadtree contains no points
        /// @return True if the quadtree is empty, false otherwise
        [[nodiscard]] bool empty() const noexcept;

        /// @brief Get the number of points stored in the quadtree
        /// @return The number of points
        [[nodiscard]] size_t size() const noexcept;

        /// @brief Remove all points from the quadtree
        void clear() noexcept;

    private:
        /// @brief Internal node structure for the quadtree
        struct Node
        {
            T data_{};
            double x_{};
            double y_{};
            double center_x_{};
            double center_y_{};
            double half_dim_{};
            bool is_leaf_{true};
            bool occupied_{false};
            std::unique_ptr<Node> northwest_{};
            std::unique_ptr<Node> northeast_{};
            std::unique_ptr<Node> southwest_{};
            std::unique_ptr<Node> southeast_{};

            /// @brief Construct a node with a given boundary
            /// @param cx Center x-coordinate of the boundary
            /// @param cy Center y-coordinate of the boundary
            /// @param hd Half-dimension of the boundary
            Node(double cx, double cy, double hd)
                : center_x_(cx), center_y_(cy), half_dim_(hd)
            {
            }
        };

        std::unique_ptr<Node> root_{};
        size_t quadtree_size_{0};

        /// @brief Check if a point is within a node's boundary
        /// @param node The node whose boundary to check against
        /// @param point The point to test
        /// @return True if the point is within the boundary
        [[nodiscard]] static bool contains(const Node& node, const Point& point);

        /// @brief Check if two axis-aligned square regions intersect
        /// @param cx1 Center x of the first region
        /// @param cy1 Center y of the first region
        /// @param hd1 Half-dimension of the first region
        /// @param cx2 Center x of the second region
        /// @param cy2 Center y of the second region
        /// @param hd2 Half-dimension of the second region
        /// @return True if the two regions intersect
        [[nodiscard]] static bool intersects(double cx1, double cy1, double hd1,
                                             double cx2, double cy2, double hd2);

        /// @brief Create a child node for a specific quadrant relative to a parent
        /// @param parent The parent node
        /// @param left True if the child is in the left half of the parent
        /// @param top True if the child is in the top half of the parent
        /// @return A new child node
        [[nodiscard]] static std::unique_ptr<Node> makeChild(const Node& parent, bool left, bool top);

        /// @brief Subdivide a leaf node into four child nodes
        /// @param node The leaf node to subdivide
        void subdivide(Node& node);

        /// @brief Insert a point into a subtree rooted at the given node
        /// @param node The root of the subtree
        /// @param point The point to insert
        /// @param value The value to associate with the point
        void insert(Node& node, const Point& point, const T& value);

        /// @brief Collect points within a region from a subtree
        /// @param node The root of the subtree to search
        /// @param center The center of the query region
        /// @param half_dim Half-dimension of the query region
        /// @param result The vector to append results to
        void queryRange(const Node& node, const Point& center, double half_dim,
                        std::vector<std::pair<Point, T>>& result) const;
    };

    template <typename T>
    Quadtree<T>::Quadtree(const double center_x, const double center_y, const double half_dim)
        : root_(std::make_unique<Node>(center_x, center_y, half_dim))
    {
    }

    template <typename T>
    void Quadtree<T>::insert(const Point& point, const T& value)
    {
        insert(*root_, point, value);
    }

    template <typename T>
    std::vector<std::pair<Point, T>> Quadtree<T>::queryRange(const Point& center, const double half_dim) const
    {
        std::vector<std::pair<Point, T>> result;
        queryRange(*root_, center, half_dim, result);
        return result;
    }

    template <typename T>
    bool Quadtree<T>::empty() const noexcept
    {
        return quadtree_size_ == 0;
    }

    template <typename T>
    size_t Quadtree<T>::size() const noexcept
    {
        return quadtree_size_;
    }

    template <typename T>
    void Quadtree<T>::clear() noexcept
    {
        root_->northwest_.reset();
        root_->northeast_.reset();
        root_->southwest_.reset();
        root_->southeast_.reset();
        root_->is_leaf_ = true;
        root_->occupied_ = false;
        quadtree_size_ = 0;
    }

    template <typename T>
    bool Quadtree<T>::contains(const Node& node, const Point& point)
    {
        return std::abs(point.x_ - node.center_x_) <= node.half_dim_ &&
            std::abs(point.y_ - node.center_y_) <= node.half_dim_;
    }

    template <typename T>
    bool Quadtree<T>::intersects(const double cx1, const double cy1, const double hd1,
                                 const double cx2, const double cy2, const double hd2)
    {
        return std::abs(cx1 - cx2) <= hd1 + hd2 &&
            std::abs(cy1 - cy2) <= hd1 + hd2;
    }

    template <typename T>
    std::unique_ptr<typename Quadtree<T>::Node> Quadtree<T>::makeChild(const Node& parent, const bool left, const bool top)
    {
        const double child_cx = parent.center_x_ + (left ? -1.0 : 1.0) * parent.half_dim_ / 2.0;
        const double child_cy = parent.center_y_ + (top ? 1.0 : -1.0) * parent.half_dim_ / 2.0;
        return std::make_unique<Node>(child_cx, child_cy, parent.half_dim_ / 2.0);
    }

    template <typename T>
    void Quadtree<T>::subdivide(Node& node)
    {
        node.northwest_ = Quadtree<T>::makeChild(node, true, true);
        node.northeast_ = Quadtree<T>::makeChild(node, false, true);
        node.southwest_ = Quadtree<T>::makeChild(node, true, false);
        node.southeast_ = Quadtree<T>::makeChild(node, false, false);
        node.is_leaf_ = false;
    }

    template <typename T>
    void Quadtree<T>::insert(Node& node, const Point& point, const T& value)
    {
        if (!Quadtree<T>::contains(node, point))
        {
            throw std::runtime_error("Point outside quadtree boundary");
        }

        if (node.is_leaf_)
        {
            if (!node.occupied_)
            {
                node.x_ = point.x_;
                node.y_ = point.y_;
                node.data_ = value;
                node.occupied_ = true;
                ++quadtree_size_;
                return;
            }

            // Subdivide and reinsert both the existing and new points
            const Point old_point(node.x_, node.y_);
            T old_value = std::move(node.data_);
            node.occupied_ = false;
            --quadtree_size_;
            subdivide(node);
            insert(node, old_point, old_value);
            insert(node, point, value);
            return;
        }

        // Internal node -- route to the correct child
        const bool left = point.x_ <= node.center_x_;
        const bool top = point.y_ >= node.center_y_;

        if (left && top)
        {
            if (!node.northwest_) node.northwest_ = Quadtree<T>::makeChild(node, true, true);
            insert(*node.northwest_, point, value);
        }
        else if (!left && top)
        {
            if (!node.northeast_) node.northeast_ = Quadtree<T>::makeChild(node, false, true);
            insert(*node.northeast_, point, value);
        }
        else if (left)
        {
            if (!node.southwest_) node.southwest_ = Quadtree<T>::makeChild(node, true, false);
            insert(*node.southwest_, point, value);
        }
        else
        {
            if (!node.southeast_) node.southeast_ = Quadtree<T>::makeChild(node, false, false);
            insert(*node.southeast_, point, value);
        }
    }

    template <typename T>
    void Quadtree<T>::queryRange(const Node& node, const Point& center, const double half_dim,
                                 std::vector<std::pair<Point, T>>& result) const
    {
        if (!Quadtree<T>::intersects(node.center_x_, node.center_y_, node.half_dim_,
                        center.x_, center.y_, half_dim))
        {
            return;
        }

        if (node.is_leaf_)
        {
            if (node.occupied_ &&
                std::abs(node.x_ - center.x_) <= half_dim &&
                std::abs(node.y_ - center.y_) <= half_dim)
            {
                result.emplace_back(Point(node.x_, node.y_), node.data_);
            }
            return;
        }

        if (node.northwest_) queryRange(*node.northwest_, center, half_dim, result);
        if (node.northeast_) queryRange(*node.northeast_, center, half_dim, result);
        if (node.southwest_) queryRange(*node.southwest_, center, half_dim, result);
        if (node.southeast_) queryRange(*node.southeast_, center, half_dim, result);
    }
}
