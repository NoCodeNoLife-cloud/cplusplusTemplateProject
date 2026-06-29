/**
 * @file toolkit_benchmark.cc
 * @brief Micro-benchmarks for utility toolkits and BloomFilter
 * @details Measures BloomFilter insert/contains (hit/miss) throughput;
 *          StringToolkit split, replace, repeat, trim, toUpperCase;
 *          RegexToolkit is_match, get_matches, replace_all;
 *          RadixToolkit number-to-string and string-to-number conversion
 *          across bases 2, 10, 16, and 36.
 */

#include <benchmark/benchmark.h>
#include <random>
#include <string>
#include <vector>

#include "data_structure/filter/BloomFilter.hpp"
#include "toolkit/StringToolkit.hpp"
#include "toolkit/RegexToolkit.hpp"
#include "toolkit/RadixToolkit.hpp"

// ══════════════════════════════════════════════════════════════════════════
//  BloomFilter
// ══════════════════════════════════════════════════════════════════════════

/// @brief Creates a BloomFilter configured for @p expected_elements with 1�? FP rate.
static cppforge::data_structure::BloomFilter create_bloom(const size_t expected_elements)
{
    cppforge::data_structure::BloomParameters params;
    params.projected_element_count = expected_elements;
    params.false_positive_probability = 0.01;
    params.minimum_size = 1;
    params.maximum_size = expected_elements * 20;
    params.minimum_number_of_hashes = 1;
    params.maximum_number_of_hashes = 20;
    (void)params.compute_optimal_parameters();
    return cppforge::data_structure::BloomFilter(params);
}

/// @brief BloomFilter insert throughput with random string keys.
static void BM_BloomFilter_Insert(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    auto bloom = create_bloom(n);
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution dist(0, 1000000);

    for (auto _ : state)
    {
        (void)_;
        bloom.insert(std::to_string(dist(rng)));
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_BloomFilter_Insert)->Arg(1000)->Arg(10000);

/// @brief BloomFilter contains throughput �?all queries target present keys.
static void BM_BloomFilter_Contains_Hit(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    auto bloom = create_bloom(n);
    std::vector<std::string> keys;
    for (size_t i = 0; i < n; ++i) keys.push_back("key_" + std::to_string(i));
    for (auto& k : keys) bloom.insert(k);

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, n - 1);

    for (auto _ : state)
    {
        (void)_;
        bool found = bloom.contains(keys[dist(rng)]);
        benchmark::DoNotOptimize(found);
    }
}
BENCHMARK(BM_BloomFilter_Contains_Hit)->Arg(1000)->Arg(10000);

/// @brief BloomFilter contains throughput �?all queries target absent keys.
static void BM_BloomFilter_Contains_Miss(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    auto bloom = create_bloom(n);
    for (size_t i = 0; i < n; ++i) bloom.insert("key_" + std::to_string(i));

    for (auto _ : state)
    {
        (void)_;
        bool found = bloom.contains("nonexistent_key_999999");
        benchmark::DoNotOptimize(found);
    }
}
BENCHMARK(BM_BloomFilter_Contains_Miss)->Arg(1000)->Arg(10000);

// ══════════════════════════════════════════════════════════════════════════
//  StringToolkit
// ══════════════════════════════════════════════════════════════════════════

/// @brief StringToolkit::split throughput with varying numbers of parts.
static void BM_StringToolkit_Split(benchmark::State& state)
{
    const auto n = static_cast<size_t>(state.range(0));
    std::string text;
    for (size_t i = 0; i < n; ++i)
        text += "part" + std::to_string(i) + ",";

    for (auto _ : state)
    {
        (void)_;
        auto parts = cppforge::toolkit::StringToolkit::split(text, ',');
        benchmark::DoNotOptimize(parts);
    }
}
BENCHMARK(BM_StringToolkit_Split)->Arg(10)->Arg(100)->Arg(1000);

