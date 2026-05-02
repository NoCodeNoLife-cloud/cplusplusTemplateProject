#include "src/gen/RandomGenerator.hpp"

#include <fmt/format.h>
#include <random>
#include <string>
#include <stdexcept>
#include <mutex>

namespace common::gen {
    RandomGenerator::RandomGenerator() noexcept : engine_(std::random_device{}()) {
}

    RandomGenerator::RandomGenerator(const unsigned int seed) noexcept : engine_(seed) {
}

    auto RandomGenerator::nextInt(const int min, const int max) -> int {
        if (min > max) {
throw std::invalid_argument("common::RandomGenerator::nextInt: min cannot be greater than max");
        }
        std::lock_guard lock(mutex_);
        std::uniform_int_distribution dist(min, max);
        const auto result = dist(engine_);
return result;
    }

    auto RandomGenerator::nextBool() -> bool {
        return nextBool(0.5);
    }

    auto RandomGenerator::nextBool(const double trueProbability) -> bool {
        if (trueProbability < 0.0 || trueProbability > 1.0) {
throw std::invalid_argument("common::RandomGenerator::nextBool: probability must be in [0, 1]");
        }
        std::lock_guard lock(mutex_);
        std::bernoulli_distribution dist(trueProbability);
        const auto result = dist(engine_);
return result;
    }

    auto RandomGenerator::nextDouble(const double min, const double max) -> double {
        if (min >= max) {
throw std::invalid_argument("common::RandomGenerator::nextDouble: min must be less than max");
        }
        std::lock_guard lock(mutex_);
        std::uniform_real_distribution dist(min, max);
        const auto result = dist(engine_);
return result;
    }

    auto RandomGenerator::nextString(const size_t length, const std::string &charset) -> std::string {
        if (charset.empty()) {
throw std::invalid_argument("common::RandomGenerator::nextString: charset cannot be empty");
        }
        if (length == 0) {
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
return result;
    }

    auto RandomGenerator::nextGaussian(const double mean, const double stddev) -> double {
        if (stddev <= 0.0) {
throw std::invalid_argument("common::RandomGenerator::nextGaussian: stddev must be positive");
        }
        std::lock_guard lock(mutex_);
        std::normal_distribution dist(mean, stddev);
        const auto result = dist(engine_);
return result;
    }

    auto RandomGenerator::setSeed(const unsigned int seed) -> void {
std::lock_guard lock(mutex_);
        engine_.seed(seed);
    }
}
