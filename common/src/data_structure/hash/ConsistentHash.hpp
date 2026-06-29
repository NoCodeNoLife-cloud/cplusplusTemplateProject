/**
 * @file ConsistentHash.hpp
 * @brief ConsistentHash �?consistent hash ring with virtual nodes
 * @details A production-grade consistent hash ring that maps keys to nodes
 *          using a circular hash space with virtual nodes for improved load
 *          distribution.  When nodes are added or removed, only O(1/N) of
 *          keys are remapped on average.
 *
 * Reference: Karger et al., "Consistent Hashing and Random Trees:
 *            Distributed Caching Protocols for Relieving Hot Spots on the
 *            World Wide Web" (1997).
 */

#pragma once

#include "IHashRing.hpp"

#include <cstdint>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace cppforge::data_structure::hash
{
    /// @brief A production-grade consistent hash ring with virtual nodes.
    ///
    /// @par Thread Safety
    /// All public methods are thread-safe.  Concurrent access is serialised
    /// via an internal std::mutex.
    ///
    /// @par Hash Function
    /// Uses MurmurHash2-64A with the MurmurHash3 fmix64 finaliser �?the
    /// same hash used by the project's HyperLogLog implementation �?for
    /// fast, well-distributed 64-bit hashes.
    ///
    /// @par Virtual Nodes
    /// Each physical node is represented by multiple virtual replicas on
    /// the ring.  The default of 3 virtual nodes is suitable for simple
    /// setups; for production clusters we recommend 100�?00 virtual nodes
    /// per physical node for balanced distributions.
    ///
    /// @par Usage Example
    /// @code
    /// ConsistentHash ring;
    /// ring.addNode("redis-01:6379", 150);
    /// ring.addNode("redis-02:6379", 150);
    ///
    /// const auto node = ring.getNode("user:10086");     // primary
    /// const auto replicas = ring.getNodes("blob:x", 3); // 3 replicas
    /// @endcode
    class ConsistentHash final : public IHashRing
    {
    public:
        /// @brief Default virtual node count used when the caller does not
        ///        specify one.
        static constexpr uint32_t DEFAULT_VNODE_COUNT = 3;

        ConsistentHash() = default;

        // Disable copy (internal std::mutex is non-copyable)
        ConsistentHash(const ConsistentHash&) = delete;
        auto operator=(const ConsistentHash&) -> ConsistentHash& = delete;

        // ── Move ───────────────────────────────────────────────────────
        // std::mutex is not movable, so we must implement move operations
        // manually.  The moved-from object is left in a valid empty state.

        ConsistentHash(ConsistentHash&& other) noexcept
            : ring_(std::move(other.ring_))
            , nodeVNodeCounts_(std::move(other.nodeVNodeCounts_))
        {
            // mutex_ is not moved; the new object gets a fresh one.
        }

        auto operator=(ConsistentHash&& other) noexcept -> ConsistentHash&
        {
            if (this != &other)
            {
                // Lock both to ensure consistency during the swap
                std::lock(mutex_, other.mutex_);
                std::lock_guard lk_this(mutex_, std::adopt_lock);
                std::lock_guard lk_other(other.mutex_, std::adopt_lock);

                ring_ = std::move(other.ring_);
                nodeVNodeCounts_ = std::move(other.nodeVNodeCounts_);
            }
            return *this;
        }

        // ── IHashRing interface ────────────────────────────────────────

        void addNode(const NodeName& node,
                     uint32_t virtualNodeCount = DEFAULT_VNODE_COUNT) override;

        [[nodiscard]] bool removeNode(const NodeName& node) override;

        [[nodiscard]] bool containsNode(const NodeName& node) const override;

        [[nodiscard]] uint32_t nodeCount() const override;

        [[nodiscard]] NodeName getNode(std::string_view key) const override;

        [[nodiscard]] std::vector<NodeName>
        getNodes(std::string_view key, uint32_t n) const override;

        void clear() override;

        [[nodiscard]] uint64_t virtualNodeCount() const override;

    private:
        // ── Internal types ─────────────────────────────────────────────

        /// @brief A virtual replica entry on the ring.
        struct VNode
        {
            NodeName physicalNode; ///< Owning physical node.
            uint32_t index;        ///< Virtual replica index [0, count).
        };

        /// @brief Ring storage: hash value �?virtual node.
        ///        Using std::map for O(log N) lookup with natural ordering.
        using Ring = std::map<uint64_t, VNode>;

        // ── Mutable state (guarded by mutex_) ──────────────────────────

        mutable std::mutex mutex_;
        Ring ring_;                                     ///< Circular hash space.
        std::map<NodeName, uint32_t, std::less<>>
            nodeVNodeCounts_;                           ///< Physical node �?virtual count.

        // ── Internal helpers ───────────────────────────────────────────

        /// @brief MurmurHash2-64A with fmix64 finaliser.
        /// @param data  Pointer to the input bytes.
        /// @param len   Number of bytes.
        /// @return 64-bit uniformly distributed hash value.
        static uint64_t hashBytes(const void* data, std::size_t len) noexcept;

        /// @brief Computes the hash for a virtual node.
        ///        The virtual node identifier is "<physicalNode>#<index>".
        /// @param node  Physical node name.
        /// @param index Virtual replica index.
        /// @return 64-bit hash value.
        [[nodiscard]] static uint64_t hashVNode(const NodeName& node,
                                                uint32_t index) noexcept;

        /// @brief Forms the string "<node>#<index>" used for hashing.
        [[nodiscard]] static std::string vNodeName(const NodeName& node,
                                                   uint32_t index);
    };

} // namespace cppforge::data_structure::hash
