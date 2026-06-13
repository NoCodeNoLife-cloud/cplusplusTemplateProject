/**
 * @file IHashRing.hpp
 * @brief Abstract interface for consistent hash ring
 * @details This header defines the IHashRing interface that provides a common
 *          contract for consistent hash ring implementations. A consistent hash
 *          ring maps keys to nodes in a way that minimises re-mapping when nodes
 *          are added or removed, making it ideal for distributed caching, load
 *          balancing, and sharding.
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace common::data_structure::hash
{
    /// @brief Node identifier for physical servers / cache instances.
    ///        Users should provide a globally unique name per physical node,
    ///        e.g. "cache-01", "10.0.0.1:6379".
    using NodeName = std::string;

    /// @brief Abstract interface for a consistent hash ring.
    ///
    /// A consistent hash organises the hash space as a circular ring.
    /// Each physical node occupies one or more positions on the ring
    /// (virtual nodes).  When a key is looked up, its hash determines
    /// a position on the ring and the closest clockwise virtual node
    /// identifies the responsible physical node.
    ///
    /// The key invariant is **minimal disruption**: adding or removing a
    /// node causes only O(1/N) of keys to be remapped on average, where
    /// N is the number of physical nodes.
    class IHashRing
    {
    public:
        virtual ~IHashRing() = default;

        // ── Node management ────────────────────────────────────────────

        /// @brief Adds a physical node to the ring.
        ///
        /// The node will be represented by @p virtualNodeCount virtual
        /// replicas distributed around the ring.  A higher count yields
        /// better load distribution at the cost of additional memory and
        /// insertion time.
        /// @param node             Unique name for the physical node.
        /// @param virtualNodeCount Number of virtual replicas (default 3).
        ///        If the node already exists, its virtual node count is
        ///        updated (old virtual replicas are replaced).
        virtual void addNode(const NodeName& node,
                             uint32_t virtualNodeCount = 3) = 0;

        /// @brief Removes a physical node and all its virtual replicas.
        /// @param node The node to remove.
        /// @return true if the node existed and was removed.
        [[nodiscard]] virtual bool removeNode(const NodeName& node) = 0;

        /// @brief Checks whether a physical node is registered on the ring.
        /// @param node The node name to check.
        /// @return true if the node exists.
        [[nodiscard]] virtual bool containsNode(const NodeName& node) const = 0;

        /// @brief Returns the number of distinct physical nodes on the ring.
        /// @return Physical node count.
        [[nodiscard]] virtual uint32_t nodeCount() const = 0;

        // ── Routing ────────────────────────────────────────────────────

        /// @brief Finds the physical node responsible for a given key.
        /// @param key The lookup key.
        /// @return The node name that owns the key.
        /// @throws std::runtime_error if the ring is empty.
        [[nodiscard]] virtual NodeName getNode(std::string_view key) const = 0;

        /// @brief Finds up to @p n distinct physical nodes for replication.
        ///
        /// The first element is the primary owner (same as getNode()).
        /// Subsequent elements are additional replicas chosen by walking
        /// clockwise around the ring.
        /// @param key The lookup key.
        /// @param n   Desired number of distinct replicas.
        /// @return A vector of distinct node names, in clockwise order.
        ///         The result may contain fewer than @p n elements if the
        ///         ring has fewer physical nodes than requested.
        [[nodiscard]] virtual std::vector<NodeName>
        getNodes(std::string_view key, uint32_t n) const = 0;

        // ── Administration ─────────────────────────────────────────────

        /// @brief Removes all nodes from the ring, leaving it empty.
        virtual void clear() = 0;

        /// @brief Returns the total number of virtual entries on the ring.
        /// @return Virtual node count (sum of all per-node virtual counts).
        [[nodiscard]] virtual uint64_t virtualNodeCount() const = 0;
    };

} // namespace common::data_structure::hash
