/**
 * @file hash_benchmark.cc
 * @brief Micro-benchmarks for hash-based data structures and filters
 * @details Measures insertion, lookup (hit/miss), and erasure throughput for
 *          ConsistentHash, RobinHoodHashMap, CuckooHashMap, and CuckooFilter
 *          under varying sizes and configurations.  Integer-keyed maps use
 *          shuffled [0, n) sequences; string-keyed CuckooFilter uses
 *          a "key_<n>" naming pattern.
 */

#include <benchmark/benchmark.h>

#include <algorithm>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "data_structure/filter/CuckooFilter.hpp"
#include "data_structure/hash/ConsistentHash.hpp"
#include "data_structure/hash/CuckooHashMap.hpp"
#include "data_structure/hash/RobinHoodHashMap.hpp"

// ══════════════════════════════════════════════════════════════════════════
//  Helpers
// ══════════════════════════════════════════════════════════════════════════

/// @brief Returns a deterministically-seeded RNG for reproducible benchmarks.
static auto make_rng() -> std::mt19937
{
    return std::mt19937{42};
}

/// @brief Produces a shuffled vector of [0, n) for benchmark input generation.
static std::vector<int> generate_shuffled(const size_t n)
{
    std::vector<int> data(n);
    std::iota(data.begin(), data.end(), 0);
    auto rng = make_rng();
    std::ranges::shuffle(data, rng);
    return data;
}

/// @brief Produces a vector of "node-<i>" names for ConsistentHash benchmarks.
static std::vector<std::string> generate_node_names(const size_t n)
{
    std::vector<std::string> names;
    names.reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
        names.push_back("node-" + std::to_string(i));
    }
    return names;
}

/// @brief Produces a vector of "key_<i>" strings for CuckooFilter benchmarks.
static std::vector<std::string> generate_string_keys(const size_t n)
{
    std::vector<std::string> keys;
    keys.reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
        keys.push_back("key_" + std::to_string(i));
    }
    return keys;
}

// ══════════════════════════════════════════════════════════════════════════
//  ConsistentHash
// ══════════════════════════════════════════════════════════════════════════

/// @brief ConsistentHash node insertion throughput �?ring is freshly
///        constructed each iteration.
static void BM_ConsistentHash_AddNode(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto node_names = generate_node_names(n);

    for (auto _ : state)
    {
        (void)_;
        cppforge::data_structure::hash::ConsistentHash ring;
        for (size_t i = 0; i < n; ++i)
        {
            ring.addNode(node_names[i], 3);
        }
        benchmark::DoNotOptimize(ring.nodeCount());
    }
}
BENCHMARK(BM_ConsistentHash_AddNode)->Range(64, 8 << 10);

/// @brief ConsistentHash primary-lookup throughput on a populated ring.
///        The ring is populated once with `range(0)` nodes.
static void BM_ConsistentHash_GetNode(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto node_names = generate_node_names(n);

    cppforge::data_structure::hash::ConsistentHash ring;
    for (const auto& name : node_names)
        ring.addNode(name, 3);

    auto rng = make_rng();
    std::uniform_int_distribution<size_t> dist(0, n - 1);
    std::vector<std::string> queries(n);
    for (auto& q : queries)
        q = std::to_string(dist(rng));

    size_t idx = 0;
    for (auto _ : state)
    {
        (void)_;
        auto node = ring.getNode(queries[idx++ % queries.size()]);
        benchmark::DoNotOptimize(node);
    }
}
BENCHMARK(BM_ConsistentHash_GetNode)->Range(64, 8 << 10);

/// @brief ConsistentHash replica lookup (3 replicas) on a populated ring.
static void BM_ConsistentHash_GetNodes_Replicas3(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto node_names = generate_node_names(n);

    cppforge::data_structure::hash::ConsistentHash ring;
    for (const auto& name : node_names)
        ring.addNode(name, 3);

    auto rng = make_rng();
    std::uniform_int_distribution<size_t> dist(0, n - 1);
    std::vector<std::string> queries(n);
    for (auto& q : queries)
        q = std::to_string(dist(rng));

    size_t idx = 0;
    for (auto _ : state)
    {
        (void)_;
        auto nodes = ring.getNodes(queries[idx++ % queries.size()], 3);
        benchmark::DoNotOptimize(nodes);
    }
}
BENCHMARK(BM_ConsistentHash_GetNodes_Replicas3)->Range(64, 8 << 10);

