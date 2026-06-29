/**
 * @file ConsistentHash.cc
 * @brief ConsistentHash implementation �?virtual-node consistent hashing
 * @details Implements the IHashRing interface using a std::map as a circular
 *          hash ring.  Each physical node is represented by multiple virtual
 *          replicas (VNode) distributed around the ring for improved load
 *          distribution.  When nodes are added or removed, only O(1/N) of
 *          keys are remapped on average.
 *
 * Thread safety is guaranteed via a per-instance std::mutex that serialises
 * all ring mutations and queries.
 *
 * Reference: Karger et al., "Consistent Hashing and Random Trees:
 *            Distributed Caching Protocols for Relieving Hot Spots on the
 *            World Wide Web" (1997).
 */

#include <cppforge/data_structure/hash/ConsistentHash.hpp>

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace cppforge::data_structure::hash
{

// ══════════════════════════════════════════════════════════════════════════
//  IHashRing interface
// ══════════════════════════════════════════════════════════════════════════

void ConsistentHash::addNode(const NodeName& node, uint32_t virtualNodeCount)
{
    std::lock_guard lock(mutex_);

    if (virtualNodeCount == 0)
    {
        virtualNodeCount = DEFAULT_VNODE_COUNT;
    }

    // Remove existing virtual replicas for this node first (handles
    // re-adding with a different vnode count, or idempotent re-add).
    const auto oldIt = nodeVNodeCounts_.find(node);
    if (oldIt != nodeVNodeCounts_.end())
    {
        const uint32_t oldCount = oldIt->second;
        for (uint32_t i = 0; i < oldCount; ++i)
        {
            ring_.erase(hashVNode(node, i));
        }
    }

    nodeVNodeCounts_[node] = virtualNodeCount;

    for (uint32_t i = 0; i < virtualNodeCount; ++i)
    {
        const uint64_t h = hashVNode(node, i);
        ring_[h] = VNode{node, i};
    }
}

auto ConsistentHash::removeNode(const NodeName& node) -> bool
{
    std::lock_guard lock(mutex_);

    const auto it = nodeVNodeCounts_.find(node);
    if (it == nodeVNodeCounts_.end())
    {
        return false;
    }

    const uint32_t count = it->second;
    nodeVNodeCounts_.erase(it);

    for (uint32_t i = 0; i < count; ++i)
    {
        ring_.erase(hashVNode(node, i));
    }

    return true;
}

auto ConsistentHash::containsNode(const NodeName& node) const -> bool
{
    std::lock_guard lock(mutex_);
    return nodeVNodeCounts_.contains(node);
}

auto ConsistentHash::nodeCount() const -> uint32_t
{
    std::lock_guard lock(mutex_);
    return static_cast<uint32_t>(nodeVNodeCounts_.size());
}

auto ConsistentHash::getNode(std::string_view key) const -> NodeName
{
    std::lock_guard lock(mutex_);

    if (ring_.empty())
    {
        throw std::runtime_error(
            "ConsistentHash::getNode: ring is empty, no nodes available");
    }

    const uint64_t hash = hashBytes(key.data(), key.size());
    auto it = ring_.lower_bound(hash);

    // Wrap around if no entry >= hash (circular ring)
    if (it == ring_.end())
    {
        it = ring_.begin();
    }

    return it->second.physicalNode;
}

auto ConsistentHash::getNodes(std::string_view key, uint32_t n) const
    -> std::vector<NodeName>
{
    std::lock_guard lock(mutex_);

    if (ring_.empty() || n == 0)
    {
        return {};
    }

    const uint64_t hash = hashBytes(key.data(), key.size());
    auto it = ring_.lower_bound(hash);
    if (it == ring_.end())
    {
        it = ring_.begin();
    }

    std::vector<NodeName> result;
    std::set<NodeName> seen;
    const auto startIt = it;

    do
    {
        if (seen.insert(it->second.physicalNode).second)
        {
            result.push_back(it->second.physicalNode);
            if (result.size() >= n)
            {
                break;
            }
        }

        ++it;
        if (it == ring_.end())
        {
            it = ring_.begin();
        }
    }
    while (it != startIt);

    return result;
}

void ConsistentHash::clear()
{
    std::lock_guard lock(mutex_);
    ring_.clear();
    nodeVNodeCounts_.clear();
}

auto ConsistentHash::virtualNodeCount() const -> uint64_t
{
    std::lock_guard lock(mutex_);
    return ring_.size();
}

// ══════════════════════════════════════════════════════════════════════════
//  Internal helpers
// ══════════════════════════════════════════════════════════════════════════

auto ConsistentHash::hashBytes(const void* data, std::size_t len) noexcept
    -> uint64_t
{
    // MurmurHash2-64A body with MurmurHash3 fmix64 finaliser
    // (identical to the hash used in HyperLogLog)
    const auto* bytes = static_cast<const uint8_t*>(data);
    uint64_t h = 0xbea225f9ebdef56bULL ^ static_cast<uint64_t>(len);

    std::size_t i = 0;
    while (i + 8 <= len)
    {
        uint64_t k;
        std::memcpy(&k, bytes + i, 8);
        k *= 0xc6a4a7935bd1e995ULL;
        k ^= k >> 47;
        k *= 0xc6a4a7935bd1e995ULL;
        h ^= k;
        h *= 0xc6a4a7935bd1e995ULL;
        i += 8;
    }

    // Tail bytes (1-7): accumulate remaining bytes into k, then apply
    // one final MurmurHash round.  Fallthrough switch exploits x86
    // byte-order to avoid branching on partial reads.
    uint64_t k = 0;
    switch (len & 7)
    {
        case 7: k ^= static_cast<uint64_t>(bytes[i + 6]) << 48; [[fallthrough]];
        case 6: k ^= static_cast<uint64_t>(bytes[i + 5]) << 40; [[fallthrough]];
        case 5: k ^= static_cast<uint64_t>(bytes[i + 4]) << 32; [[fallthrough]];
        case 4: k ^= static_cast<uint64_t>(bytes[i + 3]) << 24; [[fallthrough]];
        case 3: k ^= static_cast<uint64_t>(bytes[i + 2]) << 16; [[fallthrough]];
        case 2: k ^= static_cast<uint64_t>(bytes[i + 1]) << 8;  [[fallthrough]];
        case 1: k ^= static_cast<uint64_t>(bytes[i]);
            k *= 0xc6a4a7935bd1e995ULL;
            k ^= k >> 47;
            k *= 0xc6a4a7935bd1e995ULL;
            h ^= k;
    }

    // fmix64 finaliser
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;

    return h;
}

auto ConsistentHash::hashVNode(const NodeName& node, uint32_t index) noexcept
    -> uint64_t
{
    const std::string name = vNodeName(node, index);
    return hashBytes(name.data(), name.size());
}

auto ConsistentHash::vNodeName(const NodeName& node, uint32_t index)
    -> std::string
{
    return node + '#' + std::to_string(index);
}

} // namespace cppforge::data_structure::hash
