#include <benchmark/benchmark.h>
#include <numeric>
#include <vector>
#include <random>
#include <algorithm>

#include "data_structure/SkipList.hpp"
#include "data_structure/tree/RedBlackTree.hpp"
#include "data_structure/tree/BinarySearchTree.hpp"
#include "data_structure/TopK.hpp"

// ============================================================
// Helper: generate shuffled data
// ============================================================
static std::vector<int> generate_shuffled(const size_t n)
{
    std::vector<int> data(n);
    std::iota(data.begin(), data.end(), 0);
    std::mt19937 rng(std::random_device{}());
    std::ranges::shuffle(data, rng);
    return data;
}

// ============================================================
// SkipList
// ============================================================
static void BM_SkipList_Insert(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    for (auto _ : state)
    {
        (void)_;
        common::data_structure::SkipList<int> sl;
        for (size_t i = 0; i < n; ++i)
        {
            sl.insert(data[i]);
        }
        benchmark::DoNotOptimize(sl.size());
    }
}
BENCHMARK(BM_SkipList_Insert)->Range(8, 8 << 10);

static void BM_SkipList_Search_Hit(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);
    common::data_structure::SkipList<int> sl;
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

static void BM_SkipList_Erase(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    for (auto _ : state)
    {
        (void)_;
        state.PauseTiming();
        common::data_structure::SkipList<int> sl;
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

// ============================================================
// RedBlackTree
// ============================================================
static void BM_RedBlackTree_Insert(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    for (auto _ : state)
    {
        (void)_;
        common::data_structure::tree::RedBlackTree<int> rbt;
        for (size_t i = 0; i < n; ++i)
        {
            rbt.insert(data[i]);
        }
        benchmark::DoNotOptimize(rbt.getRoot());
    }
}
BENCHMARK(BM_RedBlackTree_Insert)->Range(8, 8 << 10);

static void BM_RedBlackTree_Insert_Sorted(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));

    for (auto _ : state)
    {
        (void)_;
        common::data_structure::tree::RedBlackTree<int> rbt;
        for (size_t i = 0; i < n; ++i)
        {
            rbt.insert(static_cast<int>(i));
        }
        benchmark::DoNotOptimize(rbt.getRoot());
    }
}
BENCHMARK(BM_RedBlackTree_Insert_Sorted)->Range(8, 8 << 10);

// ============================================================
// BinarySearchTree
// ============================================================
static void BM_BST_Insert(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    for (auto _ : state)
    {
        (void)_;
        common::data_structure::tree::BinarySearchTree<int> bst;
        for (size_t i = 0; i < n; ++i)
        {
            bst.insert(data[i]);
        }
        benchmark::DoNotOptimize(bst.find(data[0]));
    }
}
BENCHMARK(BM_BST_Insert)->Range(8, 8 << 10);

static void BM_BST_Find(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);
    common::data_structure::tree::BinarySearchTree<int> bst;
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

static void BM_BST_Remove(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);

    for (auto _ : state)
    {
        (void)_;
        state.PauseTiming();
        common::data_structure::tree::BinarySearchTree<int> bst;
        for (const auto v : data) bst.insert(v);
        state.ResumeTiming();

        for (const auto v : data)
        {
            bst.remove(v);
        }
    }
}
BENCHMARK(BM_BST_Remove)->Range(8, 8 << 10);

// ============================================================
// TopK
// ============================================================
static void BM_TopK_Add(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);
    common::data_structure::TopK topk(static_cast<int32_t>(n));

    for (auto _ : state)
    {
        (void)_;
        state.PauseTiming();
        topk = common::data_structure::TopK(static_cast<int32_t>(n));
        state.ResumeTiming();

        for (const auto v : data)
        {
            topk.add(v);
        }
        benchmark::DoNotOptimize(topk.size());
    }
}
BENCHMARK(BM_TopK_Add)->Range(8, 8 << 10);

static void BM_TopK_GetTopK(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    const auto data = generate_shuffled(n);
    common::data_structure::TopK topk(static_cast<int32_t>(n));
    for (const auto v : data) topk.add(v);

    for (auto _ : state)
    {
        (void)_;
        auto result = topk.getTopK(static_cast<int32_t>(n));
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_TopK_GetTopK)->Range(8, 8 << 10);

// main() provided by benchmark::benchmark_main
