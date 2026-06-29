/**
 * @file data_structure_benchmark.cc
 * @brief Micro-benchmarks for fundamental data structures
 * @details Measures insertion, search, and erasure throughput for
 *          SkipList, RedBlackTree, BinarySearchTree, and TopK;
 *          push/pop/bulk-construction for Heap; find/union for
 *          UnionSet (disjoint-set).  All benchmarks use integer
 *          keys with shuffled or sequential input distributions.
 */

#include <benchmark/benchmark.h>
#include <numeric>
#include <vector>
#include <random>
#include <algorithm>

#include <cppforge/data_structure/heap/Heap.hpp>
#include <cppforge/data_structure/union_find/UnionSet.hpp>
#include <cppforge/data_structure/list/SkipList.hpp>
#include <cppforge/data_structure/tree/balanced/RedBlackTree.hpp>
#include <cppforge/data_structure/tree/core/BinarySearchTree.hpp>
#include <cppforge/data_structure/top_k/TopK.hpp>

// ══════════════════════════════════════════════════════════════════════════
//  Helpers
// ══════════════════════════════════════════════════════════════════════════

/// @brief Produces a shuffled vector of [0, n) for benchmark input generation.
static std::vector<int> generate_shuffled(const size_t n)
{
    std::vector<int> data(n);
    std::iota(data.begin(), data.end(), 0);
    std::mt19937 rng(std::random_device{}());
    std::ranges::shuffle(data, rng);
    return data;
}

// ══════════════════════════════════════════════════════════════════════════
//  SkipList
// ══════════════════════════════════════════════════════════════════════════

/// @brief SkipList insertion throughput with shuffled keys.
static void BM_SkipList_Insert(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    for (auto _ : state)
    {
        (void)_;
        cppforge::data_structure::SkipList<int> sl;
        for (size_t i = 0; i < n; ++i)
        {
            sl.insert(data[i]);
        }
        benchmark::DoNotOptimize(sl.size());
    }
}
BENCHMARK(BM_SkipList_Insert)->Range(8, 8 << 10);

/// @brief SkipList search throughput �?all queries target present keys.
static void BM_SkipList_Search_Hit(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);
    cppforge::data_structure::SkipList<int> sl;
    for (auto v : data) sl.insert(v);
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, static_cast<int>(n - 1));

    for (auto _ : state)
    {
        (void)_;
        bool found = sl.search(data[dist(rng)]);
        benchmark::DoNotOptimize(found);
    }
}
BENCHMARK(BM_SkipList_Search_Hit)->Range(8, 8 << 10);

/// @brief SkipList erase throughput �?pacing timing excludes re-population.
static void BM_SkipList_Erase(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    for (auto _ : state)
    {
        (void)_;
        state.PauseTiming();
        cppforge::data_structure::SkipList<int> sl;
        for (auto v : data) sl.insert(v);
        state.ResumeTiming();

        for (auto v : data)
        {
            bool erased = sl.erase(v);
            benchmark::DoNotOptimize(erased);
        }
    }
}
BENCHMARK(BM_SkipList_Erase)->Range(8, 8 << 10);

// ══════════════════════════════════════════════════════════════════════════
//  RedBlackTree
// ══════════════════════════════════════════════════════════════════════════

/// @brief RBT insertion throughput with shuffled (randomised) keys.
static void BM_RedBlackTree_Insert(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    for (auto _ : state)
    {
        (void)_;
        cppforge::data_structure::tree::balanced::RedBlackTree<int> rbt;
        for (size_t i = 0; i < n; ++i)
        {
            rbt.insert(data[i]);
        }
        benchmark::DoNotOptimize(rbt.getRoot());
    }
}
BENCHMARK(BM_RedBlackTree_Insert)->Range(8, 8 << 10);

/// @brief RBT insertion throughput with sequentially increasing keys.
static void BM_RedBlackTree_Insert_Sorted(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));

    for (auto _ : state)
    {
        (void)_;
        cppforge::data_structure::tree::balanced::RedBlackTree<int> rbt;
        for (size_t i = 0; i < n; ++i)
        {
            rbt.insert(static_cast<int>(i));
        }
        benchmark::DoNotOptimize(rbt.getRoot());
    }
}
BENCHMARK(BM_RedBlackTree_Insert_Sorted)->Range(8, 8 << 10);

// ══════════════════════════════════════════════════════════════════════════
//  BinarySearchTree
// ══════════════════════════════════════════════════════════════════════════

/// @brief Unbalanced BST insertion throughput with shuffled keys.
static void BM_BST_Insert(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    for (auto _ : state)
    {
        (void)_;
        cppforge::data_structure::tree::core::BinarySearchTree<int> bst;
        for (size_t i = 0; i < n; ++i)
        {
            bst.insert(data[i]);
        }
        benchmark::DoNotOptimize(bst.find(data[0]));
    }
}
BENCHMARK(BM_BST_Insert)->Range(8, 8 << 10);

/// @brief BST search throughput �?all queries target present keys.
static void BM_BST_Find(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);
    cppforge::data_structure::tree::core::BinarySearchTree<int> bst;
    for (const auto v : data) bst.insert(v);
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, static_cast<int>(n - 1));

    for (auto _ : state)
    {
        (void)_;
        bool found = bst.find(data[dist(rng)]);
        benchmark::DoNotOptimize(found);
    }
}
BENCHMARK(BM_BST_Find)->Range(8, 8 << 10);

