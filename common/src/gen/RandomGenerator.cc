#include "src/gen/RandomGenerator.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <random>
#include <string>
#include <stdexcept>
#include <mutex>

namespace common::gen {
    RandomGenerator::RandomGenerator() noexcept : engine_(std::random_device{}()) {
        DLOG(INFO) << "RandomGenerator initialized with random device seed";
    }

    RandomGenerator::RandomGenerator(const unsigned int seed) noexcept : engine_(seed) {
        DLOG(INFO) << fmt::format("RandomGenerator initialized with seed: {}", seed);
    }

    auto RandomGenerator::nextInt(const int min, const int max) -> int {
        if (min > max) {
            DLOG(ERROR) << fmt::format("RandomGenerator nextInt failed - min > max: min={}, max={}", min, max);
            throw std::invalid_argument("common::RandomGenerator::nextInt: min cannot be greater than max");
        }
        std::lock_guard lock(mutex_);
        std::uniform_int_distribution dist(min, max);
        const auto result = dist(engine_);
        DLOG(INFO) << fmt::format("RandomGenerator nextInt - range=[{}, {}], result={}", min, max, result);
        return result;
    }

    auto RandomGenerator::nextBool() -> bool {
        return nextBool(0.5);
    }

    auto RandomGenerator::nextBool(const double trueProbability) -> bool {
        if (trueProbability < 0.0 || trueProbability > 1.0) {
            DLOG(ERROR) << fmt::format("RandomGenerator nextBool failed - probability out of range: {}", trueProbability);
            throw std::invalid_argument("common::RandomGenerator::nextBool: probability must be in [0, 1]");
        }
        std::lock_guard lock(mutex_);
        std::bernoulli_distribution dist(trueProbability);
        const auto result = dist(engine_);
        DLOG(INFO) << fmt::format("RandomGenerator nextBool - probability={}, result={}", trueProbability, result);
        return result;
    }

    auto RandomGenerator::nextDouble(const double min, const double max) -> double {
        if (min >= max) {
            DLOG(ERROR) << fmt::format("RandomGenerator nextDouble failed - min >= max: min={}, max={}", min, max);
            throw std::invalid_argument("common::RandomGenerator::nextDouble: min must be less than max");
        }
        std::lock_guard lock(mutex_);
        std::uniform_real_distribution dist(min, max);
        const auto result = dist(engine_);
        DLOG(INFO) << fmt::format("RandomGenerator nextDouble - range=[{}, {}), result={}", min, max, result);
        return result;
    }

    auto RandomGenerator::nextString(const size_t length, const std::string &charset) -> std::string {
        if (charset.empty()) {
            DLOG(ERROR) << "RandomGenerator nextString failed - charset is empty";
            throw std::invalid_argument("common::RandomGenerator::nextString: charset cannot be empty");
        }
        if (length == 0) {
            DLOG(INFO) << "RandomGenerator nextString - length is 0, returning empty string";
            return {};
        }

        std::lock_guard lock(mutex_);
        std::string result;
        result.reserve(length);

        // Pre-calculate charset size to avoid repeated size() calls
        const size_t charSetSize = charset.size();
        std::uniform_int_distribution<size_t> dist(0, charSetSize - 1);

        for (size_t i = 0; i < length; ++i) {
            result += charset[dist(engine_)];
        }
        DLOG(INFO) << fmt::format("RandomGenerator nextString - length={}, charset_size={}", length, charSetSize);
        return result;
    }

    auto RandomGenerator::nextGaussian(const double mean, const double stddev) -> double {
        if (stddev <= 0.0) {
            DLOG(ERROR) << fmt::format("RandomGenerator nextGaussian failed - stddev must be positive: {}", stddev);
            throw std::invalid_argument("common::RandomGenerator::nextGaussian: stddev must be positive");
        }
        std::lock_guard lock(mutex_);
        std::normal_distribution dist(mean, stddev);
        const auto result = dist(engine_);
        DLOG(INFO) << fmt::format("RandomGenerator nextGaussian - mean={}, stddev={}, result={}", mean, stddev, result);
        return result;
    }

    auto RandomGenerator::setSeed(const unsigned int seed) -> void {
        DLOG(INFO) << fmt::format("RandomGenerator setSeed - new seed: {}", seed);
        std::lock_guard lock(mutex_);
        engine_.seed(seed);
    }
}
