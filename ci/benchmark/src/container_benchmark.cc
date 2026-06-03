#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>

#include "container/Heap.hpp"
#include "container/UnionSet.hpp"

// ============================================================
// Heap
// ============================================================
static void BM_Heap_Push(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, 1000000);

    for (auto _ : state)
    {
        (void)_;
        common::container::Heap<int> heap;
        for (size_t i = 0; i < n; ++i)
        {
            heap.push(dist(rng));
        }
        benchmark::DoNotOptimize(heap.size());
    }
}
BENCHMARK(BM_Heap_Push)->Range(64, 8 << 10);

static void BM_Heap_Pop(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, 1000000);

    for (auto _ : state)
    {
        (void)_;
        state.PauseTiming();
        common::container::Heap<int> heap;
        for (size_t i = 0; i < n; ++i) heap.push(dist(rng));
        state.ResumeTiming();

        for (size_t i = 0; i < n; ++i)
        {
            heap.pop();
        }
    }
}
BENCHMARK(BM_Heap_Pop)->Range(64, 8 << 10);

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
        common::container::Heap<int> heap(data.begin(), data.end());
        benchmark::DoNotOptimize(heap.size());
    }
}
BENCHMARK(BM_Heap_BulkConstruction)->Range(64, 8 << 10);

static void BM_Heap_PushPop_Interleaved(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, 1000000);

    common::container::Heap<int> heap;
    for (size_t i = 0; i < n; ++i) heap.push(dist(rng));

    for (auto _ : state)
    {
        (void)_;
        heap.push(dist(rng));
        heap.pop();
    }
}
BENCHMARK(BM_Heap_PushPop_Interleaved)->Range(64, 8 << 10);

// ============================================================
// UnionSet (Disjoint Set Union)
// ============================================================
static void BM_UnionSet_Find(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    common::container::UnionSet<int> us;
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

static void BM_UnionSet_Union(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));

    for (auto _ : state)
    {
        (void)_;
        common::container::UnionSet<int> us;
        for (size_t i = 0; i < n; ++i)
        {
        (void)us.unionSets(static_cast<int>(i), static_cast<int>(i + 1));
    }
        benchmark::DoNotOptimize(us.connected(0, static_cast<int>(n)));
    }
}
BENCHMARK(BM_UnionSet_Union)->Range(64, 8 << 10);

// main() provided by benchmark::benchmark_main
