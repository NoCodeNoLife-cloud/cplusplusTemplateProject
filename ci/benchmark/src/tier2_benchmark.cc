/**
 * @file tier2_benchmark.cc
 * @brief Micro-benchmarks for concurrency and generation utilities
 * @details Measures ThreadPool submit/get latency, batch throughput,
 *          and construction overhead; SnowflakeGenerator ID generation
 *          rate; RandomGenerator throughput for int, double, bool,
 *          string, and gaussian distributions.
 */

#include <benchmark/benchmark.h>
#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "gen/RandomGenerator.hpp"
#include "gen/SnowflakeGenerator.hpp"
#include "thread/ThreadPool.hpp"

// ══════════════════════════════════════════════════════════════════════════
//  ThreadPool
// ══════════════════════════════════════════════════════════════════════════

/// @brief ThreadPool submit + future::get round-trip latency (empty task).
static void BM_ThreadPool_Submit_Get(benchmark::State& state)
{
    common::thread::ThreadPool pool(4, 8, 1024, std::chrono::seconds(60));

    for (auto _ : state)
    {
        (void)_;
        auto fut = pool.submit([] { return 42; });
        int val = fut.get();
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_ThreadPool_Submit_Get);

/// @brief ThreadPool submit + get with a 1 µs simulated workload.
static void BM_ThreadPool_Submit_Sleep1us(benchmark::State& state)
{
    common::thread::ThreadPool pool(4, 8, 1024, std::chrono::seconds(60));

    for (auto _ : state)
    {
        (void)_;
        auto fut = pool.submit([] {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            return 0;
        });
        benchmark::DoNotOptimize(fut.get());
    }
}
BENCHMARK(BM_ThreadPool_Submit_Sleep1us);

/// @brief ThreadPool batch throughput: submit N tasks, then collect all results.
static void BM_ThreadPool_BatchThroughput(benchmark::State& state)
{
    const auto batch_size = static_cast<int>(state.range(0));
    common::thread::ThreadPool pool(4, 8, 65536, std::chrono::seconds(60));
    std::atomic counter{0};

    for (auto _ : state)
    {
        (void)_;
        std::vector<std::future<int>> futs;
        futs.reserve(batch_size);
        for (int i = 0; i < batch_size; ++i)
            futs.push_back(pool.submit([&counter] { return counter.fetch_add(1, std::memory_order_relaxed); }));
        int sum = 0;
        for (auto& f : futs) sum += f.get();
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_ThreadPool_BatchThroughput)->Arg(64)->Arg(256)->Arg(1024);

/// @brief ThreadPool construction + single-task cost (minimal pool).
static void BM_ThreadPool_NoTaskOverhead(benchmark::State& state)
{
    for (auto _ : state)
    {
        (void)_;
        common::thread::ThreadPool pool(2, 4, 64, std::chrono::seconds(60));
        auto fut = pool.submit([] { return 1; });
        benchmark::DoNotOptimize(fut.get());
    }
}
BENCHMARK(BM_ThreadPool_NoTaskOverhead);

// ══════════════════════════════════════════════════════════════════════════
//  SnowflakeGenerator
// ══════════════════════════════════════════════════════════════════════════

/// @brief SnowflakeGenerator sequential ID generation throughput.
static void BM_SnowflakeGenerator_NextId(benchmark::State& state)
{
    common::gen::SnowflakeGenerator gen(1, 1);

    for (auto _ : state)
    {
        (void)_;
        auto id = gen.NextId();
        benchmark::DoNotOptimize(id);
    }
}
BENCHMARK(BM_SnowflakeGenerator_NextId);

// ══════════════════════════════════════════════════════════════════════════
//  RandomGenerator
// ══════════════════════════════════════════════════════════════════════════

/// @brief Uniform random int in [0, 1 000 000].
static void BM_RandomGenerator_NextInt(benchmark::State& state)
{
    common::gen::RandomGenerator rng;

    for (auto _ : state)
    {
        (void)_;
        auto val = rng.nextInt(0, 1000000);
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_RandomGenerator_NextInt);

/// @brief Uniform random double in [0.0, 1.0).
static void BM_RandomGenerator_NextDouble(benchmark::State& state)
{
    common::gen::RandomGenerator rng;

    for (auto _ : state)
    {
        (void)_;
        auto val = rng.nextDouble(0.0, 1.0);
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_RandomGenerator_NextDouble);

/// @brief Random boolean generation throughput.
static void BM_RandomGenerator_NextBool(benchmark::State& state)
{
    common::gen::RandomGenerator rng;

    for (auto _ : state)
    {
        (void)_;
        auto val = rng.nextBool();
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_RandomGenerator_NextBool);

/// @brief Random alphanumeric string generation at various lengths.
static void BM_RandomGenerator_NextString(benchmark::State& state)
{
    common::gen::RandomGenerator rng;
    const auto len = static_cast<size_t>(state.range(0));

    for (auto _ : state)
    {
        (void)_;
        auto s = rng.nextString(len);
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_RandomGenerator_NextString)->Arg(8)->Arg(64)->Arg(256);

/// @brief Gaussian (normal) distribution sample throughput.
static void BM_RandomGenerator_NextGaussian(benchmark::State& state)
{
    common::gen::RandomGenerator rng;

    for (auto _ : state)
    {
        (void)_;
        auto val = rng.nextGaussian(0.0, 1.0);
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_RandomGenerator_NextGaussian);

// main() provided by benchmark::benchmark_main