/// @brief BST erase throughput �?pacing timing excludes re-population.
static void BM_BST_Remove(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    for (auto _ : state)
    {
        (void)_;
        state.PauseTiming();
        cppforge::data_structure::tree::core::BinarySearchTree<int> bst;
        for (const auto v : data) bst.insert(v);
        state.ResumeTiming();

        for (const auto v : data)
        {
            bst.remove(v);
        }
    }
}
BENCHMARK(BM_BST_Remove)->Range(8, 8 << 10);

// ══════════════════════════════════════════════════════════════════════════
//  TopK
// ══════════════════════════════════════════════════════════════════════════

/// @brief TopK::add throughput �?pacing excludes container re-creation.
static void BM_TopK_Add(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);
    cppforge::data_structure::TopK topk(static_cast<int32_t>(n));

    for (auto _ : state)
    {
        (void)_;
        state.PauseTiming();
        topk = cppforge::data_structure::TopK(static_cast<int32_t>(n));
        state.ResumeTiming();

        for (const auto v : data)
        {
            topk.add(v);
        }
        benchmark::DoNotOptimize(topk.size());
    }
}
BENCHMARK(BM_TopK_Add)->Range(8, 8 << 10);

/// @brief TopK::getTopK throughput after all elements are inserted.
static void BM_TopK_GetTopK(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);
    cppforge::data_structure::TopK topk(static_cast<int32_t>(n));
    for (const auto v : data) topk.add(v);

    for (auto _ : state)
    {
        (void)_;
        auto result = topk.getTopK(static_cast<int32_t>(n));
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_TopK_GetTopK)->Range(8, 8 << 10);

// ══════════════════════════════════════════════════════════════════════════
//  Heap
// ══════════════════════════════════════════════════════════════════════════

/// @brief Min-heap push throughput with random values.
static void BM_Heap_Push(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, 1000000);

    for (auto _ : state)
    {
        (void)_;
        cppforge::data_structure::Heap<int> heap;
        for (size_t i = 0; i < n; ++i)
        {
            heap.push(dist(rng));
        }
        benchmark::DoNotOptimize(heap.size());
    }
}
BENCHMARK(BM_Heap_Push)->Range(64, 8 << 10);

/// @brief Min-heap pop throughput �?pacing excludes re-population.
static void BM_Heap_Pop(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, 1000000);

    for (auto _ : state)
    {
        (void)_;
        state.PauseTiming();
        cppforge::data_structure::Heap<int> heap;
        for (size_t i = 0; i < n; ++i) heap.push(dist(rng));
        state.ResumeTiming();

        for (size_t i = 0; i < n; ++i)
        {
            heap.pop();
        }
    }
}
BENCHMARK(BM_Heap_Pop)->Range(64, 8 << 10);

/// @brief Heap bulk-construction from shuffled iterators (heapify).
static void BM_Heap_BulkConstruction(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    std::vector<int> data(n);
    std::iota(data.begin(), data.end(), 0);
    std::mt19937 rng(std::random_device{}());
    std::ranges::shuffle(data, rng);

    for (auto _ : state)
    {
        (void)_;
        cppforge::data_structure::Heap<int> heap(data.begin(), data.end());
        benchmark::DoNotOptimize(heap.size());
    }
}
BENCHMARK(BM_Heap_BulkConstruction)->Range(64, 8 << 10);

/// @brief Interleaved push+pop on a pre-filled heap (steady-state size).
static void BM_Heap_PushPop_Interleaved(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, 1000000);

    cppforge::data_structure::Heap<int> heap;
    for (size_t i = 0; i < n; ++i) heap.push(dist(rng));

    for (auto _ : state)
    {
        (void)_;
        heap.push(dist(rng));
        heap.pop();
    }
}
BENCHMARK(BM_Heap_PushPop_Interleaved)->Range(64, 8 << 10);

// ══════════════════════════════════════════════════════════════════════════
//  UnionSet (Disjoint Set Union)
// ══════════════════════════════════════════════════════════════════════════

/// @brief UnionSet find throughput after a chain of union operations.
static void BM_UnionSet_Find(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    cppforge::data_structure::UnionSet<int> us;
    for (size_t i = 0; i < n; ++i)
    {
        (void)us.unionSets(static_cast<int>(i), static_cast<int>(i + 1));
    }
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, static_cast<int>(n));

    for (auto _ : state)
    {
        (void)_;
        benchmark::DoNotOptimize(us.find(dist(rng)));
    }
}
BENCHMARK(BM_UnionSet_Find)->Range(64, 8 << 10);

/// @brief UnionSet consecutive union throughput with a fresh instance each iteration.
static void BM_UnionSet_Union(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));

    for (auto _ : state)
    {
        (void)_;
        cppforge::data_structure::UnionSet<int> us;
        for (size_t i = 0; i < n; ++i)
        {
        (void)us.unionSets(static_cast<int>(i), static_cast<int>(i + 1));
    }
        benchmark::DoNotOptimize(us.connected(0, static_cast<int>(n)));
    }
}
BENCHMARK(BM_UnionSet_Union)->Range(64, 8 << 10);

// main() provided by benchmark::benchmark_main
