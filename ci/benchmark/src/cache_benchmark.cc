#include <benchmark/benchmark.h>
#include <random>

#include "cache/LRUCache.hpp"
#include "cache/LFUCache.hpp"

// ============================================================
// LRUCache
// ============================================================
static void BM_LRUCache_Put(benchmark::State& state)
{
    const auto capacity = static_cast<int>(state.range(0));
    common::cache::LRUCache<int, int> cache(capacity);
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, capacity * 2);

    for (auto _ : state)
    {
        (void)_;
        (void)cache.put(dist(rng), dist(rng));
    }
}
BENCHMARK(BM_LRUCache_Put)->Arg(16)->Arg(128)->Arg(1024);

static void BM_LRUCache_Get_Hit(benchmark::State& state)
{
    const auto capacity = static_cast<int>(state.range(0));
    common::cache::LRUCache<int, int> cache(capacity);
    for (int i = 0; i < capacity; ++i) (void)cache.put(i, i);
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, capacity - 1);

    for (auto _ : state)
    {
        (void)_;
        benchmark::DoNotOptimize(cache.get(dist(rng)));
    }
}
BENCHMARK(BM_LRUCache_Get_Hit)->Arg(16)->Arg(128)->Arg(1024);

static void BM_LRUCache_Get_Miss(benchmark::State& state)
{
    const auto capacity = static_cast<int>(state.range(0));
    common::cache::LRUCache<int, int> cache(capacity);
    for (int i = 0; i < capacity; ++i) (void)cache.put(i, i);

    for (auto _ : state)
    {
        (void)_;
        benchmark::DoNotOptimize(cache.get(capacity + 9999));
    }
}
BENCHMARK(BM_LRUCache_Get_Miss)->Arg(16)->Arg(128)->Arg(1024);

static void BM_LRUCache_Put_Evict(benchmark::State& state)
{
    const auto capacity = static_cast<int>(state.range(0));
    common::cache::LRUCache<int, int> cache(capacity);

    for (auto _ : state)
    {
        (void)_;
        for (int i = 0; i < capacity * 2; ++i)
        {
            (void)cache.put(i, i);
        }
        benchmark::DoNotOptimize(cache.size());
    }
}
BENCHMARK(BM_LRUCache_Put_Evict)->Arg(16)->Arg(128)->Arg(1024);

// ============================================================
// LFUCache
// ============================================================
static void BM_LFUCache_Put(benchmark::State& state)
{
    const auto capacity = static_cast<int>(state.range(0));
    common::cache::LFUCache<int, int> cache(capacity);
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, capacity * 2);

    for (auto _ : state)
    {
        (void)_;
        (void)cache.put(dist(rng), dist(rng));
    }
}
BENCHMARK(BM_LFUCache_Put)->Arg(16)->Arg(128)->Arg(1024);

static void BM_LFUCache_Get_Hit(benchmark::State& state)
{
    const auto capacity = static_cast<int>(state.range(0));
    common::cache::LFUCache<int, int> cache(capacity);
    for (int i = 0; i < capacity; ++i) (void)cache.put(i, i);
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, capacity - 1);

    for (auto _ : state)
    {
        (void)_;
        benchmark::DoNotOptimize(cache.get(dist(rng)));
    }
}
BENCHMARK(BM_LFUCache_Get_Hit)->Arg(16)->Arg(128)->Arg(1024);

static void BM_LFUCache_Put_Evict(benchmark::State& state)
{
    const auto capacity = static_cast<int>(state.range(0));
    common::cache::LFUCache<int, int> cache(capacity);

    for (auto _ : state)
    {
        (void)_;
        for (int i = 0; i < capacity * 2; ++i)
        {
            (void)cache.put(i, i);
        }
        benchmark::DoNotOptimize(cache.size());
    }
}
BENCHMARK(BM_LFUCache_Put_Evict)->Arg(16)->Arg(128)->Arg(1024);

// main() provided by benchmark::benchmark_main