/// @brief StringToolkit::replaceAll throughput (substitution of a fixed marker).
static void BM_StringToolkit_ReplaceAll(benchmark::State& state)
{
    const std::string text = std::string(state.range(0), 'a') + "XYZ" + std::string(state.range(0), 'b');

    for (auto _ : state)
    {
        (void)_;
        auto result = cppforge::toolkit::StringToolkit::replaceAll(text, "XYZ", "REPLACED");
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringToolkit_ReplaceAll)->Arg(100)->Arg(1000)->Arg(10000);

/// @brief StringToolkit::repeat throughput for various repetition counts.
static void BM_StringToolkit_Repeat(benchmark::State& state)
{
    const std::string base = "HelloWorld!";

    for (auto _ : state)
    {
        (void)_;
        auto result = cppforge::toolkit::StringToolkit::repeat(base, state.range(0));
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringToolkit_Repeat)->Arg(10)->Arg(100)->Arg(1000);

/// @brief StringToolkit::trim throughput (removes leading/trailing whitespace).
static void BM_StringToolkit_Trim(benchmark::State& state)
{
    const std::string text = std::string(state.range(0), ' ') + "content" + std::string(state.range(0), ' ');

    for (auto _ : state)
    {
        (void)_;
        auto result = cppforge::toolkit::StringToolkit::trim(text);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringToolkit_Trim)->Arg(100)->Arg(1000)->Arg(10000);

/// @brief StringToolkit::toUpperCase throughput across various input lengths.
static void BM_StringToolkit_ToUpperCase(benchmark::State& state)
{
    const std::string text(state.range(0), 'a');

    for (auto _ : state)
    {
        (void)_;
        auto result = cppforge::toolkit::StringToolkit::toUpperCase(text);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringToolkit_ToUpperCase)->Arg(100)->Arg(1000)->Arg(10000);

// ══════════════════════════════════════════════════════════════════════════
//  RegexToolkit
// ══════════════════════════════════════════════════════════════════════════

/// @brief RegexToolkit::is_match with an email regex pattern.
static void BM_RegexToolkit_IsMatch(benchmark::State& state)
{
    const std::string pattern = R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)";

    for (auto _ : state)
    {
        (void)_;
        bool match = cppforge::toolkit::RegexToolkit::is_match("user.name+tag@example.com", pattern);
        benchmark::DoNotOptimize(match);
    }
}
BENCHMARK(BM_RegexToolkit_IsMatch);

/// @brief RegexToolkit::get_matches throughput (word-boundary pattern).
static void BM_RegexToolkit_GetMatches(benchmark::State& state)
{
    const std::string pattern = R"(\b\w+\b)";
    std::string text(state.range(0), 'a');
    for (size_t i = 0; i < text.size(); i += 10) text[i] = ' ';

    for (auto _ : state)
    {
        (void)_;
        auto matches = cppforge::toolkit::RegexToolkit::get_matches(text, pattern);
        benchmark::DoNotOptimize(matches);
    }
}
BENCHMARK(BM_RegexToolkit_GetMatches)->Arg(100)->Arg(1000);

/// @brief RegexToolkit::replace_all throughput (whitespace-to-underscore).
static void BM_RegexToolkit_ReplaceAll(benchmark::State& state)
{
    const std::string pattern = R"(\s+)";
    std::string text(state.range(0), ' ');
    for (size_t i = 0; i < text.size(); ++i) text[i] = (i % 3 == 0) ? ' ' : 'a';

    for (auto _ : state)
    {
        (void)_;
        auto result = cppforge::toolkit::RegexToolkit::replace_all(text, pattern, "_");
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_RegexToolkit_ReplaceAll)->Arg(100)->Arg(1000);

// ══════════════════════════════════════════════════════════════════════════
//  RadixToolkit
// ══════════════════════════════════════════════════════════════════════════

/// @brief RadixToolkit::convert_to_string throughput across bases 2�?6.
static void BM_RadixToolkit_ConvertToString(benchmark::State& state)
{
    const auto base = static_cast<int>(state.range(0));

    for (auto _ : state)
    {
        constexpr uint64_t value = 0xDEADBEEFCAFEBABEULL;
        (void)_;
        auto s = cppforge::toolkit::RadixToolkit::convert_to_string(value, base, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_RadixToolkit_ConvertToString)->Arg(2)->Arg(10)->Arg(16)->Arg(36);

/// @brief RadixToolkit::convert_from_string throughput across bases 2, 10, 16.
static void BM_RadixToolkit_ConvertFromString(benchmark::State& state)
{
    const auto base = static_cast<int>(state.range(0));
    std::string str = "DEADBEEFCAFEBABE";
    // truncate for smaller bases
    if (base == 2) str = "110111101010110110111110111111001010111111101011101010111110";
    else if (base == 10) str = "17279655951921951166";

    for (auto _ : state)
    {
        (void)_;
        auto val = cppforge::toolkit::RadixToolkit::convert_from_string<uint64_t>(str, base);
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_RadixToolkit_ConvertFromString)->Arg(2)->Arg(10)->Arg(16);

// main() provided by benchmark::benchmark_main