/// @brief ConsistentHash node removal throughput �?pacing timing excludes
///        re-population.
static void BM_ConsistentHash_RemoveNode(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto node_names = generate_node_names(n);

    for (auto _ : state)
    {
        (void)_;
        state.PauseTiming();
        cppforge::data_structure::hash::ConsistentHash ring;
        for (const auto& name : node_names)
        {
            ring.addNode(name, 3);
        }
        state.ResumeTiming();

        for (const auto& name : node_names)
        {
            bool removed = ring.removeNode(name);
            benchmark::DoNotOptimize(removed);
        }
    }
}
BENCHMARK(BM_ConsistentHash_RemoveNode)->Range(64, 8 << 10);

// ══════════════════════════════════════════════════════════════════════════
//  RobinHoodHashMap<int, int>
// ══════════════════════════════════════════════════════════════════════════

/// @brief RobinHoodHashMap insert throughput with shuffled integer keys.
static void BM_RobinHoodMap_Insert(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    for (auto _ : state)
    {
        (void)_;
        cppforge::data_structure::hash::RobinHoodHashMap<int, int> map;
        for (size_t i = 0; i < n; ++i)
        {
            map.insert(data[i], data[i]);
        }
        benchmark::DoNotOptimize(map.size());
    }
}
BENCHMARK(BM_RobinHoodMap_Insert)->Range(64, 8 << 10);

/// @brief RobinHoodHashMap find throughput �?all queries target present keys.
static void BM_RobinHoodMap_Find_Hit(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    cppforge::data_structure::hash::RobinHoodHashMap<int, int> map;
    for (const auto v : data) map.insert(v, v);

    auto rng = make_rng();
    std::uniform_int_distribution dist(0, static_cast<int>(n - 1));

    for (auto _ : state)
    {
        (void)_;
        bool found = map.contains(data[dist(rng)]);
        benchmark::DoNotOptimize(found);
    }
}
BENCHMARK(BM_RobinHoodMap_Find_Hit)->Range(64, 8 << 10);

/// @brief RobinHoodHashMap find throughput �?all queries target absent keys.
static void BM_RobinHoodMap_Find_Miss(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    cppforge::data_structure::hash::RobinHoodHashMap<int, int> map;
    for (const auto v : data) map.insert(v, v);

    auto rng = make_rng();
    std::uniform_int_distribution<int> dist(static_cast<int>(n),
                                            static_cast<int>(2 * n - 1));

    for (auto _ : state)
    {
        (void)_;
        bool found = map.contains(dist(rng));
        benchmark::DoNotOptimize(found);
    }
}
BENCHMARK(BM_RobinHoodMap_Find_Miss)->Range(64, 8 << 10);

/// @brief RobinHoodHashMap erase throughput �?pacing timing excludes
///        re-population.
static void BM_RobinHoodMap_Erase(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    for (auto _ : state)
    {
        (void)_;
        state.PauseTiming();
        cppforge::data_structure::hash::RobinHoodHashMap<int, int> map;
        for (const auto v : data) map.insert(v, v);
        state.ResumeTiming();

        for (const auto v : data)
        {
            bool erased = map.erase(v);
            benchmark::DoNotOptimize(erased);
        }
    }
}
BENCHMARK(BM_RobinHoodMap_Erase)->Range(64, 8 << 10);

// ══════════════════════════════════════════════════════════════════════════
//  CuckooHashMap<int, int>
// ══════════════════════════════════════════════════════════════════════════

/// @brief CuckooHashMap insert throughput with shuffled integer keys.
static void BM_CuckooMap_Insert(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    for (auto _ : state)
    {
        (void)_;
        cppforge::data_structure::hash::CuckooHashMap<int, int> map;
        for (size_t i = 0; i < n; ++i)
        {
            map.insert(data[i], data[i]);
        }
        benchmark::DoNotOptimize(map.size());
    }
}
BENCHMARK(BM_CuckooMap_Insert)->Range(64, 8 << 10);

/// @brief CuckooHashMap find throughput �?all queries target present keys.
static void BM_CuckooMap_Find_Hit(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    cppforge::data_structure::hash::CuckooHashMap<int, int> map;
    for (const auto v : data) map.insert(v, v);

    auto rng = make_rng();
    std::uniform_int_distribution dist(0, static_cast<int>(n - 1));

    for (auto _ : state)
    {
        (void)_;
        bool found = map.contains(data[dist(rng)]);
        benchmark::DoNotOptimize(found);
    }
}
BENCHMARK(BM_CuckooMap_Find_Hit)->Range(64, 8 << 10);

/// @brief CuckooHashMap find throughput �?all queries target absent keys.
static void BM_CuckooMap_Find_Miss(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    cppforge::data_structure::hash::CuckooHashMap<int, int> map;
    for (const auto v : data) map.insert(v, v);

    auto rng = make_rng();
    std::uniform_int_distribution<int> dist(static_cast<int>(n),
                                            static_cast<int>(2 * n - 1));

    for (auto _ : state)
    {
        (void)_;
        bool found = map.contains(dist(rng));
        benchmark::DoNotOptimize(found);
    }
}
BENCHMARK(BM_CuckooMap_Find_Miss)->Range(64, 8 << 10);

/// @brief CuckooHashMap erase throughput �?pacing timing excludes
///        re-population.
static void BM_CuckooMap_Erase(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    for (auto _ : state)
    {
        (void)_;
        state.PauseTiming();
        cppforge::data_structure::hash::CuckooHashMap<int, int> map;
        for (const auto v : data) map.insert(v, v);
        state.ResumeTiming();

        for (const auto v : data)
        {
            bool erased = map.erase(v);
            benchmark::DoNotOptimize(erased);
        }
    }
}
BENCHMARK(BM_CuckooMap_Erase)->Range(64, 8 << 10);

// ══════════════════════════════════════════════════════════════════════════
//  CuckooFilter<std::string>
// ══════════════════════════════════════════════════════════════════════════

/// @brief CuckooFilter insert throughput with pre-generated string keys.
///        A fresh filter is created each iteration.
static void BM_CuckooFilter_Insert(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto keys = generate_string_keys(n);

    for (auto _ : state)
    {
        (void)_;
        cppforge::data_structure::CuckooFilter<std::string> filter(
            static_cast<uint64_t>(n));
        for (size_t i = 0; i < n; ++i)
        {
            bool inserted = filter.insert(keys[i]);
            benchmark::DoNotOptimize(inserted);
        }
        benchmark::DoNotOptimize(filter);
    }
}
BENCHMARK(BM_CuckooFilter_Insert)->Range(64, 8 << 10);

/// @brief CuckooFilter contains throughput �?all queries target present keys.
static void BM_CuckooFilter_Contains_Hit(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto keys = generate_string_keys(n);

    cppforge::data_structure::CuckooFilter<std::string> filter(
        static_cast<uint64_t>(n));
    for (const auto& k : keys) filter.insert(k);

    auto rng = make_rng();
    std::uniform_int_distribution<size_t> dist(0, n - 1);

    for (auto _ : state)
    {
        (void)_;
        bool found = filter.contains(keys[dist(rng)]);
        benchmark::DoNotOptimize(found);
    }
}
BENCHMARK(BM_CuckooFilter_Contains_Hit)->Range(64, 8 << 10);

/// @brief CuckooFilter contains throughput �?all queries target absent keys
///        from a disjoint key space.
static void BM_CuckooFilter_Contains_Miss(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));

    cppforge::data_structure::CuckooFilter<std::string> filter(
        static_cast<uint64_t>(n));
    for (size_t i = 0; i < n; ++i)
        filter.insert("key_" + std::to_string(i));

    // Pre-generate miss queries from a disjoint key space
    std::vector<std::string> miss_keys(n);
    for (size_t i = 0; i < n; ++i)
        miss_keys[i] = "miss_" + std::to_string(i + n);

    auto rng = make_rng();
    std::uniform_int_distribution<size_t> dist(0, n - 1);

    for (auto _ : state)
    {
        (void)_;
        bool found = filter.contains(miss_keys[dist(rng)]);
        benchmark::DoNotOptimize(found);
    }
}
BENCHMARK(BM_CuckooFilter_Contains_Miss)->Range(64, 8 << 10);

/// @brief CuckooFilter remove throughput �?pacing timing excludes
///        re-population.
static void BM_CuckooFilter_Remove(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto keys = generate_string_keys(n);

    for (auto _ : state)
    {
        (void)_;
        state.PauseTiming();
        cppforge::data_structure::CuckooFilter<std::string> filter(
            static_cast<uint64_t>(n));
        for (const auto& k : keys) filter.insert(k);
        state.ResumeTiming();

        for (const auto& k : keys)
        {
            bool removed = filter.remove(k);
            benchmark::DoNotOptimize(removed);
        }
    }
}
BENCHMARK(BM_CuckooFilter_Remove)->Range(64, 8 << 10);

// main() provided by benchmark::benchmark_main
